
#include "stdafx.h"
#include "shfrwkres.h"

#include "ShellFolder.h"
#include "EnumIDList.h"
#include "QueryInfo.h"
#include "DataObject.h"
#include "FileStream.h"
#include "DropTarget.h"
#include "IdentityName.h"
#include "TransferItem.h"
#include "PropertyStore.h"
#include "CommandProvider.h"
#include "CategoryProvider.h"

#pragma warning( disable : 4510 4610 )


//////////////////////////////////////////////////////////////////////
// CShellFolder

HRESULT WINAPI CShellFolder::UpdateRegistry(BOOL bRegister) throw()
{
   // Prepare lookup table for different placements
   CComBSTR bstrExtension;
   bstrExtension.LoadString(IDS_NSE_FILEEXTENSION);
   struct { 
      UINT nRegistryResource;
      CComBSTR bstrName;
      REFKNOWNFOLDERID RefreshFolderId;
   } aLocations[] = 
   {
      /* VFS_LOCATION_JUNCTION */   { IDR_SHELL_LOC_JUNCTION,   bstrExtension,           FOLDERID_Desktop },
      /* VFS_LOCATION_DESKTOP */    { IDR_SHELL_LOC_ROOTED,     L"Desktop",              FOLDERID_Desktop },
      /* VFS_LOCATION_MYCOMPUTER */ { IDR_SHELL_LOC_ROOTED,     L"MyComputer",           FOLDERID_ComputerFolder },
      /* VFS_LOCATION_USERFILES */  { IDR_SHELL_LOC_ROOTED,     L"UserFiles",            FOLDERID_UsersFiles },      
      /* VFS_LOCATION_NETHOOD */    { IDR_SHELL_LOC_ROOTED,     L"NetworkNeighborhood",  FOLDERID_NetworkFolder },
      /* VFS_LOCATION_REMOTE */     { IDR_SHELL_LOC_ROOTED,     L"RemoteComputer",       FOLDERID_NetworkFolder },
      /* VFS_LOCATION_PRINTERS */   { IDR_SHELL_LOC_ROOTED,     L"PrintersAndFaxes",     FOLDERID_PrintersFolder },
   };
   int iLocationIndex = _ShellModule.GetConfigInt(VFS_INT_LOCATION);
   ATLASSERT(iLocationIndex<lengthof(aLocations));
   // We define a couple of meta-tokens for the registry script
   // so it is more generic.
   WCHAR wszTemp[50];
   CComBSTR bstrCLSID = CLSID_ShellFolder;
   CComBSTR bstrSendToCLSID = CLSID_SendTo;
   CComBSTR bstrPreviewCLSID = CLSID_Preview;
   CComBSTR bstrDropTargetCLSID = CLSID_DropTarget;
   CComBSTR bstrContextMenuCLSID = CLSID_ContextMenu;   
   CComBSTR bstrPropertySheetCLSID = CLSID_PropertySheet;
   CComBSTR bstrLocation = aLocations[iLocationIndex].bstrName;
   CComBSTR bstrProjectName, bstrDisplayName, bstrDescription, bstrUrlProtocol, bstrAttribs, bstrPreviewHost;
   bstrProjectName.LoadString(IDS_NSE_PROJNAME);
   bstrDisplayName.LoadString(IDS_NSE_DISPLAYNAME);
   bstrDescription.LoadString(IDS_NSE_DESCRIPTION);
   bstrUrlProtocol.LoadString(IDS_NSE_URLPROTOCOL);
   bstrPreviewHost = L"{6D2B5079-2F0B-48DD-AB7F-97CEC514D30B}";
   // 32bit PreviewHost under 64bit Windows
   BOOL bIsWOW64 = FALSE; ::IsWow64Process(::GetCurrentProcess(), &bIsWOW64);
   if( bIsWOW64 ) bstrPreviewHost = L"{534A1E02-D58F-44F0-B58B-36CBED287C7C}";
   // Format Root attributes
   ::wsprintf(wszTemp, L"&H%08X", _ShellModule.GetConfigInt(VFS_INT_SHELLROOT_SFGAO)); 
   bstrAttribs = wszTemp;
   // Format Location and find its associated registry script
   _ATL_REGMAP_ENTRY regMapEntries[] =
   {
      { L"CLSID", bstrCLSID },
      { L"PROJECTNAME", bstrProjectName },
      { L"DISPLAYNAME", bstrDisplayName },
      { L"DESCRIPTION", bstrDescription },
      { L"FOLDERATTRIBS", bstrAttribs },
      { L"LOCATION", bstrLocation },
      { L"URLPROTOCOL", bstrUrlProtocol },
      { L"PREVIEWHOST", bstrPreviewHost },
      { L"SENDTOCLSID", bstrSendToCLSID },
      { L"PREVIEWCLSID", bstrPreviewCLSID },
      { L"DROPTARGETCLSID", bstrDropTargetCLSID },
      { L"CONTEXTMENUCLSID", bstrContextMenuCLSID },
      { L"PROPERTYSHEETCLSID", bstrPropertySheetCLSID },
      { NULL, NULL }
   };
   HRESULT Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELLFOLDER, bRegister, regMapEntries);
   if( SUCCEEDED(Hr) ) {
      Hr = _AtlModule.UpdateRegistryFromResource(aLocations[iLocationIndex].nRegistryResource, bRegister, regMapEntries);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_SENDTO) ) {
      Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELL_ROOT_SENDTO, bRegister, regMapEntries);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_PREVIEW) ) {
      Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELL_ROOT_PREVIEW, bRegister, regMapEntries);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_SHELLNEW) ) {
      Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELL_ROOT_SHELLNEW, bRegister, regMapEntries);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_PROPSHEET) ) {
      Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELL_ROOT_PROPSHEET, bRegister, regMapEntries);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_DROPTARGET) ) {
      Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELL_ROOT_DROPTARGET, bRegister, regMapEntries);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_CONTEXTMENU) ) {
      Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELL_ROOT_CONTEXTMENU, bRegister, regMapEntries);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_CUSTOMSCRIPT) ) {
      Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELL_ROOT_CUSTOMSCRIPT, bRegister, regMapEntries);
   }   
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_PROPERTIES) ) {
      Hr = UpdatePropertySchemaFromResource(IDR_SHELL_ROOT_PROPERTIES, bRegister);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_URLPROTOCOL) ) {
      ATLASSERT(bstrUrlProtocol.Length()>0);
      Hr = _AtlModule.UpdateRegistryFromResource(IDR_SHELL_ROOT_URLPROTOCOL, bRegister, regMapEntries);
   }   
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_SENDTO) ) {
      ATLASSERT(_ShellModule.GetConfigInt(VFS_INT_LOCATION)!=VFS_LOCATION_JUNCTION);
      Hr = UpdateSendToFromProject(bstrDisplayName, bstrProjectName, bRegister);
   }
   if( SUCCEEDED(Hr) && _ShellModule.GetConfigBool(VFS_INSTALL_STARTMENU_LINK) ) {
      ATLASSERT(_ShellModule.GetConfigInt(VFS_INT_LOCATION)!=VFS_LOCATION_JUNCTION);
      Hr = UpdateStartMenuLink(bstrDisplayName, bstrDescription, aLocations[iLocationIndex].RefreshFolderId, bRegister);
   }
   // Refresh shell...
   CPidl pidlKnownFolder;
   pidlKnownFolder.CreateFromKnownFolder(aLocations[iLocationIndex].RefreshFolderId);
   ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlKnownFolder);
   ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
   return Hr;
}

HRESULT CShellFolder::FinalConstruct()
{
   ATLTRACE(L"CShellFolder::FinalConstruct\n");
   m_hwndOwner = NULL;
   m_hMenu = m_hContextMenu = NULL;
   return S_OK;
}

void CShellFolder::FinalRelease()
{
   ATLTRACE(L"CShellFolder::FinalRelease\n");
   if( ::IsMenu(m_hMenu) ) ::DestroyMenu(m_hMenu);
}

// IPersist

STDMETHODIMP CShellFolder::GetClassID(CLSID* pclsid)
{
   ATLTRACE(L"CShellFolder::GetClassID\n");
   *pclsid = GetObjectCLSID();
   return S_OK;
}

// IPersistFolder

STDMETHODIMP CShellFolder::Initialize(PCIDLIST_ABSOLUTE pidl)
{
   ATLTRACE(L"CShellFolder::Initialize\n");
   // Spawn a file-system
   CRefPtr<CNseFileSystem> spFS;
   HR( _ShellModule.CreateFileSystem(pidl, &spFS) );
   m_spFS = spFS;
   // Clear data members
   m_pidlRoot = pidl;
   m_pidlParent.Delete();
   m_pidlFolder.Delete();
   m_pidlMonitor = m_pidlRoot;
   // Generate an item for the root folder
   m_spFolderItem = m_spFS->GenerateRoot(this);
   return S_OK;
}

// IPersistFolder2

STDMETHODIMP CShellFolder::GetCurFolder(PIDLIST_ABSOLUTE* ppidl)
{
   ATLTRACE(L"CShellFolder::GetCurFolder\n");
   return m_pidlMonitor.CopyTo(ppidl);
}

// IPersistFolder3

STDMETHODIMP CShellFolder::InitializeEx(IBindCtx* pbc, PCIDLIST_ABSOLUTE pidlRoot, const PERSIST_FOLDER_TARGET_INFO* pPFTI)
{
   ATLTRACE(L"CShellFolder::InitializeEx\n");
   return Initialize(pidlRoot);
}

STDMETHODIMP CShellFolder::GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO* pPFTI)
{
   ::ZeroMemory(pPFTI, sizeof(PERSIST_FOLDER_TARGET_INFO));
   ATLTRACENOTIMPL(L"CShellFolder::GetFolderTargetInfo");
}

// IPersistIDlist

