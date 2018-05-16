
#include "stdafx.h"

#include "FileSystem.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemImageset

CFlickrItemImageset::CFlickrItemImageset(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem) :
   CNseBaseItem(pFolder, pidlFolder, pidlItem, bReleaseItem),
   m_pImageset(NULL)
{
}

BYTE CFlickrItemImageset::GetType()
{
   return FLICKRFS_MAGIC_ID_IMAGESET;
}

SFGAOF CFlickrItemImageset::GetSFGAOF(SFGAOF dwMask)
{
   DWORD dwFlags = SFGAO_FOLDER
                   | SFGAO_CANCOPY
                   | SFGAO_BROWSABLE
                   | SFGAO_HASPROPSHEET
                   | SFGAO_STORAGE
                   | SFGAO_STORAGEANCESTOR
                   | SFGAO_FILESYSANCESTOR;

   // Enable features depending on the type
   if( IsBitSet(dwMask, SFGAO_CANRENAME|SFGAO_CANDELETE|SFGAO_DROPTARGET) ) 
   {
      if( SUCCEEDED( _EnsureFlickrRef() ) ) {
         if( m_pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH ) {
            dwFlags |= SFGAO_CANRENAME | SFGAO_CANDELETE;
         }
         if( m_pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET && m_pImageset->pAccount->Access == FLICKRFS_ACCOUNT_OWNED ) {
            dwFlags |= SFGAO_CANRENAME | SFGAO_CANDELETE;
         }
         if( m_pImageset->pAccount->Access == FLICKRFS_ACCOUNT_OWNED ) {
            dwFlags |= SFGAO_DROPTARGET;
         }
      }
   }

   return dwFlags;
}

HRESULT CFlickrItemImageset::GetExtractIcon(REFIID riid, LPVOID* ppRetVal)
{
   return ::SHCreateFileExtractIcon(_T("Folder"), FILE_ATTRIBUTE_DIRECTORY, riid, ppRetVal);
}

HRESULT CFlickrItemImageset::IsDropDataAvailable(IDataObject* pDataObj)
{
   HR( _EnsureFlickrRef() );
   // User can only drop new files onto his own accounts
   if( m_pImageset->Type == FLICKRFS_IMAGESET_RECENT ) return S_FALSE;
   if( m_pImageset->pAccount->Access != FLICKRFS_ACCOUNT_OWNED ) return S_FALSE;
   // We support file drops
   return DataObj_HasFileClipFormat(pDataObj) ? S_OK : S_FALSE;
}

VFS_FIND_DATA CFlickrItemImageset::GetFindData()
{
   VFS_FIND_DATA wfd = { 0 };
   CPidlMemPtr<FLICKRFS_PIDL_IMAGESET> pItem = m_pidlItem;
   wcscpy_s(wfd.cFileName, lengthof(wfd.cFileName), pItem->cImagesetID);
   wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
   return wfd;
}

VFS_FOLDERSETTINGS CFlickrItemImageset::GetFolderSettings()
{
   VFS_FOLDERSETTINGS Settings = { 0 };
   Settings.FlagsValue = FWF_USESEARCHFOLDER;
   Settings.FlagsMask = FWF_USESEARCHFOLDER;
   Settings.ViewMode = FLVM_ICONS;
   Settings.cxyIcon = 96;
   return Settings;
}

