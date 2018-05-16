
#include "stdafx.h"

#include "FileSystem.h"
#include "ShellFolder.h"
#include "LaunchFile.h"
#include "ImageThumbProvider.h"


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemPhoto

CFlickrItemPhoto::CFlickrItemPhoto(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem) :
   CNseBaseItem(pFolder, pidlFolder, pidlItem, bReleaseItem),
   m_pPhoto(NULL)
{
}

BYTE CFlickrItemPhoto::GetType()
{
   return FLICKRFS_MAGIC_ID_PHOTO;
}

SFGAOF CFlickrItemPhoto::GetSFGAOF(SFGAOF dwMask)
{
   DWORD dwFlags = SFGAO_CANCOPY 
                   | SFGAO_STREAM 
                   | SFGAO_HASPROPSHEET;

   // Enable features depending on the account type
   if( IsBitSet(dwMask, SFGAO_CANMOVE|SFGAO_CANDELETE) ) {
      if( SUCCEEDED( _EnsureFlickrRef() ) ) {
         if( m_pPhoto->bIsOwned ) dwFlags |= SFGAO_CANMOVE | SFGAO_CANDELETE;
      }
   }
   // If we're in the SaveAs mode/hack then pretend we are a real file
   if( IsBitSet(dwMask, SFGAO_FILESYSANCESTOR) ) {
      if( SUCCEEDED( _EnsureFlickrRef() ) ) {
         if( m_pPhoto->bIsSpawned ) dwFlags |= SFGAO_FILESYSTEM;
      }
   }

   return dwFlags;
}

HRESULT CFlickrItemPhoto::GetExtractIcon(REFIID riid, LPVOID* ppRetVal)
{
   return ::SHCreateFileExtractIcon(_T("image.jpg"), FILE_ATTRIBUTE_NORMAL, riid, ppRetVal);
}

HRESULT CFlickrItemPhoto::GetThumbnail(REFIID riid, LPVOID* ppRetVal)
{
   return ::SHCreateImageThumbProvider(m_pFolder, GetITEMID(), riid, ppRetVal);
}

VFS_FIND_DATA CFlickrItemPhoto::GetFindData()
{
   VFS_FIND_DATA wfd = { 0 };
   if( SUCCEEDED( _EnsureFlickrRef() ) ) {
      wcscpy_s(wfd.cFileName, lengthof(wfd.cFileName), m_pPhoto->sFilename);
      wcscat_s(wfd.cFileName, lengthof(wfd.cFileName), m_pPhoto->sExtension);
      wfd.ftCreationTime = m_pPhoto->ftDateTaken;
      wfd.ftLastWriteTime = m_pPhoto->ftDatePosted;
      if( m_pPhoto->bIsSpawned ) wfd.cAlternateFileName[1] = VFS_HACK_SAVEAS_JUNCTION;
   }
   else {
      // We should return the ParsingName here because of the dependency on how
      // CNseBaseItem::GetProperty resolves PKEY_ParsingName. Nasty.
      CPidlMemPtr<FLICKRFS_PIDL_PHOTO> pItem = m_pidlItem;
      wcscpy_s(wfd.cFileName, lengthof(wfd.cFileName), pItem->cPhotoID);
   }
   wfd.dwFileAttributes = FILE_ATTRIBUTE_VIRTUAL | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
   return wfd;
}