STDMETHODIMP CShellFolder::SetIDList(PCIDLIST_ABSOLUTE pidl)
{
   ATLTRACENOTIMPL(L"CShellFolder::SetIDList");
}

STDMETHODIMP CShellFolder::GetIDList(PIDLIST_ABSOLUTE* ppidl)
{
   ATLTRACE(L"CShellFolder::GetIDList\n");
   return m_pidlMonitor.CopyTo(ppidl);
}

// IShellFolder

STDMETHODIMP CShellFolder::ParseDisplayName(HWND hwnd, IBindCtx* pbc, PWSTR pszDisplayName, ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes)
{
   ATLTRACE(L"CShellFolder::ParseDisplayName  name='%ws' pbc=%p\n", pszDisplayName, pbc);
   if( pchEaten != NULL ) *pchEaten = 0;
   if( pszDisplayName == NULL ) pszDisplayName = L"";
   // Parse URL Protocol?
   if( _ShellModule.GetConfigBool(VFS_INSTALL_URLPROTOCOL) ) {
      static CComBSTR bstrUrlProtocol;
      if( !bstrUrlProtocol ) bstrUrlProtocol.LoadString(IDS_NSE_URLPROTOCOL);
      if( _wcsnicmp(pszDisplayName, bstrUrlProtocol, bstrUrlProtocol.Length()) == 0 && pszDisplayName[ bstrUrlProtocol.Length() ] == ':' ) {
         pszDisplayName += bstrUrlProtocol.Length() + 1;
         if( *pszDisplayName == '/' ) ++pszDisplayName;
         if( *pszDisplayName == '/' ) ++pszDisplayName;
      }
   }
   // Extract first component of display string (may be a path)
   WCHAR wszComponent[MAX_PATH] = { 0 };
   wcscpy_s(wszComponent, lengthof(wszComponent), pszDisplayName);
   if( wcschr(wszComponent, '\\') != NULL ) *wcschr(wszComponent, '\\') = '\0';
   LPWSTR pwstrNext = wcschr(pszDisplayName, '\\');
   // Find the file from the parsing/editing name; we may not have refreshed the cache
   // lately so a deleted file may or may not be found.
   CNseItemPtr spItem;
   m_spFolderItem->GetChild(wszComponent, SHGDN_FOREDITING, &spItem);
   if( pbc != NULL && pwstrNext == NULL ) {
      // During SaveAs dialog the Shell will eventually call us with
      // binding option STGM_CREATE. Later the Shell might query IIdentityName
      // for this item to get the real filename. Other STGM_XXX options are also
      // possible when the Shell really requests a disk file.
      BIND_OPTS Opts = { sizeof(Opts) };
      if( SUCCEEDED( pbc->GetBindOptions(&Opts) ) ) {
         HRESULT Hr = _ParseDisplayNameWithBind(spItem, pszDisplayName, pbc, Opts);
         if( FAILED(Hr) ) return Hr;
      }
   }
   if( spItem == NULL ) {
      ATLTRACE(L"CShellFolder::ParseDisplayName - failed\n");
      // An undocumented feature is that our IBindCtx::GetBindOptions() trick above
      // only works if we return ERROR_FILE_NOT_FOUND for the regular query first.
      return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   }
   CPidl pidl = spItem->GetITEMID();
   if( pwstrNext != NULL ) {
      // There are more segments in the path...
      CPidl pidlNext;
      CComPtr<IShellFolder> spShellFolder;
      HR( BindToObject(pidl, pbc, IID_PPV_ARGS(&spShellFolder)) );
      HRESULT hResult = ( spShellFolder->ParseDisplayName(hwnd, pbc, ++pwstrNext, pchEaten, &pidlNext, pdwAttributes) );
	  if (S_OK != hResult) {
		  return hResult;
	  }
      if( pchEaten != NULL ) *pchEaten += 1;
      pidl += pidlNext;
   }
   else {
      // This is the last PIDL
      if( pdwAttributes != NULL ) GetAttributesOf(1, (PCUITEMID_CHILD_ARRAY) &pidl.m_p, pdwAttributes);
   }
   if( pchEaten != NULL ) *pchEaten += (ULONG) wcslen(wszComponent);
   *ppidl = pidl.Detach();
   return S_OK;
}

STDMETHODIMP CShellFolder::EnumObjects(HWND hwnd, SHCONTF grfFlags, IEnumIDList** ppEnumIDList)
{
   ATLTRACE(L"CShellFolder::EnumObjects  hwnd=%X flags=0x%X [SHCONT_%s]\n", hwnd, grfFlags, DbgGetSHCONTF(grfFlags));
   *ppEnumIDList = NULL;
   CComObject<CEnumIDList>* pEnum = NULL;
   HR( CComObject<CEnumIDList>::CreateInstance(&pEnum) );
   CComPtr<IUnknown> spKeepAlive = pEnum->GetUnknown();
   HR( pEnum->Init(this, hwnd, grfFlags) );
   return pEnum->QueryInterface(ppEnumIDList);
}

STDMETHODIMP CShellFolder::BindToObject(PCUIDLIST_RELATIVE pidl, IBindCtx* pbc, REFIID riid, LPVOID* ppRetVal)
{
   ATLTRACE(L"CShellFolder::BindToObject  riid=%s\n", DbgGetIID(riid));
   if( !::ILIsChild(pidl) )
   {
      CComPtr<IShellFolder> spShellFolder;
      PCUITEMID_CHILD pidlChild = NULL;
      HR( ::SHBindToFolderIDListParent(this, pidl, IID_PPV_ARGS(&spShellFolder), &pidlChild) );
      return spShellFolder->BindToObject(pidlChild, pbc, riid, ppRetVal);
   }
   // At this point, we are left with a child PIDL only
   ATLASSERT(::ILIsChild(pidl));
   PCUITEMID_CHILD pidlItem = static_cast<PCUITEMID_CHILD>(pidl);
   // Ask NSE Item first...
   if( _ShellModule.GetConfigBool(VFS_HAVE_OBJECTOF) ) {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      HRESULT Hr = spItem->GetObjectOf(VFS_OBJECTOF_BIND, NULL, riid, ppRetVal);
      if( SUCCEEDED(Hr) ) return Hr;
   }
   // Attach to child folder?
   if( riid == IID_IShellFolder || riid == IID_IShellFolder2 || riid == IID_IStorage || riid == IID_IObjectProvider )
   {
      CRefPtr<CShellFolder> spFolder;
      HR( BindToFolder(pidlItem, spFolder) );
      return spFolder->QueryInterface(riid, ppRetVal);
   }
   // Process standard objects...
   if( riid == IID_IStream )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      if( IsBitSet(spItem->GetSFGAOF(SFGAO_FOLDER), SFGAO_FOLDER) ) return E_NOINTERFACE;
      if( !IsBitSet(spItem->GetSFGAOF(SFGAO_STREAM), SFGAO_STREAM) ) return E_NOINTERFACE;
      CComObject<CFileStream>* pStream = NULL;
      HR( CComObject<CFileStream>::CreateInstance(&pStream) );
      CComPtr<IUnknown> spKeepAlive = pStream->GetUnknown();
      HR( pStream->Init(this, pidlItem) );
      return pStream->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_IMoniker )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      if( IsBitSet(spItem->GetSFGAOF(SFGAO_FOLDER), SFGAO_FOLDER) ) return E_NOINTERFACE;
      if( !IsBitSet(spItem->GetSFGAOF(SFGAO_CANMONIKER), SFGAO_CANMONIKER) ) return E_NOINTERFACE;
      return spItem->GetMoniker(reinterpret_cast<IMoniker**>(ppRetVal));
   }
   if( riid == IID_IPropertyStoreFactory )
   {      
      CComObject<CPropertyStoreFactory>* pPsFactory = NULL;
      HR( CComObject<CPropertyStoreFactory>::CreateInstance(&pPsFactory) );
      CComPtr<IUnknown> spKeepAlive = pPsFactory->GetUnknown();
      HR( pPsFactory->Init(this, pidlItem) );
      return pPsFactory->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_IPropertyStore )
   {      
      CComObject<CPropertyStoreFactory>* pPsFactory = NULL;
      HR( CComObject<CPropertyStoreFactory>::CreateInstance(&pPsFactory) );
      CComPtr<IUnknown> spKeepAlive = pPsFactory->GetUnknown();
      HR( pPsFactory->Init(this, pidlItem) );
      return pPsFactory->GetPropertyStore(GPS_DEFAULT, NULL, riid, ppRetVal);
   }
   if( riid == IID_IPropertySetStorage )
   {
      CComObject<CPropertyStoreFactory>* pPsFactory = NULL;
      HR( CComObject<CPropertyStoreFactory>::CreateInstance(&pPsFactory) );
      CComPtr<IUnknown> spKeepAlive = pPsFactory->GetUnknown();
      HR( pPsFactory->Init(this, pidlItem) );
      CComPtr<IPropertyStore> spStore;
      HR( pPsFactory->GetPropertyStore(GPS_DELAYCREATION, NULL, IID_PPV_ARGS(&spStore)) );
      return ::PSCreateAdapterFromPropertyStore(spStore, riid, ppRetVal);
   }
   if( riid == IID_IExtractImage )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      return spItem->GetExtractIcon(riid, ppRetVal);
   }
   if( riid == IID_IThumbnailProvider )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      return spItem->GetThumbnail(riid, ppRetVal);
   }
   if( riid == IID_ITransferMediumItem )
   {
      CComObject<CTransferMediumItem>* pTransferMediumItem = NULL;
      HR( CComObject<CTransferMediumItem>::CreateInstance(&pTransferMediumItem) );
      CComPtr<IUnknown> spKeepAlive = pTransferMediumItem->GetUnknown();
      HR( pTransferMediumItem->Init(this, pidlItem) );
      return pTransferMediumItem->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_IIdentityName )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      CComObject<CIdentityName>* pIdentityName = NULL;
      HR( CComObject<CIdentityName>::CreateInstance(&pIdentityName) );
      CComPtr<IUnknown> spKeepAlive = pIdentityName->GetUnknown();
      if( FAILED( pIdentityName->Init(this, pidlItem) ) ) return E_FAIL;
      return pIdentityName->QueryInterface(riid, ppRetVal);
   }
   ATLTRACE(L"CShellFolder::BindToObject - failed\n");
   return E_NOINTERFACE;
}