HRESULT CFlickrItemImageset::GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Info)
{
   static VFS_COLUMNINFO aColumns[] = {
      { PKEY_ItemNameDisplay,             SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_ItemPathDisplay,             SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_FileName,                    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_Title,                       SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_WRITEABLE },
      { PKEY_Comment,                     SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_WRITEABLE },
      { PKEY_Keywords,                    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI | SHCOLSTATE_SLOW,  VFS_COLF_WRITEABLE },
      { PKEY_FileName,                    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_FileExtension,               SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_FileAttributes,              SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ParsingPath,                 SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_Size,                        SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ItemType,                    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ItemTypeText,                SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_DateCreated,                 SHCOLSTATE_TYPE_DATE | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_DateModified,                SHCOLSTATE_TYPE_DATE | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_ItemPathDisplayNarrow,       SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ItemFolderPathDisplayNarrow, SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },      
      { PKEY_Photo_DateTaken,             SHCOLSTATE_TYPE_DATE | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_Kind,                        SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_Media_Publisher,             SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_Media_ContentDistributor,    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },      
      { PKEY_Photo_CameraManufacturer,    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_Photo_CameraModel,           SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_Image_HorizontalResolution,  SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_Image_VerticalResolution,    SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_Image_HorizontalSize,        SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_Image_VerticalSize,          SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_Image_BitDepth,              SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_Image_Dimensions,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN | SHCOLSTATE_SLOW,       VFS_COLF_NOTCOLUMN },
      { PKEY_SFGAOFlags,                  SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PerceivedType,               SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_Volume_IsRoot,               SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_InfoTip,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_TileInfo,           SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_FullDetails,        SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_PreviewTitle,       SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_PreviewDetails,     SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_ExtendedTileInfo,   SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
   };
   if( iColumn >= lengthof(aColumns) ) return E_FAIL;
   Info = aColumns[iColumn];
   return S_OK;
}

HRESULT CFlickrItemImageset::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   if( FAILED( _EnsureFlickrRef() ) ) {
      return CNseBaseItem::GetProperty(pkey, v);
   }
   if( pkey == PKEY_ItemName ) {
      return ::InitPropVariantFromString(m_pImageset->sTitle, &v);
   }
   if( pkey == PKEY_ItemNameDisplay ) {
      return ::InitPropVariantFromString(m_pImageset->sTitle, &v);
   }
   if( pkey == PKEY_ItemType ) {
      return ::InitPropVariantFromString(L"Folder", &v);
   }
   if( pkey == PKEY_ItemTypeText ) {
      CComBSTR bstr;
      switch( m_pImageset->Type ) {
      case FLICKRFS_IMAGESET_GROUP:     bstr.LoadString(IDS_TYPE_GROUP); break;
      case FLICKRFS_IMAGESET_PHOTOSET:  bstr.LoadString(IDS_TYPE_PHOTOSET); break;
      case FLICKRFS_IMAGESET_TAGSEARCH: bstr.LoadString(IDS_TYPE_TAGFOLDER); break;
      default:                          bstr.LoadString(IDS_TYPE_STANDARD); break;
      }
      return ::InitPropVariantFromString(bstr, &v);
   }
   if( pkey == PKEY_Title ) {
      if( m_pImageset->Type != FLICKRFS_IMAGESET_PHOTOSET && m_pImageset->Type != FLICKRFS_IMAGESET_TAGSEARCH ) return E_FAIL;
      return ::InitPropVariantFromString(m_pImageset->sTitle, &v);
   }
   if( pkey == PKEY_Comment ) {
      if( m_pImageset->Type != FLICKRFS_IMAGESET_PHOTOSET && m_pImageset->Type != FLICKRFS_IMAGESET_TAGSEARCH ) return E_FAIL;
      return ::InitPropVariantFromString(m_pImageset->sDescription, &v);
   }
   if( pkey == PKEY_Keywords ) {
      if( m_pImageset->Type != FLICKRFS_IMAGESET_TAGSEARCH ) return E_FAIL;
      v = m_pImageset->sKeywords;
      return v.CoerceToCanonicalValue(PKEY_Keywords);
   }
   if( pkey == PKEY_Document_SlideCount ) {
      return ::InitPropVariantFromInt32(m_pImageset->lPriority, &v);
   }
   if( pkey == PKEY_FileName ) {
      return ::InitPropVariantFromString(m_pImageset->sFilename, &v);
   }
   if( pkey == PKEY_Volume_IsRoot ) {
      return ::InitPropVariantFromBoolean(FALSE, &v);
   }
   if( pkey == PKEY_PropList_TileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.ItemTypeText;", &v);
   }
   if( pkey == PKEY_PropList_ExtendedTileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.ItemTypeText;System.DateModified;", &v);
   }
   if( pkey == PKEY_PropList_PreviewTitle ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.ItemTypeText;", &v);
   }
   if( pkey == PKEY_PropList_PreviewDetails ) {
      return ::InitPropVariantFromString(L"prop:System.DateModified;System.Title;System.Comment;System.Keywords;", &v);
   }
   if( pkey == PKEY_PropList_InfoTip ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.ItemTypeText;System.DateModified;System.Comment;System.Keywords;", &v);
   }
   if( pkey == PKEY_PropList_FullDetails ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.ItemTypeText;System.DateModified;System.Title;System.Comment;System.Keywords;", &v);
   }
   return CNseBaseItem::GetProperty(pkey, v);
}

HRESULT CFlickrItemImageset::SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v)
{
   HR( _EnsureFlickrRef() );
   HR( _ShellModule.Rest.SetImagesetProperty(m_pImageset, pkey, v) );
   HR( _RefreshFolderView() );
   return S_OK;
}

