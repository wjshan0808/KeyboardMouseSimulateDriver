
#include "stdafx.h"

#include "FileSystem.h"
#include "ShellFolder.h"
#include "PropSheetExt.h"


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemRoot

CFlickrItemRoot::CFlickrItemRoot(CShellFolder* pFolder) :
   CNseBaseItem(pFolder, NULL, NULL, FALSE)
{
}

BYTE CFlickrItemRoot::GetType()
{
   return FLICKRFS_MAGIC_ID_ROOT;
}

SFGAOF CFlickrItemRoot::GetSFGAOF(SFGAOF dwMask)
{
   return SFGAO_FOLDER
          | SFGAO_CANCOPY
          | SFGAO_BROWSABLE
          | SFGAO_HASSUBFOLDER
          | SFGAO_HASPROPSHEET
          | SFGAO_DROPTARGET
          | SFGAO_STORAGE
          | SFGAO_STORAGEANCESTOR
          | SFGAO_FILESYSANCESTOR;
}

HRESULT CFlickrItemRoot::GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Info)
{
   static VFS_COLUMNINFO aColumns[] = {
      { PKEY_ItemNameDisplay,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_ItemPathDisplay,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_Contact_FullName,           SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_Contact_PrimaryAddressCity, SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },      
      { PKEY_FileName,                   SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_FileAttributes,             SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ParsingPath,                SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
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

HRESULT CFlickrItemRoot::GetPropertyPage(CNsePropertyPage** ppPage)
{
   *ppPage = new CNseFileRootPropertyPage();
   return *ppPage != NULL ? S_OK : E_OUTOFMEMORY;
}

HRESULT CFlickrItemRoot::GetExtractIcon(REFIID riid, LPVOID* ppRetVal)
{
   return ::SHCreateFileExtractIcon(_T("Folder"), FILE_ATTRIBUTE_DIRECTORY, riid, ppRetVal);
}

HRESULT CFlickrItemRoot::IsDropDataAvailable(IDataObject* pDataObj)
{
   // We support file drops for the Shell SendTo feature
   return DataObj_HasFileClipFormat(pDataObj) ? S_OK : S_FALSE;
}

VFS_FIND_DATA CFlickrItemRoot::GetFindData()
{
   VFS_FIND_DATA wfd = { 0 };
   wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
   return wfd;
}

VFS_FOLDERSETTINGS CFlickrItemRoot::GetFolderSettings()
{
   VFS_FOLDERSETTINGS Settings = { 0 };
   Settings.ViewMode = FLVM_ICONS;
   Settings.cxyIcon = 64;
   return Settings;
}

HRESULT CFlickrItemRoot::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   if( pkey == PKEY_ItemNameDisplay ) {
      TFlickrAccount* pAccount = NULL;
      HR( _ShellModule.Rest.GetAccountFromPrimary(&pAccount) );
      return ::InitPropVariantFromString(pAccount->sUserName, &v);
   }
   if( pkey == PKEY_ItemTypeText ) {
      return ::InitPropVariantFromString(L"Flickr Drive", &v);
   }
   if( pkey == PKEY_Volume_IsRoot ) {
      return ::InitPropVariantFromBoolean(TRUE, &v);
   }
   // In the Property Page, the File System name is displayed
   if( pkey == PKEY_Volume_FileSystem ) {
      return ::InitPropVariantFromString(L"FlickrFS", &v);
   }
   // Our default root Property Page will show a pie chart with capacity/free space
   if( pkey == PKEY_Capacity ) {
      HR( _ShellModule.Rest.ReadUploadStatus() );
      return ::InitPropVariantFromUInt64(_ShellModule.Rest.m_VolumeInfo.ullUploadMaxBytes, &v);
   }
   if( pkey == PKEY_FileAllocationSize ) {
      HR( _ShellModule.Rest.ReadUploadStatus() );
      return ::InitPropVariantFromUInt64(_ShellModule.Rest.m_VolumeInfo.ullUploadUsedBytes, &v);
   }
   if( pkey == PKEY_FreeSpace ) {
      HR( _ShellModule.Rest.ReadUploadStatus() );
      return ::InitPropVariantFromUInt64(_ShellModule.Rest.m_VolumeInfo.ullUploadRemainingBytes, &v);
   }
   if( pkey == PKEY_PercentFull ) {
      HR( _ShellModule.Rest.ReadUploadStatus() );
      if( _ShellModule.Rest.m_VolumeInfo.ullUploadMaxBytes == 0 ) return ::InitPropVariantFromUInt32(0, &v);
      return ::InitPropVariantFromUInt32((ULONG)((DOUBLE)_ShellModule.Rest.m_VolumeInfo.ullUploadRemainingBytes / (DOUBLE)_ShellModule.Rest.m_VolumeInfo.ullUploadMaxBytes * 100.0), &v);
   }
   // Define what information to display in tool tip and tile information
   if( pkey == PKEY_PropList_TileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.Volume.FileSystem;", &v);
   }
   if( pkey == PKEY_PropList_ExtendedTileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;", &v);
   }
   if( pkey == PKEY_PropList_PreviewTitle ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.ItemTypeText;", &v);
   }
   if( pkey == PKEY_PropList_PreviewDetails ) {
      return ::InitPropVariantFromString(L"prop:System.Volume.FileSystem;", &v);
   }
   if( pkey == PKEY_PropList_InfoTip ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;Volume.FileSystem;", &v);
   }
   if( pkey == PKEY_PropList_FullDetails ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.PercentFull;System.Volume.FileSystem;", &v);
   }
   return CNseBaseItem::GetProperty(pkey, v);
}

CNseItem* CFlickrItemRoot::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem)
{
   if( !CPidlMemPtr<FLICKRFS_PIDL_ACCOUNT>(pidlItem).IsType(FLICKRFS_MAGIC_ID_ACCOUNT) ) return NULL;
   return new CFlickrItemAccount(pFolder, pidlFolder, pidlItem, bReleaseItem);
}

CNseItem* CFlickrItemRoot::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd)
{
   return new CFlickrItemAccount(pFolder, pidlFolder, CFlickrItemAccount::GenerateITEMID(wfd), TRUE);
}