STDMETHODIMP CShellFolder::BindToStorage(PCUIDLIST_RELATIVE pidl, IBindCtx* pbc, REFIID riid, LPVOID* ppRetVal)
{
   ATLTRACE(L"CShellFolder::BindToStorage\n", this);
   // Ask NSE Item first...
   if( _ShellModule.GetConfigBool(VFS_HAVE_OBJECTOF) ) {
      PCUITEMID_CHILD pidlItem = static_cast<PCUITEMID_CHILD>(pidl);
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      HRESULT Hr = spItem->GetObjectOf(VFS_OBJECTOF_STORAGE, NULL, riid, ppRetVal);
      if( SUCCEEDED(Hr) ) return Hr;
   }
   // Fall back to BindToObject call...
   return BindToObject(pidl, pbc, riid, ppRetVal);
}

STDMETHODIMP CShellFolder::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
   ATLTRACE(L"CShellFolder::CompareIDs  column=%u flags=0x%X\n", LOWORD(lParam), HIWORD(lParam));
   CRefPtr<CShellFolder> spFolder = this;
   BOOL bHaveUniqueNames = _ShellModule.GetConfigBool(VFS_HAVE_UNIQUE_NAMES);
   UINT iSortColumn = LOWORD(lParam);
   UINT uColumnFlags = HIWORD(lParam);
   VFS_COLUMNINFO Column = { 0 };
   HR( m_spFolderItem->GetColumnInfo(iSortColumn, Column) );
   UINT nMaxSpanColumns = 3; 
   PROPERTYKEY SortKey = Column.pkey;
   if( IsBitSet(uColumnFlags, SHCIDS_ALLFIELDS) ) nMaxSpanColumns = 6, bHaveUniqueNames = FALSE;
   if( IsBitSet(uColumnFlags, SHCIDS_CANONICALONLY) && bHaveUniqueNames ) SortKey = PKEY_ParsingName;
   while( !::ILIsEmpty(pidl1) || !::ILIsEmpty(pidl2) ) 
   {
      if( ::ILIsEmpty(pidl1) ) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(-1));
      if( ::ILIsEmpty(pidl2) ) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(1));
      CNseItemPtr spItem1 = spFolder->GenerateChildItem(static_cast<PCUITEMID_CHILD>(pidl1));
      CNseItemPtr spItem2 = spFolder->GenerateChildItem(static_cast<PCUITEMID_CHILD>(pidl2));
      if( spItem1 == NULL || spItem2 == NULL ) return E_FAIL;
      // Items may be sorted by DIRECTORY attribute first
      if( !IsBitSet(Column.dwFlags, SHCOLSTATE_NOSORTBYFOLDERNESS) ) {
         CComPropVariant vfd1, vfd2;
         spItem1->GetProperty(PKEY_FileAttributes, vfd1);
         spItem2->GetProperty(PKEY_FileAttributes, vfd2);
         vfd1.ulVal &= FILE_ATTRIBUTE_DIRECTORY;
         vfd2.ulVal &= FILE_ATTRIBUTE_DIRECTORY;
         int iRes = vfd2.CompareTo(vfd1);
         if( iRes != 0 ) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(iRes));
      }
      // Items are sorted by chosen column
      CComPropVariant v1, v2;
      spItem1->GetProperty(SortKey, v1);
      spItem2->GetProperty(SortKey, v2);
      int iRes = v1.CompareTo(v2);
      if( iRes != 0 ) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(iRes));
      // Finally, we sort items by the next few remaining columns in order.
      // We do this especially if the primary sort-column is not the 1st column (assumed
      // to be the items' Title), so a sort on Type will use the Title as its second
      // sorting field.
      if( !bHaveUniqueNames || iSortColumn != 0 ) {
         for( UINT iColumn = 0; iColumn < nMaxSpanColumns; iColumn++ ) {
            if( iSortColumn == iColumn ) continue;
            VFS_COLUMNINFO Column2 = { 0 };
            if( FAILED( m_spFolderItem->GetColumnInfo(iColumn, Column2) ) ) continue;
            if( !IsBitSet(Column2.dwFlags, SHCOLSTATE_ONBYDEFAULT) ) continue;
            CComPropVariant vc1, vc2;
            spItem1->GetProperty(Column2.pkey, vc1);
            spItem2->GetProperty(Column2.pkey, vc2);
            iRes = vc1.CompareTo(vc2);
            if( iRes != 0 ) return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(iRes));
         }
      }
      // If this is a relative PIDL, traverse next sub-folder
      if( !::ILIsChild(pidl1) ) {
         CRefPtr<CShellFolder> spChildFolder;
         CPidl pidlChild( (PCUITEMID_CHILD) pidl1 );
         HR( spFolder->BindToFolder(pidlChild.GetItem(0), spChildFolder) );
         spFolder = spChildFolder;
      }
      // Ready for next PIDL level
      pidl1 = ::ILGetNext(pidl1);
      pidl2 = ::ILGetNext(pidl2);
   }
   return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

STDMETHODIMP CShellFolder::CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID* ppRetVal)
{
   ATLTRACE(L"CShellFolder::CreateViewObject  riid=%s\n", DbgGetIID(riid));
   ATLASSERT(m_spFolderItem);
   if( m_spFolderItem == NULL ) return E_UNEXPECTED;
   // Ask Folder NSE Item first...
   if( _ShellModule.GetConfigBool(VFS_HAVE_OBJECTOF) ) {
      HRESULT Hr = m_spFolderItem->GetObjectOf(VFS_OBJECTOF_VIEW, hwndOwner, riid, ppRetVal);
      if( SUCCEEDED(Hr) ) return Hr;
   }
   // Process standard objects...
   if( riid == IID_IShellView )
   {
      m_hwndOwner = hwndOwner;
      SFV_CREATE sfvData = { sizeof(sfvData), 0 };
      CComQIPtr<IShellFolder> spFolder = GetUnknown();
      sfvData.psfvcb = this;
      sfvData.pshf = spFolder;
      return ::SHCreateShellFolderView(&sfvData, (IShellView**) ppRetVal);
   }
   if( riid == IID_IDropTarget ) 
   {
      CComObject<CDropTarget>* pDropTarget = NULL;
      HR( CComObject<CDropTarget>::CreateInstance(&pDropTarget) );
      CComPtr<IUnknown> spKeepAlive = pDropTarget->GetUnknown();
      HR( pDropTarget->Init(this, NULL, hwndOwner) );
      return pDropTarget->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_IContextMenu )
   {
      if( ::IsMenu(m_hMenu) ) ::DestroyMenu(m_hMenu);
      m_hMenu = m_spFolderItem->GetMenu();
      m_hContextMenu = ::GetSubMenu(m_hMenu, _T("ViewMenu"));
      DEFCONTEXTMENU dcm = { hwndOwner, static_cast<IContextMenuCB*>(this), m_pidlMonitor, static_cast<IShellFolder*>(this), 0, NULL, NULL, 0, NULL };
      return ::SHCreateDefaultContextMenu(&dcm, riid, ppRetVal);
   }
   if( riid == IID_IExplorerCommandProvider )
   {
      CComObject<CExplorerCommandProvider>* pCommandProvider = NULL;
      HR( CComObject<CExplorerCommandProvider>::CreateInstance(&pCommandProvider) );
      CComPtr<IUnknown> spKeepAlive = pCommandProvider->GetUnknown();     
      HRESULT Hr = pCommandProvider->Init(this, hwndOwner, m_spFolderItem->GetMenu());
      if( FAILED(Hr) ) return Hr;
      return pCommandProvider->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_ICategoryProvider )
   {
      CComObject<CCategoryProvider>* pCategoryProvider = NULL;
      HR( CComObject<CCategoryProvider>::CreateInstance(&pCategoryProvider) );
      CComPtr<IUnknown> spKeepAlive = pCategoryProvider->GetUnknown();
      HR( pCategoryProvider->Init(this) );
      return pCategoryProvider->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_ITransferSource )
   {
      CComObject<CTransferSource>* pTransferSource = NULL;
      HR( CComObject<CTransferSource>::CreateInstance(&pTransferSource) );
      CComPtr<IUnknown> spKeepAlive = pTransferSource->GetUnknown();
      HR( pTransferSource->Init(this) );
      return pTransferSource->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_ITransferDestination )
   {
      CComObject<CTransferDestination>* pTransferDestination = NULL;
      HR( CComObject<CTransferDestination>::CreateInstance(&pTransferDestination) );
      CComPtr<IUnknown> spKeepAlive = pTransferDestination->GetUnknown();
      HR( pTransferDestination->Init(this) );
      return pTransferDestination->QueryInterface(riid, ppRetVal);
   }
   if( riid == CLSID_ShellFolder )
   {
      // Allows internal objects to query the C++ implementation of this IShellFolder instance.
      // This violates basic COM sense, but we need this as a programming shortcut. It restricts
      // our Shell Extension to only work as an in-proc component.
      AddRef(); 
      *ppRetVal = this;
      return S_OK;
   }
   return QueryInterface(riid, ppRetVal);
}

