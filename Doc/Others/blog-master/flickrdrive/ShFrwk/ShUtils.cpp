
#include "stdafx.h"

#include "ShellFolder.h"

#include <msxml2.h>


///////////////////////////////////////////////////////////////////////////////
// Win32 Helpers

/**
 * Return a GUID from a string.
 */
GUID UuidFromString(LPOLESTR pwstr)
{
   ATLASSERT(pwstr);
   CLSID clsid = { 0 };
   if( pwstr[0] == '{') ::CLSIDFromString(pwstr, &clsid);
   else ::CLSIDFromProgID(pwstr, &clsid);
   return clsid;
}

/**
 * Get a named submenu.
 */
HMENU GetSubMenu(HMENU hMenu, LPCTSTR pstrTitle)
{
   if( !::IsMenu(hMenu) ) return NULL;
   UINT nMenuItems = (UINT) ::GetMenuItemCount(hMenu);
   for( UINT i = 0; i < nMenuItems; i++ ) {
      TCHAR szTitle[100] = { 0 };
      MENUITEMINFO mii = { 0 };
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_STRING | MIIM_SUBMENU;
      mii.dwTypeData = szTitle;
      mii.cch = lengthof(szTitle) - 1;
      if( !::GetMenuItemInfo(hMenu, i, TRUE, &mii) ) return NULL;
      if( _tcscmp(szTitle, pstrTitle) == 0 ) return mii.hSubMenu;
   }
   return NULL;
}

/**
 * Return the file last-modified timestamp of a disk file.
 */