HRESULT CFlickrItemPhoto::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   if( FAILED( _EnsureFlickrRef() ) ) {
      return CNseBaseItem::GetProperty(pkey, v);
   }
   if( pkey == PKEY_ItemName ) {
      return ::InitPropVariantFromString(m_pPhoto->sTitle, &v);
   }
   if( pkey == PKEY_ParsingName ) {
      return ::InitPropVariantFromString(m_pPhoto->sPhotoID, &v);
   } 
   if( pkey == PKEY_ItemNameDisplay ) {
      return ::InitPropVariantFromString(m_pPhoto->sTitle, &v);
   }
   if( pkey == PKEY_ItemType ) {
      return ::InitPropVariantFromString(m_pPhoto->sExtension, &v);
   }
   if( pkey == PKEY_ItemTypeText ) {
      SHFILEINFO sfi = { 0 }; 
      CString sFilename = m_pPhoto->sFilename + m_pPhoto->sExtension;
      ::SHGetFileInfo(sFilename, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
      return ::InitPropVariantFromString(sfi.szTypeName, &v);
   }
   if( pkey == PKEY_Title ) {
      return ::InitPropVariantFromString(m_pPhoto->sTitle, &v);
   }
   if( pkey == PKEY_Kind ) {
      v = KIND_PICTURE;
      return v.CoerceToCanonicalValue(PKEY_Kind);
   }   
   if( pkey == PKEY_FileName ) {
      CString sFilename = m_pPhoto->sFilename + m_pPhoto->sExtension;
      return ::InitPropVariantFromString(sFilename, &v);
   }
   if( pkey == PKEY_FileExtension ) {
      return ::InitPropVariantFromString(m_pPhoto->sExtension, &v);
   }
   if( pkey == PKEY_PerceivedType ) {
      return ::InitPropVariantFromInt32(PERCEIVED_TYPE_IMAGE, &v);
   }
   if( pkey == PKEY_DateCreated ) {
      return ::InitPropVariantFromFileTime(&m_pPhoto->ftDateTaken, &v);
   }
   if( pkey == PKEY_DateModified ) {
      return ::InitPropVariantFromFileTime(&m_pPhoto->ftDatePosted, &v);
   }
   if( pkey == PKEY_Photo_DateTaken ) {
      return ::InitPropVariantFromFileTime(&m_pPhoto->ftDateTaken, &v);
   }
   if( pkey == PKEY_Keywords ) {
      HR( _ShellModule.Rest.ReadPhotoExtendedInfo(m_pPhoto) );
      v = m_pPhoto->sKeywords;
      return v.CoerceToCanonicalValue(PKEY_Keywords);
   }
   if( pkey == PKEY_Comment ) {
      HR( _ShellModule.Rest.ReadPhotoExtendedInfo(m_pPhoto) );
      return ::InitPropVariantFromString(m_pPhoto->sDescription, &v);
   }
   if( pkey == PKEY_Media_Publisher ) {
      HR( _ShellModule.Rest.ReadPhotoExtendedInfo(m_pPhoto) );
      return ::InitPropVariantFromString(m_pPhoto->sOwnerUsername, &v);
   }
   if( pkey == PKEY_Media_ContentDistributor ) {
      return ::InitPropVariantFromString(L"Yahoo Flickr", &v);
   }   
   if( pkey == PKEY_Photo_CameraManufacturer ) {
      HR( _ShellModule.Rest.ReadPhotoExifInfo(m_pPhoto) );
      return ::InitPropVariantFromString(m_pPhoto->sExifCameraMake, &v);
   }
   if( pkey == PKEY_Photo_CameraModel ) {
      HR( _ShellModule.Rest.ReadPhotoExifInfo(m_pPhoto) );
      return ::InitPropVariantFromString(m_pPhoto->sExifCameraModel, &v);
   }
   if( pkey == PKEY_Image_HorizontalSize ) {
      HR( _ShellModule.Rest.ReadPhotoExifInfo(m_pPhoto) );
      v = m_pPhoto->sExifImageWidth;
      return v.CoerceToCanonicalValue(PKEY_Image_HorizontalSize);
   }
   if( pkey == PKEY_Image_VerticalSize ) {
      HR( _ShellModule.Rest.ReadPhotoExifInfo(m_pPhoto) );
      v = m_pPhoto->sExifImageHeight;
      return v.CoerceToCanonicalValue(PKEY_Image_VerticalSize);
   }
   if( pkey == PKEY_Image_HorizontalResolution ) {
      HR( _ShellModule.Rest.ReadPhotoExifInfo(m_pPhoto) );
      v = m_pPhoto->sExifXResolution;
      return v.CoerceToCanonicalValue(PKEY_Image_HorizontalResolution);
   }
   if( pkey == PKEY_Image_VerticalResolution ) {
      HR( _ShellModule.Rest.ReadPhotoExifInfo(m_pPhoto) );
      v = m_pPhoto->sExifYResolution;
      return v.CoerceToCanonicalValue(PKEY_Image_VerticalResolution);
   }
   if( pkey == PKEY_Image_BitDepth ) {
      HR( _ShellModule.Rest.ReadPhotoExifInfo(m_pPhoto) );
      v = m_pPhoto->sExifBitsPrSample;
      return v.CoerceToCanonicalValue(PKEY_Image_BitDepth);
   }   
   if( pkey == PKEY_Image_Dimensions ) {
      HR( _ShellModule.Rest.ReadPhotoExifInfo(m_pPhoto) );
      return ::InitPropVariantFromString(m_pPhoto->sExifDimensions, &v);
   }
   if( pkey == PKEY_PropList_TileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.ItemType;System.DateCreated;", &v);
   }
   if( pkey == PKEY_PropList_ExtendedTileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.ItemType;System.DateCreated;", &v);
   }
   if( pkey == PKEY_PropList_PreviewTitle ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.ItemType;", &v);
   }
   if( pkey == PKEY_PropList_PreviewDetails ) {
      return ::InitPropVariantFromString(L"prop:System.Photo.DateTaken;System.Media.Publisher;System.Title;System.Comment;System.Keywords;", &v);
   }
   if( pkey == PKEY_PropList_InfoTip ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.ItemType;System.Photo.DateTaken;System.Media.Publisher;", &v);
   }
   if( pkey == PKEY_PropList_FullDetails ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.DateCreated;System.DateModified;System.Title;System.Comment;System.Media.Publisher;System.Keywords;System.PropGroup.Image;System.Photo.DateTaken;*System.Image.Dimensions;*System.Image.HorizontalSize;*System.Image.VerticalSize;*System.Image.HorizontalResolution;*System.Image.VerticalResolution;*System.Image.BitDepth;System.PropGroup.Camera;*System.Photo.CameraManufacturer;*System.Photo.CameraModel;", &v);
   }
   return CNseBaseItem::GetProperty(pkey, v);
}