STDMETHODIMP CShellFolder::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY rgpidl, SFGAOF* rgfInOut)
{
   ATLTRACE(L"CShellFolder::GetAttributesOf  count=%u mask=0x%X [SFGAO_%s]\n", cidl, *rgfInOut, DbgGetSFGAOF(*rgfInOut));
   DWORD dwOrigAttributes = *rgfInOut;
   if( *rgfInOut == 0 ) *rgfInOut = (SFGAOF)(~SFGAO_VALIDATE);
   // Wants to validate folder items?
   if( IsBitSet(dwOrigAttributes, SFGAO_VALIDATE) ) m_spFolderItem->Refresh(VFS_REFRESH_VALIDATE);
   // Gather attribute information for child items...
   for( UINT i = 0; i < cidl; i++ ) {
      CNseItemPtr spItem = GenerateChildItem(rgpidl[i]);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      *rgfInOut &= spItem->GetSFGAOF(dwOrigAttributes);
      // Must honour SFGAO_VALIDATE flag: we need to trigger an actual lookup rather
      // than possibly just fetching information out of the PIDL structure.
      if( IsBitSet(dwOrigAttributes, SFGAO_VALIDATE) ) {
         CNseItemPtr spTemp;
         const WIN32_FIND_DATA wfd = spItem->GetFindData();
         HRESULT Hr = m_spFolderItem->GetChild(wfd.cFileName, SHGDN_FORPARSING, &spTemp);
         if( FAILED(Hr) ) return Hr;
      }
   }
   return S_OK;
}

STDMETHODIMP CShellFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY rgpidl, REFIID riid, UINT* rgfReserved, LPVOID* ppRetVal)
{
   ATLTRACE(L"CShellFolder::GetUIObjectOf  count=%u riid=%s\n", cidl, DbgGetIID(riid));
   *ppRetVal = NULL;
   if( cidl == 0 ) return E_INVALIDARG;
   if( rgpidl == NULL ) return E_INVALIDARG;
   PCUITEMID_CHILD pidlItem = static_cast<PCUITEMID_CHILD>(rgpidl[0]);
   // Ask NSE Item first...
   if( _ShellModule.GetConfigBool(VFS_HAVE_OBJECTOF) ) {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      HRESULT Hr = spItem->GetObjectOf(VFS_OBJECTOF_UI, hwndOwner, riid, ppRetVal);
      if( SUCCEEDED(Hr) ) return Hr;
   }
   // Process standard objects...
   if( riid == IID_IDataObject )
   {
      CComObject<CDataObject>* pDataObject = NULL;
      HR( CComObject<CDataObject>::CreateInstance(&pDataObject) );
      CComPtr<IUnknown> spKeepAlive = pDataObject->GetUnknown();
      CComPtr<IDataObject> spMaster;
      HR( ::SHCreateDataObject(m_pidlMonitor, cidl, rgpidl, pDataObject, IID_PPV_ARGS(&spMaster)) );
      HR( pDataObject->Init(this, hwndOwner, spMaster, cidl, rgpidl) );
      HRESULT Hr = m_spFolderItem->SetDataObject(spMaster);
      if( FAILED(Hr) && Hr != E_NOTIMPL ) return Hr;
      return spMaster->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_IExtractIconA || riid == IID_IExtractIconW || riid == IID_IExtractImage )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      return spItem->GetExtractIcon(riid, ppRetVal);
   }
   if( riid == IID_IDropTarget )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      if( !IsBitSet(spItem->GetSFGAOF(SFGAO_DROPTARGET), SFGAO_DROPTARGET) ) return E_FAIL;
      CComObject<CDropTarget>* pDropTarget = NULL;
      HR( CComObject<CDropTarget>::CreateInstance(&pDropTarget) );
      CComPtr<IUnknown> spKeepAlive = pDropTarget->GetUnknown();
      HR( pDropTarget->Init(this, pidlItem, hwndOwner) );
      return pDropTarget->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_IContextMenu )
   {
      // BUG: We don't support merging of menus for multiple items, but the menu
      //      can disable its menuitems if needed.
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      if( ::IsMenu(m_hMenu) ) ::DestroyMenu(m_hMenu);
      m_hMenu = spItem->GetMenu();
      m_hContextMenu = ::GetSubMenu(m_hMenu, _T("ContextMenu"));
      DEFCONTEXTMENU dcm = { hwndOwner, static_cast<IContextMenuCB*>(this), m_pidlMonitor, static_cast<IShellFolder*>(this), cidl, rgpidl, NULL, 0, NULL };
      return ::SHCreateDefaultContextMenu(&dcm, riid, ppRetVal);
   }
   if( riid == IID_IQueryInfo && _ShellModule.GetConfigBool(VFS_HAVE_INFOTIPS) )
   {
      CComObject<CQueryInfo>* pQueryInfo = NULL;
      HR( CComObject<CQueryInfo>::CreateInstance(&pQueryInfo) );
      CComPtr<IUnknown> spKeepAlive = pQueryInfo->GetUnknown();
      HR( pQueryInfo->Init(this, pidlItem) );
      return pQueryInfo->QueryInterface(riid, ppRetVal);
   }
   if( riid == IID_IPreviewHandler )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      return spItem->GetPreview(riid, ppRetVal);
   }
   if( riid == IID_IQueryAssociations )
   {
      CNseItemPtr spItem = GenerateChildItem(pidlItem);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      CComBSTR bstrCLSID = GetObjectCLSID();
      ASSOCIATIONELEMENT rgAssoc[] = { { ASSOCCLASS_CLSID_STR, NULL, bstrCLSID }, { ASSOCCLASS_FOLDER, NULL, NULL } };
      return ::AssocCreateForClasses(rgAssoc, spItem->IsFolder() ? 2U : 1U, riid, ppRetVal);
   }
   ATLTRACE(L"CShellFolder::GetUIObjectOf - failed\n");
   return E_FAIL;
}

STDMETHODIMP CShellFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* psrName)
{
   ATLTRACE(L"CShellFolder::GetDisplayNameOf  flags=0x%X [SHGDN_%s]\n", uFlags, DbgGetSHGDNF(uFlags));
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   // Ask NSE Item first...
   if( _ShellModule.GetConfigBool(VFS_HAVE_NAMEOF) ) {
      if( spItem->GetNameOf(uFlags, &(psrName->pOleStr)) == S_OK ) {
         psrName->uType = STRRET_WSTR;
         return S_OK;
      }
   }
   WCHAR wszName[300] = { 0 };
   const WIN32_FIND_DATA wfd = spItem->GetFindData();
   // This is part of the hack to get the SaveAs dialog working.
   // We have redirected the file to a temporary file in the %TEMP% folder.
   if( IsBitSet(uFlags, SHGDN_FORPARSING) && wfd.cAlternateFileName[1] == VFS_HACK_SAVEAS_JUNCTION && m_spFS->m_wszOpenSaveAsFilename[0] != '\0' ) {
      return StrToSTRRET(IsBitSet(uFlags, SHGDN_INFOLDER) ? ::PathFindFileName(m_spFS->m_wszOpenSaveAsFilename) : m_spFS->m_wszOpenSaveAsFilename, psrName);
   }
   // Needs parsing- or display-name?
   bool bNeedsParsingName = IsBitSet(uFlags, SHGDN_FORPARSING);
   if( uFlags == (SHGDN_FORADDRESSBAR|SHGDN_FORPARSING) ) bNeedsParsingName = false;
   // Asking for the full pathname?
   if( IsBitSet(uFlags, SHGDN_FORPARSING) && !IsBitSet(uFlags, SHGDN_INFOLDER) ) {
      CCoTaskString strFolder;
      HR( ::SHGetNameFromIDList(m_pidlMonitor, bNeedsParsingName ? SIGDN_DESKTOPABSOLUTEPARSING : SIGDN_DESKTOPABSOLUTEEDITING, &strFolder) );
      wcscat_s(wszName, lengthof(wszName), strFolder);
      wcscat_s(wszName, lengthof(wszName), L"\\");
   }
   // Append item's name too
   REFPROPERTYKEY pkey = bNeedsParsingName ? PKEY_ParsingName : PKEY_ItemNameDisplay;
   CComPropVariant v;
   HR( spItem->GetProperty(pkey, v) );
   ATLASSERT(v.vt==VT_LPWSTR);
   wcscat_s(wszName, lengthof(wszName), v.pwszVal);
   return StrToSTRRET(wszName, psrName);
}

STDMETHODIMP CShellFolder::SetNameOf(HWND hwnd, PCUITEMID_CHILD pidl, LPCWSTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut)
{
   ATLTRACE(L"CShellFolder::SetNameOf  flags=0x%X [SHGDN_%s]\n", uFlags, DbgGetSHGDNF(uFlags));
   if( pszName == NULL ) return E_INVALIDARG;
   if( wcslen(pszName) == 0 ) return AtlHresultFromWin32(ERROR_FILENAME_EXCED_RANGE);
   // Do the rename of the child item
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   TCHAR wszOutputName[MAX_PATH] = { 0 };
   wcscpy_s(wszOutputName, lengthof(wszOutputName), pszName);
   HR( spItem->Rename(pszName, wszOutputName) );
   // Lookup the new item and return it as the result
   if( ppidlOut != NULL ) {
      CNseItemPtr spNewItem;
      HR( m_spFolderItem->GetChild(wszOutputName, SHGDN_FORPARSING, &spNewItem) );
      *ppidlOut = ::ILCloneChild( spNewItem->GetITEMID() );
      // Notify Shell directly about the rename operation...
      CPidl pidlOld = m_pidlMonitor + pidl;
      CPidl pidlNew = m_pidlMonitor + spNewItem->GetITEMID();
      ::SHChangeNotify(spNewItem->IsFolder() ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, SHCNF_IDLIST | SHCNF_FLUSH, pidlOld, pidlNew);
   }
   ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, m_pidlMonitor);
   return S_OK;
}

// IShellFolder2

STDMETHODIMP CShellFolder::GetDefaultSearchGUID(GUID* pguid)
{
   ATLTRACENOTIMPL(L"CShellFolder::GetDefaultSearchGUID\n");
}

