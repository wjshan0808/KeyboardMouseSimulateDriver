
#include "stdafx.h"

#include "FileSystem.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemAccount

CFlickrItemAccount::CFlickrItemAccount(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem) :
   CNseBaseItem(pFolder, pidlFolder, pidlItem, bReleaseItem),
   m_pAccount(NULL)
{
}

BYTE CFlickrItemAccount::GetType()
{
   return FLICKRFS_MAGIC_ID_ACCOUNT;
}

SFGAOF CFlickrItemAccount::GetSFGAOF(SFGAOF dwMask)
{
   return SFGAO_FOLDER
          | SFGAO_CANCOPY
          | SFGAO_CANDELETE
          | SFGAO_BROWSABLE
          | SFGAO_HASSUBFOLDER
          | SFGAO_HASPROPSHEET
          | SFGAO_STORAGE
          | SFGAO_STORAGEANCESTOR
          | SFGAO_FILESYSANCESTOR;
}

HRESULT CFlickrItemAccount::GetExtractIcon(REFIID riid, LPVOID* ppRetVal)
{
   return ::SHCreateFileExtractIcon(_T("Folder"), FILE_ATTRIBUTE_DIRECTORY, riid, ppRetVal);
}

VFS_FIND_DATA CFlickrItemAccount::GetFindData()
{
   VFS_FIND_DATA wfd = { 0 };
   CPidlMemPtr<FLICKRFS_PIDL_ACCOUNT> pItem = m_pidlItem;
   wcscpy_s(wfd.cFileName, lengthof(wfd.cFileName), pItem->cNSID);
   wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
   return wfd;
}

VFS_FOLDERSETTINGS CFlickrItemAccount::GetFolderSettings()
{
   VFS_FOLDERSETTINGS Settings = { 0 };
   Settings.FlagsValue = FWF_USESEARCHFOLDER;
   Settings.FlagsMask = FWF_USESEARCHFOLDER;
   Settings.ViewMode = FLVM_ICONS;
   Settings.cxyIcon = 80;
   return Settings;
}

HRESULT CFlickrItemAccount::GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Info)
{
   static VFS_COLUMNINFO aColumns[] = {
      { PKEY_ItemNameDisplay,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_ItemPathDisplay,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_ItemTypeText,               SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_Title,                      SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    VFS_COLF_WRITEABLE },
      { PKEY_Comment,                    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    VFS_COLF_WRITEABLE },
      { PKEY_Keywords,                   SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    VFS_COLF_WRITEABLE },
      { PKEY_FileName,                   SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ParsingPath,                SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_FileAttributes,             SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_Document_SlideCount,        SHCOLSTATE_TYPE_INT  | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_ItemPathDisplayNarrow,      SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_SFGAOFlags,                 SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_Volume_IsRoot,              SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_InfoTip,           SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_TileInfo,          SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_FullDetails,       SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_PreviewTitle,      SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_PreviewDetails,    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_ExtendedTileInfo,  SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
   };
   if( iColumn >= lengthof(aColumns) ) return E_FAIL;
   Info = aColumns[iColumn];
   return S_OK;
}

HRESULT CFlickrItemAccount::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   if( FAILED( _EnsureFlickrRef() ) ) {
      return CNseBaseItem::GetProperty(pkey, v);
   }
   if( pkey == PKEY_ItemName ) {
      return ::InitPropVariantFromString(m_pAccount->sUserName, &v);
   }
   if( pkey == PKEY_ItemNameDisplay ) {
      return ::InitPropVariantFromString(m_pAccount->sUserName, &v);
   }
   if( pkey == PKEY_Contact_FullName ) {
      return ::InitPropVariantFromString(m_pAccount->sFullName, &v);
   }
   if( pkey == PKEY_Contact_PrimaryAddressCity ) {
      return ::InitPropVariantFromString(m_pAccount->sLocation, &v);
   }   
   if( pkey == PKEY_FileName ) {
      return ::InitPropVariantFromString(m_pAccount->sFilename, &v);
   }
   if( pkey == PKEY_Volume_IsRoot ) {
      return ::InitPropVariantFromBoolean(FALSE, &v);
   }
   if( pkey == PKEY_PropList_TileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.Contact.FullName;", &v);
   }
   if( pkey == PKEY_PropList_ExtendedTileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.Contact.FullName;System.Contact.PrimaryAddressCity;", &v);
   }
   if( pkey == PKEY_PropList_PreviewTitle ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;", &v);
   }
   if( pkey == PKEY_PropList_PreviewDetails ) {
      return ::InitPropVariantFromString(L"prop:System.Contact.FullName;System.Contact.PrimaryAddressCity;", &v);
   }
   if( pkey == PKEY_PropList_InfoTip ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.Contact.FullName;System.Contact.PrimaryAddressCity;", &v);
   }
   if( pkey == PKEY_PropList_FullDetails ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;*System.Contact.FullName;*System.Contact.PrimaryAddressCity;", &v);
   }
   return CNseBaseItem::GetProperty(pkey, v);
}

CNseItem* CFlickrItemAccount::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem)
{
   if( !CPidlMemPtr<FLICKRFS_PIDL_IMAGESET>(pidlItem).IsType(FLICKRFS_MAGIC_ID_IMAGESET) ) return NULL;
   return new CFlickrItemImageset(pFolder, pidlFolder, pidlItem, bReleaseItem);
}

CNseItem* CFlickrItemAccount::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd)
{
   return new CFlickrItemImageset(pFolder, pidlFolder, CFlickrItemImageset::GenerateITEMID(wfd), TRUE);
}