HRESULT CFlickrItemPhoto::SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v)
{
   HR( _EnsureFlickrRef() );
   HR( _ShellModule.Rest.SetPhotoProperty(m_pPhoto, pkey, v) );
   HR( _RefreshFolderView() );
   return S_OK;
}

VFS_PROPSTATE CFlickrItemPhoto::GetPropertyState(REFPROPERTYKEY pkey)
{
   if( SUCCEEDED( _EnsureFlickrRef() ) )
   {
      // Some photos you can edit meta-data, some you can not
      if( pkey == PKEY_Title ) {
         return m_pPhoto->bIsOwned ? VFS_PROPSTATE_NORMAL : VFS_PROPSTATE_READONLY;
      }
      if( pkey == PKEY_Comment ) {
         return m_pPhoto->bIsOwned ? VFS_PROPSTATE_NORMAL : VFS_PROPSTATE_READONLY;
      }
      if( pkey == PKEY_Keywords ) {
         return m_pPhoto->bIsOwned ? VFS_PROPSTATE_NORMAL : VFS_PROPSTATE_READONLY;
      }
   }
   return VFS_PROPSTATE_NORMAL;
}

HMENU CFlickrItemPhoto::GetMenu()
{
   return ::LoadMenu(_pModule->GetResourceInstance(), MAKEINTRESOURCE(IDM_PHOTO));
}

HRESULT CFlickrItemPhoto::SetMenuState(const VFS_MENUSTATE& State)
{
   HR( _EnsureFlickrRef() );
   // Disable the ability to "Remove Photo from Imageset" when not applicable
   const TFlickrImageset* pImageset = _FindParentImageset();
   if( pImageset != NULL && (m_pPhoto->bIsOwned || pImageset->Type == FLICKRFS_IMAGESET_FAVOURITES) ) {
      ::EnableMenuItem(State.hMenu, ID_FILE_SENDTO_FAVS, MF_DISABLED|MF_GRAYED);
      ::EnableMenuItem(State.hMenu, ID_FILE_SENDTO_ACCOUNTS, MF_DISABLED|MF_GRAYED);
      switch( pImageset->Type ) {
      case FLICKRFS_IMAGESET_GROUP:
      case FLICKRFS_IMAGESET_PHOTOSET:
      case FLICKRFS_IMAGESET_FAVOURITES:
         ::EnableMenuItem(State.hMenu, ID_FILE_REMOVE, MF_ENABLED);
         break;
      }
   }
   return S_OK;
}