STDMETHODIMP CShellFolder::EnumSearches(IEnumExtraSearch** ppenum)
{
   ATLTRACENOTIMPL(L"CShellFolder::EnumSearches\n");
}

STDMETHODIMP CShellFolder::GetDefaultColumn(DWORD /*dwRes*/, ULONG* pSort, ULONG* pDisplay)
{
   ATLTRACE(L"CShellFolder::GetDefaultColumn\n");
   const VFS_FOLDERSETTINGS Settings = m_spFolderItem->GetFolderSettings();
   if( pSort != NULL ) *pSort = Settings.uDefaultSortColumn;
   if( pDisplay != NULL ) *pDisplay = Settings.uDefaultDisplayColumn;
   return S_OK;
}

STDMETHODIMP CShellFolder::GetDefaultColumnState(UINT iColumn, SHCOLSTATEF* pcsFlags)
{
   ATLTRACE(L"CShellFolder::GetDefaultColumnState  column=%u\n", iColumn);
   VFS_COLUMNINFO Column = { 0 };
   HR( m_spFolderItem->GetColumnInfo(iColumn, Column) );
   *pcsFlags = Column.dwFlags;
   return S_OK;
}

STDMETHODIMP CShellFolder::GetDetailsEx(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
   ATLTRACE(L"CShellFolder::GetDetailsEx  scid=%s\n", DbgGetPKEY(*pscid));
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   CComPropVariant var;
   if( SUCCEEDED( spItem->GetProperty(*pscid, var) ) ) return var.CopyTo(pv);
   ATLTRACE(L"CShellFolder::GetDetailsEx - failed\n");
   return E_FAIL;
}

STDMETHODIMP CShellFolder::GetDetailsOf(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS* psd)
{
   ATLTRACE(L"CShellFolder::GetDetailsOf  column=%u\n", iColumn);
   ATLASSERT(psd);
   SHCOLUMNID scid = { 0 };
   HRESULT Hr = MapColumnToSCID(iColumn, &scid);
   if( FAILED(Hr) ) return Hr;
   // If it's for the column, return column information
   if( pidl == NULL ) return _GetColumnDetailsOf(&scid, psd);
   // Return member information for this item...
   CComVariant v;
   HR( GetDetailsEx(pidl, &scid, &v) );
   HR( v.ChangeType(VT_BSTR) );
   return StrToSTRRET(v.bstrVal, &psd->str);
}

STDMETHODIMP CShellFolder::MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid)
{
   ATLTRACE(L"CShellFolder::MapColumnToSCID  column=%u\n", iColumn);
   VFS_COLUMNINFO Info = { 0 };
   HRESULT Hr = m_spFolderItem->GetColumnInfo(iColumn, Info);
   if( FAILED(Hr) ) return Hr;
   if( IsBitSet(Info.dwAttributes, VFS_COLF_NOTCOLUMN) ) return E_FAIL;
   *pscid = Info.pkey;
   return S_OK;
}       

// IShellIcon

STDMETHODIMP CShellFolder::GetIconOf(PCUITEMID_CHILD pidl, UINT uIconFlags, int* pIconIndex)
{
   ATLTRACE(L"CShellFolder::GetIconOf\n");
   if( !_ShellModule.GetConfigBool(VFS_HAVE_SYSICONS) ) return S_FALSE;
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return S_FALSE;
   return spItem->GetSysIcon(uIconFlags, pIconIndex);
}

// IShellIconOverlay

STDMETHODIMP CShellFolder::GetOverlayIndex(PCUITEMID_CHILD pidl, int* pIndex)
{
   ATLTRACE(L"CShellFolder::GetOverlayIndex\n");
   if( !_ShellModule.GetConfigBool(VFS_HAVE_ICONOVERLAYS) ) return S_FALSE;
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return S_FALSE;
   return spItem->GetIconOverlay(pIndex);
}

STDMETHODIMP CShellFolder::GetOverlayIconIndex(PCUITEMID_CHILD pidl, int* pIconIndex)
{
   ATLTRACE(L"CShellFolder::GetOverlayIconIndex\n");
   if( !_ShellModule.GetConfigBool(VFS_HAVE_ICONOVERLAYS) ) return S_FALSE;
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return S_FALSE;
   int iIndex = 0;
   HRESULT Hr = spItem->GetIconOverlay(&iIndex);
   if( Hr == S_OK ) *pIconIndex = INDEXTOOVERLAYMASK(iIndex);
   return Hr;
}

// IFolderViewSettings

STDMETHODIMP CShellFolder::GetColumnPropertyList(REFIID riid, LPVOID* ppv)
{
   ATLTRACE(L"CShellFolder::GetColumnPropertyList\n");
   return E_NOTIMPL;
}

STDMETHODIMP CShellFolder::GetGroupByProperty(PROPERTYKEY* pkey, BOOL* pfGroupAscending)
{
   ATLTRACE(L"CShellFolder::GetGroupByProperty\n");
   const VFS_FOLDERSETTINGS Settings = m_spFolderItem->GetFolderSettings();
   if( Settings.GroupByKey == PKEY_Null ) return E_NOTIMPL;
   *pkey = Settings.GroupByKey;
   *pfGroupAscending = Settings.bGroupByAsc;
   return S_OK;
}

STDMETHODIMP CShellFolder::GetViewMode(FOLDERLOGICALVIEWMODE* plvm)
{
   ATLTRACE(L"CShellFolder::GetViewMode\n");
   const VFS_FOLDERSETTINGS Settings = m_spFolderItem->GetFolderSettings();
   if( Settings.ViewMode == 0 ) return E_NOTIMPL;
   *plvm = Settings.ViewMode;
   return S_OK;
}

STDMETHODIMP CShellFolder::GetIconSize(UINT* puIconSize)
{
   ATLTRACE(L"CShellFolder::GetIconSize\n");
   const VFS_FOLDERSETTINGS Settings = m_spFolderItem->GetFolderSettings();
   if( Settings.cxyIcon == 0 ) return E_NOTIMPL;
   *puIconSize = Settings.cxyIcon;
   return S_OK;
}

STDMETHODIMP CShellFolder::GetFolderFlags(FOLDERFLAGS* pfolderMask, FOLDERFLAGS* pfolderFlags)
{
   ATLTRACE(L"CShellFolder::GetFolderFlags\n");
   const VFS_FOLDERSETTINGS Settings = m_spFolderItem->GetFolderSettings();
   if( Settings.FlagsMask == 0 ) return E_NOTIMPL;
   *pfolderMask = Settings.FlagsMask;
   *pfolderFlags = Settings.FlagsValue;
   return S_OK;
}

STDMETHODIMP CShellFolder::GetSortColumns(SORTCOLUMN* rgSortColumns, UINT cColumnsIn, UINT* pcColumnsOut)
{
   ATLTRACE(L"CShellFolder::GetSortColumns\n");
   return E_NOTIMPL;
}

STDMETHODIMP CShellFolder::GetGroupSubsetCount(UINT* pcVisibleRows)
{
   ATLTRACE(L"CShellFolder::GetGroupSubsetCount\n");
   const VFS_FOLDERSETTINGS Settings = m_spFolderItem->GetFolderSettings();
   if( Settings.nGroupVisible == 0 ) return E_NOTIMPL;
   *pcVisibleRows = Settings.nGroupVisible;
   return S_OK;
}

// IExplorerPaneVisibility

STDMETHODIMP CShellFolder::GetPaneState(REFEXPLORERPANE ep, EXPLORERPANESTATE* peps)
{
   ATLTRACE(L"CShellFolder::GetPaneState  ep=%s\n", DbgGetIID(ep));
   static struct { 
      REFEXPLORERPANE ep; 
      VFS_CONFIG config; 
      EXPLORERPANESTATE peps;
   } aList[] = {
      { EP_DetailsPane, VFS_SHOW_DETAILS_PANEL, EPS_DEFAULT_ON | EPS_INITIALSTATE },
      { EP_PreviewPane, VFS_SHOW_PREVIEW_PANEL, EPS_DEFAULT_ON | EPS_INITIALSTATE },
      { EP_NavPane,     VFS_HIDE_NAVTREE_PANEL, EPS_DEFAULT_OFF | EPS_INITIALSTATE | EPS_FORCE },
      { EP_DetailsPane, VFS_HIDE_DETAILS_PANEL, EPS_DEFAULT_OFF | EPS_INITIALSTATE | EPS_FORCE },
      { EP_PreviewPane, VFS_HIDE_PREVIEW_PANEL, EPS_DEFAULT_OFF | EPS_FORCE },
   };
   *peps = EPS_DONTCARE;
   for( int i = 0; i < lengthof(aList); i++ ) {
      if( ep == aList[i].ep && _ShellModule.GetConfigBool(aList[i].config) ) *peps = aList[i].peps;
   }
   return m_spFolderItem->GetPaneState(ep, peps);
}

// IThumbnailHandlerFactory

STDMETHODIMP CShellFolder::GetThumbnailHandler(PCUITEMID_CHILD pidlItem, IBindCtx *pbc, REFIID riid, LPVOID* ppv)
{
   ATLTRACE(L"CShellFolder::GetThumbnailHandler  riid=%s\n", DbgGetIID(riid));
   CNseItemPtr spItem = GenerateChildItem(pidlItem);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   return spItem->GetThumbnail(riid, ppv);
}

// IStorage

STDMETHODIMP CShellFolder::CreateStream(LPCWSTR pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream** ppstm)
{
   ATLTRACENOTIMPL(L"CShellFolder::CreateStream");
}