VFS_PROPSTATE CFlickrItemImageset::GetPropertyState(REFPROPERTYKEY pkey)
{
   if( SUCCEEDED( _EnsureFlickrRef() ) )
   {
      if( pkey == PKEY_Title ) {
         if( m_pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH ) return VFS_PROPSTATE_NORMAL;
         if( m_pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET && m_pImageset->pAccount->Access == FLICKRFS_ACCOUNT_OWNED ) return VFS_PROPSTATE_NORMAL;
         return VFS_PROPSTATE_READONLY;
      }
      if( pkey == PKEY_Comment ) {
         if( m_pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH ) return VFS_PROPSTATE_NORMAL;
         if( m_pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET && m_pImageset->pAccount->Access == FLICKRFS_ACCOUNT_OWNED ) return VFS_PROPSTATE_NORMAL;
         return VFS_PROPSTATE_READONLY;
      }
      if( pkey == PKEY_Keywords ) {
         if( m_pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH ) return VFS_PROPSTATE_NORMAL;
         return VFS_PROPSTATE_READONLY;
      }
   }
   return VFS_PROPSTATE_NORMAL;
}

CNseItem* CFlickrItemImageset::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem)
{
   if( !CPidlMemPtr<FLICKRFS_PIDL_PHOTO>(pidlItem).IsType(FLICKRFS_MAGIC_ID_PHOTO) ) return NULL;
   return new CFlickrItemPhoto(pFolder, pidlFolder, pidlItem, bReleaseItem);
}

CNseItem* CFlickrItemImageset::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd)
{
   return new CFlickrItemPhoto(pFolder, pidlFolder, CFlickrItemPhoto::GenerateITEMID(wfd), TRUE);
}

HRESULT CFlickrItemImageset::GetChild(LPCWSTR pwstrName, SHGNO ParseType, CNseItem** ppItem)
{
   HR( _EnsureFlickrRef() );
   TFlickrPhoto* pPhoto = NULL;
   HR( _ShellModule.Rest.UpdateImagesetPhotoList(m_pImageset) );
   HR( _ShellModule.Rest.GetPhoto(pwstrName, &pPhoto) );
   *ppItem = new CFlickrItemPhoto(m_pFolder, m_pidlFolder, CFlickrItemPhoto::GenerateITEMID(pPhoto), TRUE);
   return *ppItem != NULL ? S_OK : E_OUTOFMEMORY;
}

HRESULT CFlickrItemImageset::EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aItems)
{
   if( !IsBitSet(grfFlags, SHCONTF_NONFOLDERS|SHCONTF_STORAGE) ) return S_OK;
   HR( _EnsureFlickrRef() );
   CFlickrPhotoList aList;
   HRESULT Hr = _ShellModule.Rest.GetImagesetPhotoList(m_pImageset, aList);
   if( HRESULT_CODE(Hr) == ERROR_NOT_FOUND ) Hr = S_FALSE;
   if( FAILED(Hr) && hwndOwner != NULL ) DoDisplayErrorMessage(hwndOwner, IDS_ERR_ENUM, Hr);
   for( int i = 0; i < aList.GetSize(); i++ ) {
      const TFlickrPhoto* pPhoto = aList[i];
      aItems.Add( new CFlickrItemPhoto(m_pFolder, m_pidlFolder, CFlickrItemPhoto::GenerateITEMID(pPhoto), TRUE) );
   }
   return Hr;
}

HMENU CFlickrItemImageset::GetMenu()
{
   return ::LoadMenu(_pModule->GetResourceInstance(), MAKEINTRESOURCE(IDM_IMAGESET));
}

HRESULT CFlickrItemImageset::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   HR( _EnsureFlickrRef() );
   switch( Cmd.wMenuID ) {
   case DFM_CMD_PASTE:       return _DoPastePhotos(Cmd);
   case ID_FILE_PROPERTIES:  return _DoShowProperties(Cmd);
   }
   return E_NOTIMPL;
}

HRESULT CFlickrItemImageset::Delete()
{
   HR( _EnsureFlickrRef() );
   HR( _ShellModule.Rest.DeleteImageset(m_pImageset) );
   HR( _RefreshFolderView() );
   return S_OK;
}

HRESULT CFlickrItemImageset::Refresh(VFS_REFRESH Reason)
{
   if( Reason != VFS_REFRESH_USERFORCED ) return E_NOTIMPL;
   HR( _EnsureFlickrRef() );
   HR( _ShellModule.Rest.RefreshImagesetPhotoList(m_pImageset) );
   return S_OK;
}