FILETIME GetFileModifiedTime(LPCTSTR pstrFilename)
{
   FILETIME ftWrite = { 0 };
   HANDLE hFile = ::CreateFile(pstrFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if( hFile == INVALID_HANDLE_VALUE ) return ftWrite;
   FILETIME ftCreation = { 0 }, ftAccess = { 0 };
   ::GetFileTime(hFile, &ftCreation, &ftAccess, &ftWrite);
   ::CloseHandle(hFile);
   return ftWrite;
}

/**
 * Test if a FILETIME structure has been filled out.
 */
BOOL IsFileTimeValid(const FILETIME& ft)
{
   return ft.dwHighDateTime != 0 || ft.dwLowDateTime != 0;
}


///////////////////////////////////////////////////////////////////////////////
// Shell helpers

/**
 * Check for memory alignment on 64bit Windows.
 * On 32bit Windows we don't care much about memory-alignment, but 64bit is a different
 * story. Here we require even our child PIDLs to be aligned. Fortunately the Shell goes
 * an extra mile to keep its structures aligned too and this makes the requirement feasible.
 */
BOOL ILIsAligned64(PCUIDLIST_RELATIVE pidl)
{
#if defined(_M_X64) || defined(_M_IA64)
   return ILIsAligned(pidl);
#else
   return TRUE;
#endif // _M_X64
}

/**
 * Return a STRRET from a string.
 */
HRESULT StrToSTRRET(LPCWSTR pwstr, STRRET* pSrRet)
{
   ATLASSERT(pwstr);
   ATLASSERT(pSrRet);
   pSrRet->uType = STRRET_WSTR;
   return ::SHStrDupW(pwstr, &pSrRet->pOleStr);
}

/**
 * Filter item in IShellFolder::EnumObjects on SHCONTF flags.
 * Use this method to determine if an item is discarded from the IDLIST enum based 
 * on its WIN32_FIND_DATA FileAttributes member.
 */
HRESULT SHFilterEnumItem(SHCONTF grfFlags, const WIN32_FIND_DATA& wfd)
{
   ATLASSERT(!IsBitSet(grfFlags, SHCONTF_NETPRINTERSRCH|SHCONTF_SHAREABLE));
   if( (grfFlags == SHCONTF_STORAGE) && IsBitSet(wfd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_VIRTUAL) ) return S_OK;
   if( !IsBitSet(grfFlags, SHCONTF_FOLDERS) && IsBitSet(wfd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ) return S_FALSE;
   if( !IsBitSet(grfFlags, SHCONTF_NONFOLDERS) && !IsBitSet(wfd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ) return S_FALSE;
   if( !IsBitSet(grfFlags, SHCONTF_INCLUDEHIDDEN) && IsBitSet(wfd.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN) ) return S_FALSE;
   return S_OK;
}

/**
 * Returns the first item in a DataObject as a PIDL.
 */
HRESULT SHGetFirstIDListFromDataObject(IDataObject* pDataObj, PIDLIST_ABSOLUTE* ppidl)
{
   CComPtr<IShellItemArray> spItems;
   ::SHCreateShellItemArrayFromDataObject(pDataObj, IID_PPV_ARGS(&spItems));
   if( spItems == NULL ) return E_FAIL;
   CComPtr<IShellItem> spItem;
   spItems->GetItemAt(0, &spItem);
   if( spItem == NULL ) return E_FAIL;
   return ::SHGetIDListFromObject(spItem, ppidl);
}

/**
 * Query and extract an Object Parameter from Bind Context object.
 */
HRESULT SHGetBindCtxParam(IBindCtx* pbc, LPOLESTR pstrParam, REFIID riid, LPVOID* ppRetVal)
{
   ATLASSERT(pstrParam);
   ATLASSERT(ppRetVal!=NULL);
   ATLASSERT(*ppRetVal==NULL);
   if( pbc == NULL ) return E_INVALIDARG;
   CComPtr<IUnknown> spUnk;
   pbc->GetObjectParam(pstrParam, &spUnk);
   if( spUnk == NULL ) return E_FAIL;
   return spUnk->QueryInterface(riid, ppRetVal);
}

/**
 * Create a Shell Item from relative name.
 * Complements the SHCreateItemFromRelativeName() provided by the Shell with an IShellFolder argument.
 */
HRESULT SHCreateItemFromRelativeName(IShellFolder* pFolder, LPCWSTR pszName, IBindCtx* pbc, REFIID riid, LPVOID* ppRetVal)
{
   ATLASSERT(pFolder);
   ATLASSERT(ppRetVal!=NULL);
   ATLASSERT(*ppRetVal==NULL);
   if( pFolder == NULL || pszName == NULL ) return E_INVALIDARG;
   CComQIPtr<IPersistFolder2> spPersist = pFolder;
   if( spPersist == NULL ) return E_FAIL;
   CPidl pidlFolder;
   HR( spPersist->GetCurFolder((PIDLIST_ABSOLUTE*) &pidlFolder.m_p) );
   CPidl pidlName;
   HR( pFolder->ParseDisplayName(NULL, pbc, const_cast<LPWSTR>(pszName), NULL, &pidlName, NULL) );
   return ::SHCreateItemFromIDList(pidlFolder + pidlName, riid, ppRetVal);
}

/**
 * Create the FileOperation component
 */
HRESULT SHCreateFileOperation(HWND hWnd, DWORD dwFlags, IFileOperation** ppRetVal)
{
   ATLASSERT(ppRetVal!=NULL);
   ATLASSERT(*ppRetVal==NULL);
   HR( ::CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(ppRetVal)) );
   if( hWnd != NULL ) (*ppRetVal)->SetOwnerWindow(hWnd);
   if( dwFlags != 0 ) (*ppRetVal)->SetOperationFlags(dwFlags);
   return S_OK;
}

/**
 * Default implementation of SHGetIconOverlayIndex for an icon in the module's resource.
 */
HRESULT SHGetModuleIconOverlayIndex(int iIconIndex, int* pOverlayIndex)
{
   ATLASSERT(iIconIndex>=0);
   ATLASSERT(pOverlayIndex);
   TCHAR szDllFilename[MAX_PATH] = { 0 };
   ::GetModuleFileName(_pModule->GetModuleInstance(), szDllFilename, MAX_PATH);
   *pOverlayIndex = ::SHGetIconOverlayIndex(szDllFilename, iIconIndex);
   return *pOverlayIndex > 0 ? S_OK : S_FALSE;
}

/**
 * Create a default IExtractIcon component for an icon in the module's resource.
 */
HRESULT SHCreateModuleExtractIcon(int iIconIndex, REFIID riid, LPVOID* ppRetVal)
{
   ATLASSERT(iIconIndex>=0);
   ATLASSERT(ppRetVal!=NULL);
   ATLASSERT(*ppRetVal==NULL);
   WCHAR wszDllFilename[MAX_PATH] = { 0 };
   ::GetModuleFileNameW(_pModule->GetModuleInstance(), wszDllFilename, MAX_PATH);
   CComPtr<IDefaultExtractIconInit> spExIcon;
   HR( ::SHCreateDefaultExtractIcon(IID_PPV_ARGS(&spExIcon)) );
   HR( spExIcon->SetFlags(GIL_PERINSTANCE) );
   HR( spExIcon->SetNormalIcon(wszDllFilename, -iIconIndex) );
   return spExIcon->QueryInterface(riid, ppRetVal);
}

/**
 * Default implementation of IShellIcon::GetSysIcon for a file.
 */
HRESULT SHGetFileSysIcon(LPCTSTR pstrFileName, DWORD dwFileAttributes, UINT uIconFlags, int* pIconIndex)
{
   ATLASSERT(pstrFileName);
   ATLASSERT(pIconIndex);
   SHFILEINFO sfi = { 0 };
   UINT uFlags = SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES;
   if( IsBitSet(uIconFlags, GIL_OPENICON) ) uFlags |= SHGFI_OPENICON;
   if( IsBitSet(uIconFlags, GIL_FORSHELL) ) uFlags |= SHGFI_SHELLICONSIZE;
   if( ::SHGetFileInfo(pstrFileName, dwFileAttributes, &sfi, sizeof(sfi), uFlags) == 0 ) return S_FALSE;
   *pIconIndex = sfi.iIcon;
   return S_OK;
}

/**
 * Create the default Folder InfoTip component.
 */
HRESULT SHCreateFolderInfoTip(IShellFolder* pShellFolder, PCUITEMID_CHILD pidlItem, IQueryInfo* pInnerInfo, REFIID riid, LPVOID* ppRetVal)
{
   ATLASSERT(pShellFolder);
   ATLASSERT(ppRetVal!=NULL);
   ATLASSERT(*ppRetVal==NULL);
   const CLSID CLSID_FolderInfoTip = { 0x22c6c651, 0xf6ea, 0x46be, { 0xbc, 0x83, 0x54, 0xe8, 0x33, 0x14, 0xc6, 0x7f } };
   CComPtr<IInitializeFolderInfoTip> spIFI;
   HR( spIFI.CoCreateInstance(CLSID_FolderInfoTip) );
   CComPtr<IShellItem> spShellItem;
   HR( ::SHCreateShellItem(0, pShellFolder, pidlItem, &spShellItem) );
   HR( spIFI->Initialize(pInnerInfo, spShellItem) );
   return spIFI->QueryInterface(riid, ppRetVal);
}


///////////////////////////////////////////////////////////////////////////////
// COM Registration helpers

static CComVariant LoadResourceToVariant(LPCTSTR pstrName, LPCTSTR pstrType)
{
   CComVariant vRes;
   HRSRC hrsrc = ::FindResource(_pModule->GetResourceInstance(), pstrName, pstrType);
   if( hrsrc == NULL ) return vRes;
   DWORD dwLen = ::SizeofResource(_pModule->GetResourceInstance(), hrsrc);
   HGLOBAL hglb = ::LoadResource(_pModule->GetResourceInstance(), hrsrc);
   if( hglb != NULL ) ::InitVariantFromBuffer(static_cast<LPCSTR>(::LockResource(hglb)), dwLen, &vRes);
   ::FreeResource(hglb);
   return vRes;
}

/**
 * Generate a XML .propdesc document using XSLT transformation.
 * The XML document is loaded from the component's resource. Use XSLT to
 * replace the {$MODULE} variable.
 */
HRESULT UpdatePropertySchemaFromResource(UINT uRes, BOOL bRegister)
{
   COleInitialize OleInit;
   TCHAR szPropDescFile[MAX_PATH] = { 0 };
   ::GetModuleFileName(_pModule->GetModuleInstance(), szPropDescFile, MAX_PATH);
   ::PathRenameExtension(szPropDescFile, L".propdesc");
   if( bRegister ) {
      CComPtr<IXMLDOMDocument2> spXmlDoc;
      HR( spXmlDoc.CoCreateInstance(L"Msxml2.FreeThreadedDOMDocument") );
      HR( spXmlDoc->put_async(VARIANT_FALSE) );
      HR( spXmlDoc->put_validateOnParse(VARIANT_FALSE) );
      CComVariant vXML = LoadResourceToVariant(MAKEINTRESOURCE(uRes), _T("XML"));
      VARIANT_BOOL bStatus = VARIANT_FALSE;
      HR( spXmlDoc->load(vXML, &bStatus) );
      if( bStatus == VARIANT_FALSE ) return E_FAIL;
      CComPtr<IXSLTemplate> spXslTemplate;
      HR( spXslTemplate.CoCreateInstance(L"Msxml2.XSLTemplate") );
      HR( spXslTemplate->putref_stylesheet(spXmlDoc) );
      CComPtr<IXSLProcessor> spXslDoc;
      HR( spXslTemplate->createProcessor(&spXslDoc) );
      TCHAR szModule[MAX_PATH] = { 0 };
      ::GetModuleFileName(_pModule->GetModuleInstance(), szModule, MAX_PATH);
      HR( spXslDoc->addParameter(CComBSTR(L"MODULE"), CComVariant(szModule), NULL) );
      CComPtr<IXMLDOMDocument> spXmlOut;
      HR( spXmlOut.CoCreateInstance(L"Msxml2.FreeThreadedDOMDocument") );
      HR( spXslDoc->put_input(CComVariant(spXmlDoc)) );
      HR( spXslDoc->put_output(CComVariant(spXmlOut)) );
      VARIANT_BOOL bDone = VARIANT_FALSE;
      HR( spXslDoc->transform(&bDone) );
      if( bDone == VARIANT_FALSE ) return E_FAIL;
      HR( spXmlOut->save(CComVariant(szPropDescFile)) );
      HR( ::PSRegisterPropertySchema(szPropDescFile) );
   }
   else {
      ::PSUnregisterPropertySchema(szPropDescFile);
      ::DeleteFile(szPropDescFile);
   }
   return S_OK;
}

/**
 * Install the SendTo item.
 */
HRESULT UpdateSendToFromProject(LPCWSTR pstrDisplayName, LPCWSTR pstrProgId, BOOL bRegister)
{
   CCoTaskString strPath;
   HR( ::SHGetKnownFolderPath(FOLDERID_SendTo, 0, NULL, &strPath) );
   WCHAR wszFilename[MAX_PATH] = { 0 };
   ::wnsprintf(wszFilename, lengthof(wszFilename) - 1, L"%s\\%s.%s", static_cast<LPCWSTR>(strPath), pstrDisplayName, pstrProgId);
   if( bRegister ) CHandle( ::CreateFileW(wszFilename, GENERIC_WRITE, NULL, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) );
   else ::DeleteFileW(wszFilename);
   return S_OK;
}

/**
 * Install a shortcut for the virtual folder in the Windows Start Menu.
 * This method knows when we're installing under WOW64 so creating a shortcut will allow
 * the user to access the virtual folder, even when using a 32bit DLL under 64bit Windows.
 */
HRESULT UpdateStartMenuLink(LPCWSTR pstrDisplayName, LPCWSTR pstrDescription, KNOWNFOLDERID FolderId, BOOL bRegister)
{
   HRESULT Hr = S_OK;
   // Get folder for installation...
   CPidl pidlFolderPath;
   CCoTaskString strFolderPath;
   HR( pidlFolderPath.CreateFromKnownFolder(FolderId) );
   HR( pidlFolderPath.GetName(SIGDN_DESKTOPABSOLUTEPARSING, &strFolderPath) );
   // Create shortcut link...
   CComPtr<IShellLink> spLink;
   HR( spLink.CoCreateInstance(CLSID_ShellLink) );
   BOOL bIsWOW64 = FALSE; ::IsWow64Process(::GetCurrentProcess(), &bIsWOW64);
   CComBSTR bstrPath = bIsWOW64 ? L"%windir%\\syswow64\\explorer.exe" : L"%windir%\\explorer.exe";
   CComBSTR bstrIcon = bIsWOW64 ? L"%windir%\\syswow64\\shell32.dll" : L"%SystemRoot%\\system32\\shell32.dll";
   int iIconIndex = _ShellModule.GetConfigBool(VFS_HAVE_VIRTUAL_FILES) ? SIID_DRIVEFIXED : SIID_FOLDER;
   WCHAR wszArgs[MAX_PATH + 80] = { 0 };
   ::wnsprintf(wszArgs, lengthof(wszArgs) - 1, L"/separate,%s%s%s::%s", bIsWOW64 ? L"/select," : L"", static_cast<LPCWSTR>(strFolderPath), strFolderPath.IsEmpty() ? L"" : L"\\", static_cast<LPCWSTR>(CComBSTR(CLSID_ShellFolder)));
   spLink->SetPath(bstrPath);
   spLink->SetArguments(wszArgs);
   spLink->SetIconLocation(bstrIcon, iIconIndex);
   spLink->SetDescription(pstrDescription);
   // Write the link file to the Windows Start Menu...
   CComQIPtr<IPersistFile> spPersist = spLink;
   if( spPersist == NULL ) return E_UNEXPECTED;
   CCoTaskString strLinkPath;
   HR( ::SHGetKnownFolderPath(FOLDERID_StartMenu, 0, NULL, &strLinkPath) );
   WCHAR wszLinkFilename[MAX_PATH] = { 0 };
   ::wnsprintf(wszLinkFilename, lengthof(wszLinkFilename) - 1, L"%s\\%s.lnk", static_cast<LPCWSTR>(strLinkPath), pstrDisplayName);
   if( bRegister ) Hr = spPersist->Save(wszLinkFilename, TRUE);
   else ::DeleteFileW(wszLinkFilename);
   return Hr;
}


///////////////////////////////////////////////////////////////////////////////
// IDataObject helpers

/**
 * Query if the DataObject currently supports the clipformat.
 */
BOOL DataObj_HasClipFormat(IDataObject* pDataObj, _U_STRINGorCF cf, DWORD tymed /*= TYMED_HGLOBAL*/)
{
   ATLASSERT(pDataObj);
   if( pDataObj == NULL ) return FALSE;
   FORMATETC fe = { cf.m_cf, NULL, DVASPECT_CONTENT, -1, tymed };
   return SUCCEEDED( pDataObj->QueryGetData(&fe) );
}

/**
 * Query if the DataObject currently supports default file clipformats.
 * A DataObject containing a file can be associated with several clipboard formats.
 * Some of these would indicate a filename presence.
 */
BOOL DataObj_HasFileClipFormat(IDataObject* pDataObj)
{
   if( DataObj_HasClipFormat(pDataObj, CF_HDROP) ) return TRUE;
   if( DataObj_HasClipFormat(pDataObj, CFSTR_FILENAMEA) ) return TRUE;
   if( DataObj_HasClipFormat(pDataObj, CFSTR_FILENAMEW) ) return TRUE;
   if( DataObj_HasClipFormat(pDataObj, CFSTR_FILENAMEMAPA) ) return TRUE;
   if( DataObj_HasClipFormat(pDataObj, CFSTR_FILENAMEMAPW) ) return TRUE;
   if( DataObj_HasClipFormat(pDataObj, CFSTR_FILEDESCRIPTORA) ) return TRUE;
   if( DataObj_HasClipFormat(pDataObj, CFSTR_FILEDESCRIPTORW) ) return TRUE;
   return FALSE;
}

/**
 * Set a binary struct from a DataObject.
 * Creates a CLIPFORMAT in the IDataObject as a HGLOBAL.
 */
HRESULT DataObj_SetData(IDataObject* pDataObj, _U_STRINGorCF cf, LPCVOID pValue, SIZE_T cbSize)
{
   ATLASSERT(pDataObj);
   ATLASSERT(pValue);
   if( pDataObj == NULL ) return E_POINTER;
   HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_DISCARDABLE, cbSize);
   if( hMem == NULL ) return E_OUTOFMEMORY;
   LPVOID pData = ::GlobalLock(hMem);
   if( pData == NULL ) return E_OUTOFMEMORY;
   ::CopyMemory(pData, pValue, cbSize);
   ::GlobalUnlock(hMem);
   FORMATETC fmte = { cf.m_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
   STGMEDIUM medium = { 0 };
   medium.tymed = TYMED_HGLOBAL;
   medium.hGlobal = hMem;
   medium.pUnkForRelease = NULL;
   HRESULT Hr = pDataObj->SetData(&fmte, &medium, TRUE); 
   if( FAILED(Hr) ) ::GlobalFree(hMem);
   return Hr;
}

/**
 * Retrieve a binary struct from a DataObject.
 * This method assumes that the CLIPFORMAT is a HGLOBAL.
 */
HRESULT DataObj_GetData(IDataObject* pDataObj, _U_STRINGorCF cf, LPVOID pValue, SIZE_T cbSize)
{
   ATLASSERT(pDataObj);
   ATLASSERT(pValue);
   if( pDataObj == NULL ) return E_POINTER;
   FORMATETC fmte = { cf.m_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
   STGMEDIUM medium = { 0 };
   HRESULT Hr = pDataObj->GetData(&fmte, &medium);
   if( FAILED(Hr) ) return Hr;
   LPCVOID pData = (LPCVOID) ::GlobalLock(medium.hGlobal);
   if( pData != NULL ) {
      ::CopyMemory(pValue, pData, cbSize);
      ::GlobalUnlock(medium.hGlobal);
   }   
   else {
      Hr = E_UNEXPECTED;
   }
   ::ReleaseStgMedium(&medium);
   return Hr;
}


///////////////////////////////////////////////////////////////////////////////
// Debug functions

#ifdef _DEBUG

/**
 * Get named GUID for debug log.
 * Attempt to translate it through standard Registry lookup, or our
 * custom Registry repository of interface, GUID, Co-class names.
 */
LPCWSTR DbgGetIID(REFIID riid)
{
   static CComAutoCriticalSection s_lock;
   CComCritSecLock<CComCriticalSection> lock(s_lock);
   static CSimpleMap<GUID, CComBSTR> s_Cache;
   LPCWSTR pwstrResult = s_Cache.Lookup(riid);
   if( pwstrResult != NULL ) return pwstrResult;
   TCHAR szName[80] = { 0 };
   OLECHAR wszGUID[50] = { 0 };
   ::StringFromGUID2(riid, wszGUID, lengthof(wszGUID));
   // Attempt to find it in the interfaces section
   CRegKey key;
   DWORD dwType = 0;
   DWORD dwCount = sizeof(szName) - sizeof(TCHAR);
   key.Open(HKEY_CLASSES_ROOT, _T("Interface"), KEY_READ);
   if( szName[0] == '\0' && key.Open(key, wszGUID, KEY_READ) == NOERROR ) {
      ::RegQueryValueEx(key.m_hKey, NULL, NULL, &dwType, (LPBYTE) szName, &dwCount);
   }
   // Attempt to find it in the CLSID section
   key.Open(HKEY_CLASSES_ROOT, _T("CLSID"), KEY_READ);
   if( szName[0] == '\0' && key.Open(key, wszGUID, KEY_READ) == NOERROR ) {
      ::RegQueryValueEx(key.m_hKey, NULL, NULL, &dwType, (LPBYTE) szName, &dwCount);
   }
   // Attempt to find it in our Named GUIDs section
   key.Open(HKEY_CLASSES_ROOT, _T("Named GUIDs"), KEY_READ);
   if( szName[0] == '\0' && key.Open(key, wszGUID, KEY_READ) == NOERROR ) {
      ::RegQueryValueEx(key.m_hKey, NULL, NULL, &dwType, (LPBYTE) szName, &dwCount);
   }
   if( szName[0] == '\0' ) wcscpy_s(szName, lengthof(szName), CW2T(wszGUID));
   s_Cache.Add(riid, CComBSTR(szName));
   return s_Cache.Lookup(riid);
}

template< class T >
class CSimpleArrayPkeyEqualHelper
{
public:
   static bool IsEqual(const T& t1, const T& t2) { return IsEqualPropertyKey(t1, t2); };
};

template< class TKey, class TVal >
class CSimpleMapPkeyEqualHelper
{
public:
   static bool IsEqualKey(const TKey& k1, const TKey& k2) { return CSimpleArrayPkeyEqualHelper<TKey>::IsEqual(k1, k2); };
   static bool IsEqualValue(const TVal& v1, const TVal& v2) { return CSimpleArrayEqualHelper<TVal>::IsEqual(v1, v2); };
};

/**
 * Get named PROPERTYKEY for debug log.
 * Attempt to translate it through standard property system lookup, or our
 * custom Registry repository of GUIDs.
 */
LPCWSTR DbgGetPKEY(REFPROPERTYKEY pkey)
{
   static CComAutoCriticalSection s_lock;
   CComCritSecLock<CComCriticalSection> lock(s_lock);
   static CSimpleMap< PROPERTYKEY, CComBSTR, CSimpleMapPkeyEqualHelper<PROPERTYKEY, CComBSTR> > s_Cache;
   LPCWSTR pwstrResult = s_Cache.Lookup(pkey);
   if( pwstrResult != NULL ) return pwstrResult;
   WCHAR wszName[80] = { 0 };
   // Let Windows tell us the property name
   CCoTaskString str;
   if( SUCCEEDED( ::PSGetNameFromPropertyKey(pkey, &str) ) ) wcscpy_s(wszName, lengthof(wszName), str);
   // Attempt to find it in our Named GUIDs section
   if( wszName[0] == '\0' ) {
      WCHAR wszGUID[70] = { 0 };
      CRegKey key;
      DWORD dwType = 0, dwCount = sizeof(wszGUID) - sizeof(TCHAR);
      key.Open(HKEY_CLASSES_ROOT, _T("Named GUIDs"), KEY_READ);
      if( key.Open(key, wszGUID, KEY_READ) == NOERROR ) ::RegQueryValueExW(key.m_hKey, NULL, NULL, &dwType, (LPBYTE) wszName, &dwCount);
   }
   // Just format the GUID
   if( wszName[0] == '\0' ) ::PSStringFromPropertyKey(pkey, wszName, lengthof(wszName));
   // Add it to cache and return
   s_Cache.Add(pkey, CComBSTR(wszName));
   return s_Cache.Lookup(pkey);
}

/**
 * Get named CLIPFORMAT for debug log.
 */
LPCTSTR DbgGetCF(CLIPFORMAT cf)
{
   if( cf == CF_TEXT ) return _T("CF_TEXT");
   if( cf == CF_HDROP ) return _T("CF_HDROP");
   if( cf == CF_UNICODETEXT ) return _T("CF_UNICODETEXT");
   static TCHAR s_wszName[128] = { 0 };
   s_wszName[ ::GetClipboardFormatName(cf, s_wszName, lengthof(s_wszName) - 1) ] = '\0';
   return s_wszName;
}

/**
 * Get flags of SHGDNF type for debug log.
 */
LPCTSTR DbgGetSHGDNF(SHGDNF Flags)
{
   TCHAR szText[100] = { 0 };
   _tcscpy(szText, _T(""));
   static struct { DWORD f; LPCTSTR psz; } aList[] = {
      { SHGDN_FORADDRESSBAR, _T(" FORADDRESSBAR") },
      { SHGDN_FOREDITING,    _T(" FOREDITING") },
      { SHGDN_FORPARSING,    _T(" FORPARSING") },
      { SHGDN_INFOLDER,      _T(" INFOLDER") },
   };
   for( int i = 0; i < lengthof(aList); i++ ) {
      if( IsBitSet(Flags, aList[i].f) ) _tcscat_s(szText, lengthof(szText), aList[i].psz);
   }
   if( szText[0] == '\0' ) _tcscpy(szText, _T(" NORMAL"));
   static TCHAR s_szText[100] = { 0 };
   _tcscpy(s_szText, szText);
   return s_szText + 1;
}

/**
 * Get flags of SHCONTF type for debug log.
 */
LPCTSTR DbgGetSHCONTF(SHCONTF Flags)
{
   TCHAR szText[100] = { 0 };
   _tcscpy(szText, _T(""));
   static struct { DWORD f; LPCTSTR psz; } aList[] = {
      { SHCONTF_FOLDERS,        _T(" FOLDERS") },
      { SHCONTF_NONFOLDERS,     _T(" NONFOLDERS") },
      { SHCONTF_INCLUDEHIDDEN,  _T(" INCLUDEHIDDEN") },
      { SHCONTF_STORAGE,        _T(" STORAGE") },
      { SHCONTF_SHAREABLE,      _T(" SHAREABLE") },
      { SHCONTF_FASTITEMS,      _T(" FASTITEMS") },
      { SHCONTF_FLATLIST,       _T(" FLATLIST") },
      { SHCONTF_NETPRINTERSRCH, _T(" NETPRINTERSRCH") },
      { SHCONTF_ENABLE_ASYNC,   _T(" ENABLE_ASYNC") },
   };
   for( int i = 0; i < lengthof(aList); i++ ) {
      if( IsBitSet(Flags, aList[i].f) ) _tcscat_s(szText, lengthof(szText), aList[i].psz);
   }
   if( szText[0] == '\0' ) _tcscpy(szText, _T(" NONE"));
   static TCHAR s_szText[100] = { 0 };
   _tcscpy(s_szText, szText);
   return s_szText + 1;
}

/**
 * Get flags of SFGAOF type for debug log.
 */
LPCTSTR DbgGetSFGAOF(SFGAOF Flags)
{
   TCHAR szText[300] = { 0 };
   _tcscpy(szText, _T(""));
   static struct { DWORD f; LPCTSTR psz; } aList[] = {
      { SFGAO_FOLDER,          _T(" FOLDER") },
      { SFGAO_BROWSABLE,       _T(" BROWSABLE") },
      { SFGAO_CANCOPY,         _T(" CANCOPY") },
      { SFGAO_CANMOVE,         _T(" CANMOVE") },
      { SFGAO_CANDELETE,       _T(" CANDELETE") },
      { SFGAO_CANRENAME,       _T(" CANRENAME") },
      { SFGAO_CANLINK,         _T(" CANLINK") },
      { SFGAO_SHARE,           _T(" SHARE") },      
      { SFGAO_HIDDEN,          _T(" HIDDEN") },
      { SFGAO_GHOSTED,         _T(" GHOSTED") },
      { SFGAO_READONLY,        _T(" READONLY") },
      { SFGAO_REMOVABLE,       _T(" REMOVABLE") },
      { SFGAO_ENCRYPTED,       _T(" ENCRYPTED") },
      { SFGAO_COMPRESSED,      _T(" COMPRESSED") },
      { SFGAO_ISSLOW,          _T(" ISSLOW") },
      { SFGAO_HASSUBFOLDER,    _T(" HASSUBFOLDER") },
      { SFGAO_DROPTARGET,      _T(" DROPTARGET") },
      { SFGAO_STORAGEANCESTOR, _T(" STORAGEANCESTOR") },
      { SFGAO_FILESYSANCESTOR, _T(" FILESYSANCESTOR") },
      { SFGAO_FILESYSTEM,      _T(" FILESYSTEM") },
      { SFGAO_HASPROPSHEET,    _T(" HASPROPSHEET") },
      { SFGAO_STREAM,          _T(" STREAM") },
      { SFGAO_NONENUMERATED,   _T(" NONENUMERATED") },
      { SFGAO_NEWCONTENT,      _T(" NEWCONTENT") },      
      { SFGAO_VALIDATE,        _T(" VALIDATE") },
   };
   for( int i = 0; i < lengthof(aList); i++ ) {
      if( IsBitSet(Flags, aList[i].f) ) _tcscat_s(szText, lengthof(szText), aList[i].psz);
   }
   if( szText[0] == '\0' ) _tcscpy(szText, _T(" "));
   if( _tcslen(szText) > 140 ) _tcscpy(szText, _T(" ..."));
   static TCHAR s_szText[150] = { 0 };
   _tcscpy(s_szText, szText);
   return s_szText + 1;
}

/**
 * Get flags of PropertyStore GPS type for debug log.
 */
LPCTSTR DbgGetGPSF(GETPROPERTYSTOREFLAGS Flags)
{
   TCHAR szText[100] = { 0 };
   _tcscpy(szText, _T(""));
   static struct { DWORD f; LPCTSTR psz; } aList[] = {
      { GPS_HANDLERPROPERTIESONLY,   _T(" HANDLERPROPERTIESONLY") },
      { GPS_READWRITE,               _T(" READWRITE") },
      { GPS_TEMPORARY,               _T(" TEMPORARY") },
      { GPS_FASTPROPERTIESONLY,      _T(" FASTPROPERTIESONLY") },
      { GPS_OPENSLOWITEM,            _T(" OPENSLOWITEM") },
      { GPS_DELAYCREATION,           _T(" DELAYCREATION") },
      { GPS_BESTEFFORT,              _T(" BESTEFFORT") },
   };
   for( int i = 0; i < lengthof(aList); i++ ) {
      if( IsBitSet(Flags, aList[i].f) ) _tcscat_s(szText, lengthof(szText), aList[i].psz);
   }
   if( szText[0] == '\0' ) _tcscpy(szText, _T(" DEFAULT"));
   static TCHAR s_szText[100] = { 0 };
   _tcscpy(s_szText, szText);
   return s_szText + 1;
}

#endif // _DEBUG