STDMETHODIMP CShellFolder::OpenStream(LPCWSTR pwcsName, LPVOID reserved1, DWORD grfMode, DWORD reserved2, IStream** ppstm)
{
   ATLTRACE(L"CShellFolder::OpenStream  name=%ls\n", pwcsName);
   CNseItemPtr spItem;
   HR( m_spFolderItem->GetChild(pwcsName, SHGDN_FORPARSING, &spItem) );
   CComObject<CFileStream>* pStream = NULL;
   HR( CComObject<CFileStream>::CreateInstance(&pStream) );
   CComPtr<IUnknown> spKeepAlive = pStream->GetUnknown();
   HR( pStream->Init(this, spItem->GetITEMID()) );
   return pStream->QueryInterface(IID_PPV_ARGS(ppstm));
}

STDMETHODIMP CShellFolder::CreateStorage(LPCWSTR pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage** ppstg)
{
   ATLTRACENOTIMPL(L"CShellFolder::CreateStorage");
}

STDMETHODIMP CShellFolder::OpenStorage(LPCWSTR pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstg)
{
   ATLTRACENOTIMPL(L"CShellFolder::OpenStorage");
}

STDMETHODIMP CShellFolder::CopyTo(DWORD ciidExclude, const IID* rgiidExclude, SNB snbExclude, IStorage* pstgDest)
{
   ATLTRACENOTIMPL(L"CShellFolder::CopyTo");
}

STDMETHODIMP CShellFolder::MoveElementTo(LPCWSTR pwcsName, IStorage* pstgDest, LPCWSTR pwcsNewName, DWORD grfFlags)
{
   ATLTRACENOTIMPL(L"CShellFolder::MoveElementTo");
}

STDMETHODIMP CShellFolder::Commit(DWORD grfCommitFlags)
{
   ATLTRACE(L"CShellFolder::Commit\n");
   return S_OK;
}

STDMETHODIMP CShellFolder::Revert()
{
   ATLTRACENOTIMPL(L"CShellFolder::Revert");
}

STDMETHODIMP CShellFolder::EnumElements(DWORD reserved1, LPVOID reserved2, DWORD reserved3, IEnumSTATSTG** ppenum)
{
   ATLTRACE(L"CShellFolder::EnumElements\n");
   *ppenum = NULL;
   if( !_ShellModule.GetConfigBool(VFS_HAVE_VIRTUAL_FILES) ) return E_NOTIMPL;
   CNseItemArray aChildren;
   HR( m_spFolderItem->EnumChildren(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_STORAGE, aChildren) );
   CSimpleArray<STATSTG> aList;
   for( int i = 0; i < aChildren.GetSize(); i++ ) {
      const VFS_FIND_DATA wfd = aChildren[i]->GetFindData();
      STATSTG statstg = { 0 };
      statstg.pwcsName = ::StrDup(wfd.cFileName);
      statstg.cbSize.LowPart = wfd.nFileSizeLow;
      statstg.cbSize.HighPart = wfd.nFileSizeHigh;
      statstg.ctime = wfd.ftCreationTime;
      statstg.mtime = wfd.ftLastWriteTime;
      statstg.atime = wfd.ftLastAccessTime;
      statstg.type = STGTY_STREAM;
      aList.Add(statstg);
   }
   typedef CComEnum< IEnumSTATSTG, &IID_IEnumSTATSTG, STATSTG, _Copy<STATSTG> > CEnumSTATSTG;
   CComObject<CEnumSTATSTG>* pEnumSTATSTG = NULL;
   HR( CComObject<CEnumSTATSTG>::CreateInstance(&pEnumSTATSTG) );
   HR( pEnumSTATSTG->Init(aList.GetData(), aList.GetData() + aList.GetSize(), NULL, AtlFlagCopy) );
   for( int i = 0; i < aList.GetSize(); i++ ) ::CoTaskMemFree(aList[i].pwcsName);
   return pEnumSTATSTG->QueryInterface(IID_PPV_ARGS(ppenum));
}

STDMETHODIMP CShellFolder::DestroyElement(LPCWSTR pwcsName)
{
   ATLTRACE(L"CShellFolder::DestroyElement  name=%ls\n", pwcsName);
   CNseItemPtr spItem;
   HR( m_spFolderItem->GetChild(pwcsName, SHGDN_FORPARSING, &spItem) );
   HR( spItem->Delete() );
   return S_OK;
}

STDMETHODIMP CShellFolder::RenameElement(LPCWSTR pwcsOldName, LPCWSTR pwcsNewName)
{
   ATLTRACE(L"CShellFolder::RenameElement  name=%ls\n", pwcsOldName);
   CNseItemPtr spItem;
   HR( m_spFolderItem->GetChild(pwcsOldName, SHGDN_FORPARSING, &spItem) );
   WCHAR wszOutputName[MAX_PATH] = { 0 };
   wcscpy_s(wszOutputName, lengthof(wszOutputName), pwcsNewName);
   HR( spItem->Rename(pwcsNewName, wszOutputName) );
   return S_OK;
}

STDMETHODIMP CShellFolder::SetElementTimes(LPCWSTR pwcsName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime)
{
   ATLTRACENOTIMPL(L"CShellFolder::SetElementTimes");
}

STDMETHODIMP CShellFolder::SetClass(REFCLSID clsid)
{
   ATLTRACENOTIMPL(L"CShellFolder::SetClass");
}

STDMETHODIMP CShellFolder::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
   ATLTRACENOTIMPL(L"CShellFolder::SetStateBits");
}

STDMETHODIMP CShellFolder::Stat(STATSTG* pStatstg, DWORD grfStatFlag)
{
   ATLTRACE(L"CShellFolder::Stat\n");
   const WIN32_FIND_DATA wfd = m_spFolderItem->GetFindData();
   ::ZeroMemory(pStatstg, sizeof(STATSTG));
   if( !IsBitSet(grfStatFlag, STATFLAG_NONAME) ) pStatstg->pwcsName = ::StrDup(wfd.cFileName);
   pStatstg->grfMode = STGM_READWRITE;
   pStatstg->type = STGTY_STORAGE;
   return S_OK;
}

// IItemNameLimits

STDMETHODIMP CShellFolder::GetValidCharacters(LPWSTR* ppwszValidChars, LPWSTR* ppwszInvalidChars)
{
   ATLTRACE(L"CShellFolder::GetValidCharacters\n");
   CCoTaskString strAllowed = _ShellModule.GetConfigStr(VFS_STR_FILENAME_CHARS_ALLOWED);
   CCoTaskString strNotAllowed = _ShellModule.GetConfigStr(VFS_STR_FILENAME_CHARS_NOTALLOWED);
   if( ppwszValidChars != NULL ) *ppwszValidChars = strAllowed.Detach();
   if( ppwszInvalidChars != NULL ) *ppwszInvalidChars = strNotAllowed.Detach();
   return S_OK;
}

STDMETHODIMP CShellFolder::GetMaxLength(LPCWSTR pszName, int* piMaxNameLen)
{
   ATLTRACE(L"CShellFolder::GetMaxLength  name='%ls'\n", pszName);
   bool bHasPath = (wcschr(pszName, '\\') != NULL);
   int cchMax = _ShellModule.GetConfigInt(bHasPath ? VFS_INT_MAX_PATHNAME_LENGTH : VFS_INT_MAX_FILENAME_LENGTH);
   if( cchMax <= 0 ) cchMax = MAX_PATH;
   ATLASSERT(cchMax<=MAX_PATH);  // Our framework sets this limit!
   int cchName = pszName != NULL ? (int) ::lstrlenW(pszName) : 0;
   *piMaxNameLen = cchMax > cchName ? cchMax - cchName : 0;
   return S_OK;
}

// IObjectProvider

STDMETHODIMP CShellFolder::QueryObject(REFGUID guidObject, REFIID riid, LPVOID* ppv)
{
   ATLTRACE(L"CShellFolder::QueryObject  obj=%s riid=%s\n", DbgGetIID(guidObject), DbgGetIID(riid));
   ATLTRACE(L"CShellFolder::QueryObject - failed\n");
   *ppv = NULL;
   return E_NOTIMPL;
}

// IContextMenuCB

STDMETHODIMP CShellFolder::CallBack(IShellFolder* psf, HWND hwndOwner, IDataObject* pDataObject, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ATLTRACE(L"CShellFolder::MenuCallback  msg=%ld wp=0x%X lp=0x%X\n", uMsg, wParam, lParam);
   HRESULT Hr = E_NOTIMPL;
   switch( uMsg ) {
   case DFM_INVOKECOMMANDEX:
      {
         const DFMICS* pDFMICS = reinterpret_cast<DFMICS*>(lParam);
         ATLASSERT(pDFMICS->cbSize==sizeof(DFMICS));
         CComPtr<IShellItemArray> spItems;
         ::SHCreateShellItemArrayFromDataObject(pDataObject, IID_PPV_ARGS(&spItems));
         VFS_MENUCOMMAND Cmd = { hwndOwner, (UINT) wParam, HIWORD(pDFMICS->pici->lpVerb) != 0 ? pDFMICS->pici->lpVerb : VFS_MNUCMD_NOVERB, DROPEFFECT_COPY, NULL, spItems, NULL, pDFMICS->punkSite, NULL };
         return ExecuteMenuCommand(Cmd);
      }
   case DFM_MERGECONTEXTMENU:
      {
         QCMINFO* pqcmi = reinterpret_cast<QCMINFO*>(lParam);
         if( !::IsMenu(m_hContextMenu) ) return S_OK;
         _SetMenuState(m_hContextMenu, pDataObject);
         UINT uCmdFirst = pqcmi->idCmdFirst;
         pqcmi->idCmdFirst = ::Shell_MergeMenus(pqcmi->hmenu, m_hContextMenu, pqcmi->indexMenu, pqcmi->idCmdFirst, pqcmi->idCmdLast, 0);
         if( !IsBitSet(wParam, CMF_NODEFAULT) ) ::SetMenuDefaultItem(pqcmi->hmenu, uCmdFirst + ::GetMenuItemID(m_hContextMenu, 0), MF_BYCOMMAND);
         return S_OK;
      }
   case DFM_GETVERBA:
   case DFM_GETVERBW:
      {
         CResString<300> sCmd(LOWORD(wParam));
         if( sCmd.IsEmpty() ) return S_FALSE;
         LPCTSTR pstrSep = _tcschr(sCmd, '\n');
         if( pstrSep == NULL ) return S_FALSE;
         if( uMsg == DFM_GETVERBA ) return strncpy(reinterpret_cast<LPSTR>(lParam), CT2CA(pstrSep + 1), HIWORD(wParam)) > 0 ? S_OK : E_FAIL;
         else return wcsncpy(reinterpret_cast<LPWSTR>(lParam), CT2CW(pstrSep + 1), HIWORD(wParam)) > 0 ? S_OK : E_FAIL;
      }
   case DFM_GETHELPTEXT:
   case DFM_GETHELPTEXTW:
      {
         CResString<300> sCmd((UINT) wParam);
         if( sCmd.IsEmpty() ) return S_FALSE;
         LPTSTR pstrSep = const_cast<LPTSTR>(_tcschr(sCmd, '\n'));
         if( pstrSep != NULL ) *pstrSep = '\0';
         if( uMsg == DFM_GETHELPTEXT ) return strncpy(reinterpret_cast<LPSTR>(lParam), CT2CA(sCmd), HIWORD(wParam)) > 0 ? S_OK : E_FAIL;
         else return wcsncpy(reinterpret_cast<LPWSTR>(lParam), CT2CW(sCmd), HIWORD(wParam)) > 0 ? S_OK : E_FAIL;
      }
   case DFM_MERGECONTEXTMENU_TOP:
   case DFM_MERGECONTEXTMENU_BOTTOM:
      return S_OK;
   case DFM_GETDEFSTATICID:
   case DFM_MAPCOMMANDNAME:
   case DFM_VALIDATECMD:
      Hr = S_FALSE;
      break;
   }
   return Hr;
}