HRESULT CFlickrItemAccount::GetChild(LPCWSTR pwstrName, SHGNO ParseType, CNseItem** ppItem)
{
   HR( _EnsureFlickrRef() );
   TFlickrImageset* pImageset = NULL;
   HR( _ShellModule.Rest.FindImageset(m_pAccount, pwstrName, &pImageset) );
   *ppItem = new CFlickrItemImageset(m_pFolder, m_pidlFolder, CFlickrItemImageset::GenerateITEMID(pImageset), TRUE);
   return *ppItem != NULL ? S_OK : E_OUTOFMEMORY;
}

HRESULT CFlickrItemAccount::EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aItems)
{
   if( !IsBitSet(grfFlags, SHCONTF_FOLDERS|SHCONTF_STORAGE) ) return S_OK;
   HR( _EnsureFlickrRef() );
   CFlickrImagesetList aList;
   HRESULT Hr = _ShellModule.Rest.GetImagesetList(m_pAccount, aList);
   if( FAILED(Hr) && hwndOwner != NULL ) DoDisplayErrorMessage(hwndOwner, IDS_ERR_ENUM, Hr);
   for( int i = 0; i < aList.GetSize(); i++ ) {
      const TFlickrImageset* pImageset = aList[i];
      aItems.Add( new CFlickrItemImageset(m_pFolder, m_pidlFolder, CFlickrItemImageset::GenerateITEMID(pImageset), TRUE) );
   }
   return Hr;
}

HRESULT CFlickrItemAccount::Delete()
{
   HR( _EnsureFlickrRef() );
   HR( _ShellModule.Rest.RemoveAccount(m_pAccount) );
   return S_OK;
}

HRESULT CFlickrItemAccount::Refresh(VFS_REFRESH Reason)
{
   if( Reason != VFS_REFRESH_USERFORCED ) return E_NOTIMPL;
   HR( _EnsureFlickrRef() );
   HR( _ShellModule.Rest.RefreshAccountList(m_pAccount) );
   return S_OK;
}

HMENU CFlickrItemAccount::GetMenu()
{
   if( FAILED( _EnsureFlickrRef() ) ) return NULL;
   UINT nRes = IDM_ACCOUNT_PUBLIC;
   if( m_pAccount->Access == FLICKRFS_ACCOUNT_OWNED ) nRes = IDM_ACCOUNT_OWNED;
   return ::LoadMenu(_pModule->GetResourceInstance(), MAKEINTRESOURCE(nRes));
}

HRESULT CFlickrItemAccount::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   HR( _EnsureFlickrRef() );
   switch( Cmd.wMenuID ) {
   case ID_COMMAND_ADD_TAGFOLDER: return _DoCreateTagFolder(Cmd);
   case ID_COMMAND_ADD_PHOTOSET:  return _DoCreatePhotoSet(Cmd);
   case ID_FILE_NEW_TAGFOLDER:    return _DoCreateTagFolder(Cmd);
   case ID_FILE_NEW_PHOTOSET:     return _DoCreatePhotoSet(Cmd);
   case ID_FILE_PROPERTIES:       return _DoShowProperties(Cmd);
   case DFM_CMD_NEWFOLDER:        return _DoCreateTagFolder(Cmd);
   }
   return E_NOTIMPL;
}

// Static members

PCITEMID_CHILD CFlickrItemAccount::GenerateITEMID(const FLICKRFS_PIDL_ACCOUNT& src)
{
   FLICKRFS_PIDL_ACCOUNT data = src;
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

PCITEMID_CHILD CFlickrItemAccount::GenerateITEMID(const TFlickrAccount* pAccount)
{
   ATLASSERT(pAccount);
   FLICKRFS_PIDL_ACCOUNT data = { 0 };
   data.magic = FLICKRFS_MAGIC_ID_ACCOUNT;
   wcscpy_s(data.cNSID, lengthof(data.cNSID), pAccount->sNSID);
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

PCITEMID_CHILD CFlickrItemAccount::GenerateITEMID(const WIN32_FIND_DATA& wfd)
{
   FLICKRFS_PIDL_ACCOUNT data = { 0 };
   data.magic = FLICKRFS_MAGIC_ID_ACCOUNT;
   wcscpy_s(data.cNSID, lengthof(data.cNSID), wfd.cFileName);
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

// Implementation

HRESULT CFlickrItemAccount::_EnsureFlickrRef()
{
   // Bind this NSE Item to a real Flickr Account object
   if( m_pAccount != NULL ) return S_OK;
   CPidlMemPtr<FLICKRFS_PIDL_ACCOUNT> pItem = m_pidlItem;
   if( !pItem.IsType(FLICKRFS_MAGIC_ID_ACCOUNT) ) return E_UNEXPECTED;
   return _ShellModule.Rest.GetAccountFromNSID(pItem->cNSID, &m_pAccount);
}

HRESULT CFlickrItemAccount::_DoCreateTagFolder(VFS_MENUCOMMAND& Cmd)
{
   CComBSTR bstrLabel;
   bstrLabel.LoadString(IDS_NEWTAGFOLDER);
   HR( _ShellModule.Rest.AddImageset(m_pAccount, bstrLabel, FLICKRFS_IMAGESET_TAGSEARCH) );
   HR( _AddSelectEdit(Cmd, bstrLabel) );
   return S_OK;
}

HRESULT CFlickrItemAccount::_DoCreatePhotoSet(VFS_MENUCOMMAND& Cmd)
{
   CComBSTR bstrLabel;
   bstrLabel.LoadString(IDS_NEWPHOTOSET);
   HR( _ShellModule.Rest.AddImageset(m_pAccount, bstrLabel, FLICKRFS_IMAGESET_PHOTOSET) );
   HR( _AddSelectEdit(Cmd, bstrLabel) );
   return S_OK;
}