HRESULT CFlickrItemRoot::GetChild(LPCWSTR pwstrName, SHGNO ParseType, CNseItem** ppItem)
{
   CFlickrAccountList aList;
   HR( _ShellModule.Rest.GetAccountList(aList) );
   for( int i = 0; i < aList.GetSize(); i++ ) {
      const TFlickrAccount* pAccount = aList[i];
      if( pAccount->sNSID == pwstrName || pAccount->sUserName == pwstrName ) {
         *ppItem = new CFlickrItemAccount(m_pFolder, m_pidlFolder, CFlickrItemAccount::GenerateITEMID(pAccount), TRUE);
         return *ppItem != NULL ? S_OK : E_OUTOFMEMORY;
      }
   }
   return E_FAIL;
}

HRESULT CFlickrItemRoot::EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aItems)
{
   if( !IsBitSet(grfFlags, SHCONTF_FOLDERS|SHCONTF_STORAGE) ) return S_OK;
   CFlickrAccountList aList;
   HR( _ShellModule.Rest.GetAccountList(aList) );
   if( aList.GetSize() == 0 && hwndOwner != NULL ) {
      // Prompt user to add the first account to the list...
      HR( _ShellModule.Rest.WelcomeAccount(hwndOwner) );
      HR( _ShellModule.Rest.GetAccountList(aList) );
   }
   for( int i = 0; i < aList.GetSize(); i++ ) {
      const TFlickrAccount* pAccount = aList[i];
      aItems.Add( new CFlickrItemAccount(m_pFolder, m_pidlFolder, CFlickrItemAccount::GenerateITEMID(pAccount), TRUE) );
   }
   return S_OK;
}

HMENU CFlickrItemRoot::GetMenu()
{
   return ::LoadMenu(_pModule->GetResourceInstance(), MAKEINTRESOURCE(IDM_ROOT));
}

HRESULT CFlickrItemRoot::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   HRESULT Hr = E_NOTIMPL;
   switch( Cmd.wMenuID ) {
   case ID_FILE_PROPERTIES:   Hr = _DoShowProperties(Cmd); break;
   case ID_FILE_NEW_USER:     Hr = _ShellModule.Rest.AddAccount(Cmd.hWnd); break;
   case ID_COMMAND_ADD_USER:  Hr = _ShellModule.Rest.AddAccount(Cmd.hWnd); break;
   case DFM_CMD_NEWFOLDER:    Hr = _ShellModule.Rest.AddAccount(Cmd.hWnd); break;
   case DFM_CMD_PASTE:        Hr = _DoPasteSendTo(Cmd); break;
   }
   if( SUCCEEDED(Hr) ) _RefreshFolderView();
   return Hr;
}

// Implementation

HRESULT CFlickrItemRoot::_DoPasteSendTo(VFS_MENUCOMMAND& Cmd)
{
   if( Cmd.pDataObject == NULL ) return E_FAIL;
   // We can't paste photos directly to the root, but must find an appropriate imageset folder. We'll try
   // the primary account's "NotInSet" folder.
   TFlickrAccount* pAccount = NULL;
   TFlickrImageset* pImageset = NULL;
   HR( _ShellModule.Rest.GetAccountFromPrimary(&pAccount) );
   HR( _ShellModule.Rest.FindImageset(pAccount, FLICKRFS_FOLDERSTR_NOTINSET, &pImageset) );
   // Get a IShellItem for the target folder...
   WCHAR wszTarget[100] = { 0 };
   ::wnsprintf(wszTarget, lengthof(wszTarget) - 1, L"%s\\%s", pAccount->sNSID, pImageset->sImagesetID);
   CComPtr<IShellItem> spTargetFolder;
   HR( ::SHCreateItemFromRelativeName(m_pFolder, wszTarget, NULL, IID_PPV_ARGS(&spTargetFolder)) );
   // Do paste operation...
   if( Cmd.pFO == NULL ) {
      HR( ::SHCreateFileOperation(Cmd.hWnd, FOF_NOCOPYSECURITYATTRIBS | FOF_NOCONFIRMATION | FOFX_NOSKIPJUNCTIONS, &Cmd.pFO) );
   }
   Cmd.pFO->CopyItems(Cmd.pDataObject, spTargetFolder);
   if( Cmd.dwDropEffect == DROPEFFECT_MOVE ) Cmd.pFO->DeleteItems(Cmd.pDataObject);
   // We handled this operation successfully for all items in selection
   return NSE_S_ALL_DONE;
}