HRESULT CFlickrItemImageset::Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName)
{
   HR( _EnsureFlickrRef() );
   HR( _ShellModule.Rest.SetImagesetProperty(m_pImageset, PKEY_Title, CComPropVariant(pstrNewName)) );
   HR( _RefreshFolderView() );
   return S_OK;
}

// Static members

PCITEMID_CHILD CFlickrItemImageset::GenerateITEMID(const FLICKRFS_PIDL_IMAGESET& src)
{
   FLICKRFS_PIDL_IMAGESET data = src;
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

PCITEMID_CHILD CFlickrItemImageset::GenerateITEMID(const TFlickrImageset* pImageset)
{
   ATLASSERT(pImageset);
   FLICKRFS_PIDL_IMAGESET data = { 0 };
   data.magic = FLICKRFS_MAGIC_ID_IMAGESET;
   wcscpy_s(data.cImagesetID, lengthof(data.cImagesetID), pImageset->sImagesetID);
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

PCITEMID_CHILD CFlickrItemImageset::GenerateITEMID(const WIN32_FIND_DATA& wfd)
{
   FLICKRFS_PIDL_IMAGESET data = { 0 };
   data.magic = FLICKRFS_MAGIC_ID_IMAGESET;
   wcscpy_s(data.cImagesetID, lengthof(data.cImagesetID), wfd.cFileName);
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

// Implementation

HRESULT CFlickrItemImageset::_EnsureFlickrRef()
{
   // Bind this NSE Item to a real Flickr Imageset object
   if( m_pImageset != NULL ) return S_OK;
   CPidlMemPtr<FLICKRFS_PIDL_ACCOUNT> pRoot = m_pidlFolder;
   if( !pRoot.IsType(FLICKRFS_MAGIC_ID_ACCOUNT) ) return E_UNEXPECTED;
   CPidlMemPtr<FLICKRFS_PIDL_IMAGESET> pItem = m_pidlItem;
   if( !pItem.IsType(FLICKRFS_MAGIC_ID_IMAGESET) ) return E_UNEXPECTED;
   TFlickrAccount* pAccount = NULL;
   HR( _ShellModule.Rest.GetAccountFromNSID(pRoot->cNSID, &pAccount) );
   return _ShellModule.Rest.FindImageset(pAccount, pItem->cImagesetID, &m_pImageset);
}

HRESULT CFlickrItemImageset::_DoPastePhotos(VFS_MENUCOMMAND& Cmd)
{
   HRESULT Hr = NSE_S_ALL_DONE;
   // If we're dragging our own photos to our own folder, just apply correct properties.
   // If we're dragging a filename, then it's a regular copy-operation.
   CComPtr<IDataObject> spDataObj = Cmd.pDataObject;
   if( spDataObj == NULL ) return E_FAIL;
   CPidl pidlItem;
   ::SHGetFirstIDListFromDataObject(spDataObj, &pidlItem.m_p);
   CPidlMemPtr<FLICKRFS_PIDL_PHOTO> pItem = pidlItem.GetLastItem();
   if( !pItem.IsType(FLICKRFS_MAGIC_ID_PHOTO) )
   {
      // Source is not one of our own photos.
      // Do the default "paste" operation; this involves copying the files.
      Hr = _DoPasteFiles(Cmd);
   }
   else
   {
      // Put photos in requested imageset
      CComPtr<IShellItemArray> spItems;
      HR( ::SHCreateShellItemArrayFromDataObject(spDataObj, IID_PPV_ARGS(&spItems)) );
      DWORD dwItemCount = 0;
      HR( spItems->GetCount(&dwItemCount) );
      for( DWORD i = 0; i < dwItemCount; i++ ) {
         CPidl pidlItem;
         CComPtr<IShellItem> spItem;
         HR( spItems->GetItemAt(i, &spItem) );
         HR( pidlItem.CreateFromObject(spItem) );
         CPidlMemPtr<FLICKRFS_PIDL_PHOTO> pItem = pidlItem.GetLastItem();
         if( !pItem.IsType(FLICKRFS_MAGIC_ID_PHOTO) ) continue;
         TFlickrPhoto* pPhoto = NULL;
         HR( _ShellModule.Rest.GetPhoto(pItem->cPhotoID, &pPhoto) );
         HR( _ShellModule.Rest.PostProcessPhotoUpload(pPhoto, m_pImageset) );
         ::SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST, pidlItem);
      }
   }
   HR( _RefreshFolderView() );
   return Hr;
}