// IShellFolderViewCB messages

LRESULT CShellFolder::OnWindowCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   return 0;
}

LRESULT CShellFolder::OnGetNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // Tell the Shell we want to receive Change Notifications for our folder.
   // NOTE: A nasty non-owned referenced IDLIST is returned to the Shell here!
   * reinterpret_cast<PIDLIST_ABSOLUTE*>(wParam) = m_pidlMonitor;
   * reinterpret_cast<LONG*>(lParam) = SHCNE_DISKEVENTS;
   return S_OK;
}

LRESULT CShellFolder::OnListRefreshed(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // This is an undocumented feature, but it appears that when the user
   // forces a refresh (ie. through a F5 keypress) then the wParam is non-zero.
   if( wParam != 0 ) m_spFolderItem->Refresh(VFS_REFRESH_USERFORCED);
   return 0;
}

// Operations

/**
 * Bind to a child folder.
 * Helper function that binds to a folder item in the children list of the current folder.
 */
HRESULT CShellFolder::BindToFolder(PCUITEMID_CHILD pidl, CRefPtr<CShellFolder>& spFolder)
{
   ATLASSERT(::ILIsChild(pidl));
   // Ensure this child is browsable...
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) {
      ATLTRACE(L"CShellFolder::BindToFolder - failed (not found)\n");
      return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   }
   if( !spItem->IsFolder() ) {
      ATLTRACE(L"CShellFolder::BindToFolder - failed (SFGAO_FOLDER)\n");
      return E_INVALIDARG;
   }
   // Initialize new folder object
   CComObject<CShellFolder>* pFolder = NULL;
   HR( CComObject<CShellFolder>::CreateInstance(&pFolder) );
   CComPtr<IUnknown> spKeepAlive = pFolder->GetUnknown();
   pFolder->m_spFS = m_spFS;
   pFolder->m_pidlRoot = m_pidlRoot;
   pFolder->m_pidlParent = m_pidlFolder;
   pFolder->m_pidlFolder = m_pidlFolder + pidl;
   pFolder->m_pidlMonitor = CPidl(m_pidlRoot, m_pidlFolder, pidl);
   // Finally create a NSE Item for the folder.
   // NOTE: The delicate choice of arguments here is because we depend on the memory
   //       to be scoped by the new instance.
   pFolder->m_spFolderItem = m_spFolderItem->GenerateChild(pFolder, pFolder->m_pidlParent, ILCloneChild(pidl), TRUE);
   ATLASSERT(pFolder->m_spFolderItem);
   if( pFolder->m_spFolderItem == NULL ) return E_UNEXPECTED;
   spFolder = pFolder;
   return S_OK;
}

/**
 * Set a property on a child item.
 * Helper function to set a property on a child item.
 */
HRESULT CShellFolder::SetItemProperty(PCUITEMID_CHILD pidl, REFPROPERTYKEY pkey, PROPVARIANT* pv)
{
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   return spItem->SetProperty(pkey, *pv);
}

/**
 * Get a property from a child item.
 * Helper function to read a property from a child item.
 * When asking an item for multiple properties, use the GetItemProperties() method instead.
 */
HRESULT CShellFolder::GetItemProperty(PCUITEMID_CHILD pidl, REFPROPERTYKEY pkey, PROPVARIANT* pv)
{
   ATLASSERT(pv->vt==VT_EMPTY);
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   CComPropVariant var;
   HRESULT Hr = spItem->GetProperty(pkey, var);
   if( FAILED(Hr) ) return Hr;
   return var.Detach(pv);
}

/**
 * Get the property state for a property of a child item.
 */
HRESULT CShellFolder::GetItemPropertyState(PCUITEMID_CHILD pidl, REFPROPERTYKEY pkey, VFS_PROPSTATE& State)
{
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   State = spItem->GetPropertyState(pkey);
   return S_OK;
}

/**
 * Get one or more properties from a child item.
 * Helper function to read a bundle of properties quickly.
 * If a property does not exist, it is returned as VT_EMPTY.
 */
HRESULT CShellFolder::GetItemProperties(PCUITEMID_CHILD pidl, UINT cKeys, const PROPERTYKEY* pkey, PROPVARIANT* pv)
{
   CNseItemPtr spItem = GenerateChildItem(pidl);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   for( UINT i = 0; i < cKeys; i++ ) {
      CComPropVariant var;
      spItem->GetProperty(pkey[i], var);
      var.Detach(pv + i);
   }
   return S_OK;
}

/**
 * Create a NSE Item.
 * Helper function which returns a child item of this folder. This method blindly
 * creates the child PIDL as a member of this folder, it does not validate if it
 * is actually an existing item under it.
 */
CNseItem* CShellFolder::GenerateChildItem(PCUITEMID_CHILD pidlItem, BOOL bReleaseItem /*= FALSE*/)
{
   ATLASSERT(pidlItem);
   ATLASSERT(m_spFolderItem);
   //ATLASSERT(::ILIsChild(pidlItem));   // Optimized away because of CShellFolder::CompareIDs
   if( pidlItem == NULL ) return NULL;
   if( m_spFolderItem == NULL ) return NULL;
#if defined(_M_X64) || defined(_M_IA64)
   if( !bReleaseItem && !::ILIsAligned(pidlItem) ) {
      pidlItem = ILCloneChild(pidlItem);
      bReleaseItem = TRUE;
   }
   ATLASSERT(ILIsAligned(pidlItem));
   return m_spFolderItem->GenerateChild(this, m_pidlFolder, static_cast<PCITEMID_CHILD>(pidlItem), bReleaseItem);
#else
   return m_spFolderItem->GenerateChild(this, m_pidlFolder, pidlItem, bReleaseItem);
#endif  // _M_IX86
}

/**
 * Create a NSE Item from a IShellItem.
 * Even if the argument is a IShellItem, this method still assumes that the
 * item is actually a child of this folder.
 */
CNseItem* CShellFolder::GenerateChildItemFromShellItem(IShellItem* pShellItem)
{
   ATLASSERT(pShellItem);
   ATLASSERT(m_spFolderItem);
   CPidl pidlFull;
   if( FAILED( pidlFull.CreateFromObject(pShellItem) ) ) return NULL;
   ATLASSERT(m_pidlMonitor.IsParent(pidlFull, TRUE));
   ATLASSERT(m_pidlMonitor.GetItemCount()==pidlFull.GetItemCount()-1);
   if( !m_pidlMonitor.IsParent(pidlFull, TRUE) ) return NULL;
   return m_spFolderItem->GenerateChild(this, m_pidlFolder, ::ILCloneChild( pidlFull.GetLastItem() ), TRUE);
}

/**
 * Execute File-Operation commands on separate thread.
 * This solves a problem with any slow file-copy that would block since the Shell often uses the
 * UI thread to perform any menu-command execution.
 */
static DWORD WINAPI FileOperationThread(LPVOID pData)
{
   // BUG: This violates fundamental COM rules by moving an interface to a different apartment. We detached
   //      ownership to the FileOperation component, so we cross our fingers. Let's hope the FO component
   //      doesn't have any real thread-affinity (ie. uses TLS).
   IFileOperation* pFO = reinterpret_cast<IFileOperation*>(pData);
   pFO->PerformOperations();
   pFO->Release();
   return 0;
}

/**
 * Execute a menuitem command on one or more shell items.
 * This is a helper function that executes a menu command on the item selection.
 */