HRESULT CFlickrItemPhoto::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   HR( _EnsureFlickrRef() );
   switch( Cmd.wMenuID ) {
   case ID_FILE_OPEN:
      return LoadAndLaunchFile(Cmd.hWnd, m_pFolder, GetITEMID(), _T("open"));
   case ID_FILE_EDIT:
      return LoadAndLaunchFile(Cmd.hWnd, m_pFolder, GetITEMID(), _T("edit"));
   case ID_FILE_PRINT:
      return LoadAndLaunchFile(Cmd.hWnd, m_pFolder, GetITEMID(), _T("print"));
   case ID_FILE_SENDTO_FAVS: 
      return _ShellModule.Rest.AddPhotoToImageset(FLICKRFS_IMAGESET_FAVOURITES, m_pPhoto);
   case ID_FILE_SENDTO_ACCOUNTS: 
      HR( _ShellModule.Rest.ReadPhotoExtendedInfo(m_pPhoto) );
      HR( _ShellModule.Rest.AddAccount(m_pPhoto->sOwnerUsername) );
      HR( _RefreshRootView() );
      return S_OK;
   case ID_FILE_REMOVE:      
      HR( _ShellModule.Rest.RemovePhotoFromImageset(_FindParentImageset(), m_pPhoto) );
      HR( _RefreshFolderView() );
      return S_OK;
   }   
   return E_NOTIMPL;
}

HRESULT CFlickrItemPhoto::Delete()
{
   HR( _EnsureFlickrRef() );
   HR( _ShellModule.Rest.DeletePhoto(m_pPhoto) );
   HR( _RefreshFolderView() );
   return S_OK;
}

HRESULT CFlickrItemPhoto::GetStream(const VFS_STREAM_REASON& Reason, CNseFileStream** ppFile)
{
   HR( _EnsureFlickrRef() );
   *ppFile = new CFlickrItemFileStream(Reason, m_pFolder, m_pPhoto, _FindParentImageset());
   return *ppFile != NULL ? S_OK : E_OUTOFMEMORY;
}

// Static members

PCITEMID_CHILD CFlickrItemPhoto::GenerateITEMID(const FLICKRFS_PIDL_PHOTO& src)
{
   FLICKRFS_PIDL_PHOTO data = src;
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

PCITEMID_CHILD CFlickrItemPhoto::GenerateITEMID(const TFlickrPhoto* pPhoto)
{
   ATLASSERT(pPhoto);
   FLICKRFS_PIDL_PHOTO data = { 0 };
   data.magic = FLICKRFS_MAGIC_ID_PHOTO;
   wcscpy_s(data.cPhotoID, lengthof(data.cPhotoID), pPhoto->sPhotoID);
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

PCITEMID_CHILD CFlickrItemPhoto::GenerateITEMID(const WIN32_FIND_DATA& wfd)
{
   // When the Shell or our own Framework wants to create a new file we are called with this
   // function. We'll make sure to record the attempt for later queries.
   _ShellModule.Rest.SpawnTempPhoto(wfd);
   // Return PIDL data for Shell
   FLICKRFS_PIDL_PHOTO data = { 0 };
   data.magic = FLICKRFS_MAGIC_ID_PHOTO;
   wcscpy_s(data.cPhotoID, lengthof(data.cPhotoID), wfd.cFileName);
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

// Implementation

HRESULT CFlickrItemPhoto::_EnsureFlickrRef()
{
   // Bind this NSE Item to a real Flickr Photo object
   if( m_pPhoto != NULL ) return S_OK;
   CPidlMemPtr<FLICKRFS_PIDL_PHOTO> pItem = m_pidlItem;
   if( !pItem.IsType(FLICKRFS_MAGIC_ID_PHOTO) ) return E_UNEXPECTED;
   return _ShellModule.Rest.GetPhoto(pItem->cPhotoID, &m_pPhoto);
}

TFlickrImageset* CFlickrItemPhoto::_FindParentImageset() const
{
   CPidlMemPtr<FLICKRFS_PIDL_ACCOUNT> pRoot = m_pidlFolder;
   if( !pRoot.IsType(FLICKRFS_MAGIC_ID_ACCOUNT) ) return NULL;
   CPidlMemPtr<FLICKRFS_PIDL_IMAGESET> pItem = ::ILFindLastID(m_pidlFolder);
   if( !pItem.IsType(FLICKRFS_MAGIC_ID_IMAGESET) ) return NULL;
   TFlickrAccount* pAccount = NULL;
   TFlickrImageset* pImageset = NULL;
   _ShellModule.Rest.GetAccountFromNSID(pRoot->cNSID, &pAccount);
   _ShellModule.Rest.FindImageset(pAccount, pItem->cImagesetID, &pImageset);
   return pImageset;
}