HRESULT CShellFolder::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   // Translate some verbs for easy accessibility.
   // Shell sometimes calls us only with the verb and not the ID of the verb.
   static struct {
      LPCSTR pszVerb;
      UINT uVerb;
   } aVerbs[] = {
      { CMDSTR_COPYA,        DFM_CMD_COPY },
      { CMDSTR_PASTEA,       DFM_CMD_PASTE },
      { CMDSTR_DELETEA,      DFM_CMD_DELETE },
      { CMDSTR_RENAMEA,      DFM_CMD_RENAME },
      { CMDSTR_NEWFOLDERA,   DFM_CMD_NEWFOLDER },
      { CMDSTR_PROPERTIESA,  DFM_CMD_PROPERTIES },
   };
   for( int i = 0; Cmd.wMenuID == 0 && Cmd.pstrVerb != NULL && i < lengthof(aVerbs); i++ ) {
      if( strcmp(Cmd.pstrVerb, aVerbs[i].pszVerb) == 0 ) Cmd.wMenuID = aVerbs[i].uVerb;
   }
   // Do we have a DataObject available, or try getting it now...
   CComPtr<IDataObject> spDataObject;
   if( Cmd.pDataObject == NULL ) 
   {
      ::OleGetClipboard(&spDataObject);
      if( spDataObject != NULL ) {
         Cmd.pDataObject = spDataObject;
         DataObj_GetData(Cmd.pDataObject, CFSTR_PREFERREDDROPEFFECT, &Cmd.dwDropEffect, sizeof(Cmd.dwDropEffect));
      }
   }
   // Prepare command execution; it may either be a command on the folder item
   // or on one or more selected items.
   HRESULT HrRes = S_FALSE;
   if( Cmd.pShellItems == NULL ) 
   {
      // No ShellItemArray; execute directly on view
      HRESULT Hr = m_spFolderItem->ExecuteMenuCommand(Cmd);
      if( Hr != E_NOTIMPL ) HrRes = Hr;
   }
   else
   {
      // Execute on every item in selection...
      DWORD dwItems = 0;
      HR( Cmd.pShellItems->GetCount(&dwItems) );
      for( DWORD i = 0; i < dwItems; i++ ) {
         CComPtr<IShellItem> spShellItem;
         HR( Cmd.pShellItems->GetItemAt(i, &spShellItem) );
         CNseItemPtr spItem = GenerateChildItemFromShellItem(spShellItem);
         if( spItem == NULL ) continue;
         HRESULT Hr = spItem->ExecuteMenuCommand(Cmd);
         if( Hr != E_NOTIMPL ) HrRes = Hr;
         if( HrRes != S_OK ) break;
      }
   }
   // Any item can abort if they performed the entire operation alone
   if( HrRes == NSE_S_ALL_DONE ) HrRes = S_OK;
   // Perform any file-operation requested.
   // Execute commands on separate thread if slow copying is expected.
   if( Cmd.pFO != NULL ) 
   {
      if( SUCCEEDED(HrRes) ) {
         // TODO: Make this COM safe! Also right now, we can only do this on a COPY operation, and not a MOVE operation, 
         //       because of the need to notify the data-object in CDropTarget::Drop().
         if( _ShellModule.GetConfigBool(VFS_CAN_SLOW_COPY) && Cmd.dwDropEffect == DROPEFFECT_COPY ) {
            ::SHCreateThread(FileOperationThread, Cmd.pFO, CTF_COINIT_STA | CTF_PROCESS_REF | CTF_THREAD_REF | CTF_INSIST, NULL);
            Cmd.pFO = NULL;
         }
         else {
            HrRes = Cmd.pFO->PerformOperations();
            BOOL bAborted = FALSE;
            if( SUCCEEDED(HrRes) && SUCCEEDED(Cmd.pFO->GetAnyOperationsAborted(&bAborted)) && bAborted ) HrRes = E_ABORT;
         }
      }
      if( Cmd.pFO != NULL ) Cmd.pFO->Release();
      Cmd.pFO = NULL;
   }
   // Free any remaining user-data
   free(Cmd.pUserData);
   Cmd.pUserData = NULL;
   return HrRes;
}

/**
 * Show Context Menu for NSE folder item.
 */
HRESULT CShellFolder::CreateMenu(HWND hwndOwner, LPCTSTR pstrMenuType, IContextMenu3** ppRetVal)
{
   ATLASSERT(pstrMenuType);
   ATLASSERT(ppRetVal);
   if( ::IsMenu(m_hMenu) ) ::DestroyMenu(m_hMenu);
   m_hMenu = m_spFolderItem->GetMenu();
   m_hContextMenu = ::GetSubMenu(m_hMenu, pstrMenuType);
   DEFCONTEXTMENU dcm = { hwndOwner, static_cast<IContextMenuCB*>(this), m_pidlMonitor, static_cast<IShellFolder*>(this), 0, NULL, NULL, 0, NULL };
   return ::SHCreateDefaultContextMenu(&dcm, IID_PPV_ARGS(ppRetVal));
}

// Implementation

/**
 * Get information about a column.
 */
HRESULT CShellFolder::_GetColumnDetailsOf(const SHCOLUMNID* pscid, SHELLDETAILS* psd) const
{
   ATLASSERT(pscid);
   ATLASSERT(psd);
   psd->str.pOleStr = NULL;
   psd->str.uType = STRRET_WSTR;
   CComPtr<IPropertyDescription> spDescription;
   HR( ::PSGetPropertyDescription(*pscid, IID_PPV_ARGS(&spDescription)) );
   PROPDESC_VIEW_FLAGS pvf = PDVF_DEFAULT;
   HR( spDescription->GetViewFlags(&pvf) );
   if( IsBitSet(pvf, PDVF_CANWRAP) ) psd->fmt |= LVCFMT_WRAP;
   if( IsBitSet(pvf, PDVF_FILLAREA) ) psd->fmt |= LVCFMT_FILL;
   if( IsBitSet(pvf, PDVF_RIGHTALIGN) ) psd->fmt |= LVCFMT_RIGHT;
   if( IsBitSet(pvf, PDVF_HIDELABEL) ) psd->fmt |= LVCFMT_NO_TITLE;
   if( IsBitSet(pvf, PDVF_CENTERALIGN) ) psd->fmt |= LVCFMT_CENTER;
   if( IsBitSet(pvf, PDVF_BEGINNEWGROUP) ) psd->fmt |= LVCFMT_LINE_BREAK;  
   HR( spDescription->GetDefaultColumnWidth((UINT*)(&psd->cxChar)) );
   HRESULT Hr = spDescription->GetDisplayName(&psd->str.pOleStr);
   if( FAILED(Hr) ) Hr = StrToSTRRET(L"", &psd->str);
   return Hr;
}

/**
 * Set the menu state from a selection of items.
 */
HRESULT CShellFolder::_SetMenuState(HMENU hMenu, IDataObject* pDataObject)
{
   CComPtr<IShellItemArray> spShellItems;
   ::SHCreateShellItemArrayFromDataObject(pDataObject, IID_PPV_ARGS(&spShellItems));
   VFS_MENUSTATE State = { hMenu, spShellItems };
   if( spShellItems == NULL ) 
   {
      m_spFolderItem->SetMenuState(State);
   }
   else
   {
      DWORD dwItems = 0;
      HR( spShellItems->GetCount(&dwItems) );
      for( DWORD i = 0; i < dwItems; i++ ) {
         CComPtr<IShellItem> spShellItem;
         HR( spShellItems->GetItemAt(i, &spShellItem) );
         CNseItemPtr spItem = GenerateChildItemFromShellItem(spShellItem);
         if( spItem != NULL ) spItem->SetMenuState(State);
      }
   }
   return S_OK;
}

/**
 * Advanced parsing from display-name when bind-options are available.
 */
HRESULT CShellFolder::_ParseDisplayNameWithBind(CNseItemPtr& spItem, PWSTR pszDisplayName, IBindCtx* pbc, const BIND_OPTS& Opts)
{
   ATLASSERT(pbc);
   ATLASSERT(pszDisplayName);
   if( spItem == NULL && Opts.grfMode == STGM_CREATE ) {
      // The Shell wants to create a new file. For convenience it may provide a WIN32_FIND_DATA structure
      // with prefilled entries. We'll mark the item with the VFS_HACK_SAVEAS_JUNCTION flag so the physical object
      // can be requested.
      WIN32_FIND_DATAW wfd = { 0 };
      CComPtr<IFileSystemBindData> spFSBD;
      ::SHGetBindCtxParam(pbc, STR_FILE_SYS_BIND_DATA, IID_PPV_ARGS(&spFSBD));
      if( spFSBD != NULL ) spFSBD->GetFindData(&wfd);
      if( wfd.cFileName[0] == '\0' ) wcscpy_s(wfd.cFileName, lengthof(wfd.cFileName), pszDisplayName);
      if( wfd.dwFileAttributes == 0 ) wfd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
      wfd.cAlternateFileName[0] = '\0';
      wfd.cAlternateFileName[1] = VFS_HACK_SAVEAS_JUNCTION;
      spItem = m_spFolderItem->GenerateChild(this, m_pidlFolder, wfd);
   }         
   if( spItem != NULL && Opts.grfMode == STGM_FAILIFTHERE ) {
      // FIX: During a Vista Search Folder operation the Shell will call us
      //      with STGM_FAILIFTHERE and refuse to show our items. Try to
      //      detect this and fool the Shell. On Windows 7 the Search Folder
      //      is working beautifully.
      CComPtr<IUnknown> spPOI;
      if( SUCCEEDED( ::SHGetBindCtxParam(pbc, L"ParseOriginalItem", IID_PPV_ARGS(&spPOI)) ) ) return S_OK;
      // Normally we obey the STGM_FAILIFTHERE flag
      return AtlHresultFromWin32(ERROR_FILE_EXISTS);            
   }
   if( spItem != NULL && Opts.grfMode == STGM_READWRITE ) {
      // During the FileSave dialog, the Shell may want to overwrite an existing file. Let's
      // mark the file now so the physical object can be requested later.
      WIN32_FIND_DATA wfd = spItem->GetFindData();
      wfd.cAlternateFileName[0] = '\0';
      wfd.cAlternateFileName[1] = VFS_HACK_SAVEAS_JUNCTION;
      spItem = m_spFolderItem->GenerateChild(this, m_pidlFolder, wfd);
   }
   return S_OK;
}


OBJECT_ENTRY_AUTO(CLSID_ShellFolder, CShellFolder)

