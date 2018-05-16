
#include "stdafx.h"

#include "NseFileSystem.h"
#include "ShellFolder.h"
#include "PropSheetExt.h"


///////////////////////////////////////////////////////////////////////////////
// NSE Base Item - default item implementation

CNseBaseItem::CNseBaseItem(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem) :
   m_pFolder(pFolder), 
   m_pidlFolder(pidlFolder), 
   m_pidlItem(pidlItem), 
   m_bReleaseItem(bReleaseItem)
{
   // NOTE: We don't own the references for the CShellFolder or the PIDL for the folder.
   //       This is an optimization since many NSE Items could be generated.
   //       We may or may not own the child PIDL for the item. The bReleaseItem argument
   //       indicates if we need to release this memory (see destructor).

   // Our PIDL structure should always be DWORD/QWORD aligned
   ATLASSERT(ILIsAligned64(pidlItem));
   ATLASSERT(pidlItem == NULL || (pidlItem->mkid.cb % sizeof(DWORD_PTR)) == 0);
}

CNseBaseItem::~CNseBaseItem()
{
   if( m_bReleaseItem ) ::CoTaskMemFree( (LPVOID) m_pidlItem );
}

/**
 * Get Display/parsing name for Shell.
 * This method allows us to override all calls to IShellFolder::GetDisplayNameOf
 * for any folder or item.
 * This feature requires the VFS_HAVE_NAMEOF configuration flag.
 */
HRESULT CNseBaseItem::GetNameOf(SHGDNF uFlags, LPWSTR* ppstrRetVal)
{
   return E_NOTIMPL;
}

/**
 * Get COM object for Shell.
 * This method allows us to override all calls to IShellFolder::GetViewObject and
 * IShellFolder::GetUIObjectOf to any folder or item.
 * This feature requires the VFS_HAVE_OBJECTOF configuration flag.
 */
HRESULT CNseBaseItem::GetObjectOf(VFS_OBJECTOF Type, HWND hwndOwner, REFIID riid, LPVOID* ppRetVal)
{
   return E_NOTIMPL;
}

/**
 * Get system icon index.
 * Return the System Icon index if the file can be mapped to a file.
 * This feature requires the VFS_HAVE_SYSICONS configuration flag.
 * Return S_FALSE if no system index exists.
 */
HRESULT CNseBaseItem::GetSysIcon(UINT uIconFlags, int* pIconIndex)
{
   return S_FALSE;
}

/**
 * Get system icon overlay index.
 * Return the System Icon Overlay index for the item.
 * This feature requires the VFS_HAVE_ICONOVERLAYS configuration flag.
 * Return S_FALSE if no overlay is used.
 */
HRESULT CNseBaseItem::GetIconOverlay(int* pIconIndex)
{
   return S_FALSE;
}

/**
 * Add clipboard data to prefabricated data-object.
 * Allow the NSE Item to add extra clipboard data to the data-object during copy'n'paste and
 * drag'n'drop operations.
 */
HRESULT CNseBaseItem::SetDataObject(IDataObject* pData)
{
   return E_NOTIMPL;
}

/**
 * Returns whether the drop is allowed.
 * Return S_FALSE to deny the drop on the folder.
 */
HRESULT CNseBaseItem::IsDropDataAvailable(IDataObject* pDataObj)
{
   return S_FALSE;
}

/**
 * Notifies that data transfer has completed.
 */
HRESULT CNseBaseItem::TransferDone()
{
   return S_OK;
}

/**
 * Return a thumbnail handler if the NSE Item can provide a thumbnail.
 */
HRESULT CNseBaseItem::GetThumbnail(REFIID riid, LPVOID* ppRetVal)
{
   return E_NOTIMPL;
}

/**
 * Return a preview handler if the NSE Item can provide a preview.
 */
HRESULT CNseBaseItem::GetPreview(REFIID riid, LPVOID* ppRetVal)
{
   return E_NOTIMPL;
}

/**
 * Return a moniker if the NSE Item can provide one.
 */
HRESULT CNseBaseItem::GetMoniker(IMoniker** ppMoniker)
{
   return E_NOTIMPL;
}

/**
 * Return a Property Page for the NSE Item.
 */
HRESULT CNseBaseItem::GetPropertyPage(CNsePropertyPage** ppPage)
{
   return E_NOTIMPL;
}

/**
 * Return a custom info-tip (tooltip) for the NSE Item.
 * This feature requires the VFS_HAVE_INFOTIPS configuration flag.
 */
HRESULT CNseBaseItem::GetInfoTip(DWORD dwFlags, LPWSTR* ppstr)
{
   return E_NOTIMPL;
}

/**
 * Return if Shell panel should be visible.
 * Modify the peps flags to hide a particular Shell navigation/query panel.
 */
HRESULT CNseBaseItem::GetPaneState(REFEXPLORERPANE ep, EXPLORERPANESTATE* peps)
{
   return S_OK;
}

/**
 * Return column information for the folder columns.
 * A folder item contains a list of columns that is to be displayed for the contained
 * child items.
 */
HRESULT CNseBaseItem::GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column)
{
   // TODO: You're a folder; override and implement this...
   ATLASSERT(false);
   return E_NOTIMPL;
}

/**
 * Return property information.
 */
HRESULT CNseBaseItem::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   ATLASSERT(v.vt==VT_EMPTY);
   if( pkey == PKEY_ParsingName ) {
      const WIN32_FIND_DATA wfd = GetFindData();
      return ::InitPropVariantFromString(wfd.cFileName, &v);
   }
   if( pkey == PKEY_ItemName ) {
      const WIN32_FIND_DATA wfd = GetFindData();
      return ::InitPropVariantFromString(wfd.cFileName, &v);
   }
   if( pkey == PKEY_ItemNameDisplay ) {
      const WIN32_FIND_DATA wfd = GetFindData();
      return ::InitPropVariantFromString(wfd.cFileName, &v);
   }
   if( pkey == PKEY_FileName ) {
      const WIN32_FIND_DATA wfd = GetFindData();
      return ::InitPropVariantFromString(wfd.cFileName, &v);
   }
   if( pkey == PKEY_FileAttributes ) {
      const WIN32_FIND_DATA wfd = GetFindData();
      return ::InitPropVariantFromUInt32(wfd.dwFileAttributes, &v);
   }
   if( pkey == PKEY_ParsingPath ) {
      CCoTaskString strPath;
      CPidl pidlFull(m_pFolder->m_pidlRoot, m_pidlFolder, m_pidlItem);
      HR( ::SHGetNameFromIDList(pidlFull, SIGDN_DESKTOPABSOLUTEPARSING, &strPath) );
      return ::InitPropVariantFromString(strPath, &v);
   }
   if( pkey == PKEY_ItemPathDisplay ) {
      CCoTaskString strPath;
      CPidl pidlFull(m_pFolder->m_pidlRoot, m_pidlFolder, m_pidlItem);
      HR( ::SHGetNameFromIDList(pidlFull, SIGDN_DESKTOPABSOLUTEEDITING, &strPath) );
      return ::InitPropVariantFromString(strPath, &v);
   }
   if( pkey == PKEY_ItemPathDisplayNarrow ) {
      CComPropVariant vDispName;
      HR( GetProperty(PKEY_ItemNameDisplay, vDispName) );
      HR( vDispName.ChangeType(VT_LPWSTR) );
      CCoTaskString strParent;
      HR( ::SHGetNameFromIDList(m_pFolder->m_pidlMonitor, SIGDN_DESKTOPABSOLUTEEDITING, &strParent) );
      WCHAR wszNarrow[MAX_PATH + 4] = { 0 };
      ::wnsprintf(wszNarrow, lengthof(wszNarrow) - 1, L"%s (%s)", vDispName.pwszVal, static_cast<LPCWSTR>(strParent));
      return ::InitPropVariantFromString(strParent.IsEmpty() ? vDispName.pwszVal : wszNarrow, &v);
   }
   if( pkey == PKEY_ItemFolderPathDisplay ) {
      CCoTaskString strFolder;
      HR( ::SHGetNameFromIDList(m_pFolder->m_pidlMonitor, SIGDN_DESKTOPABSOLUTEEDITING, &strFolder) );
      return ::InitPropVariantFromString(strFolder, &v);
   }
   if( pkey == PKEY_ItemFolderPathDisplayNarrow ) {
      CCoTaskString strFolder;
      HR( ::SHGetNameFromIDList(m_pFolder->m_pidlMonitor, SIGDN_DESKTOPABSOLUTEEDITING, &strFolder) );
      LPWSTR pstrSep = wcsrchr(strFolder, '\\');
      if( pstrSep == NULL ) pstrSep = L"\0\0"; else *pstrSep = '\0';
      WCHAR wszNarrow[MAX_PATH + 4] = { 0 };
      ::wnsprintf(wszNarrow, lengthof(wszNarrow) - 1, L"%s (%s)", pstrSep + 1, static_cast<LPCWSTR>(strFolder));
      return ::InitPropVariantFromString(pstrSep[1] == '\0' ? static_cast<LPCWSTR>(strFolder) : wszNarrow, &v);
   }
   if( pkey == PKEY_ItemType ) {
      // Assume no canonical type; return V_EMPTY
      return S_OK;
   }
   if( pkey == PKEY_ItemTypeText ) {
      // Assume no canonical type; return V_EMPTY
      return S_OK;
   }
   if( pkey == PKEY_PerceivedType ) {
      return ::InitPropVariantFromInt32(PERCEIVED_TYPE_UNKNOWN, &v);
   }
   if( pkey == PKEY_FindData ) {
      const WIN32_FIND_DATA wfd = GetFindData();
      return ::InitPropVariantFromBuffer(&wfd, sizeof(WIN32_FIND_DATAW), &v);
   }
   if( pkey == PKEY_SFGAOFlags ) {
      return ::InitPropVariantFromUInt32(GetSFGAOF((SFGAOF)(~SFGAO_VALIDATE)), &v);
   }
   if( pkey == PKEY_DescriptionID ) {
      SHDESCRIPTIONID did = { SHDID_FS_OTHER, CLSID_ShellFolder };
      return ::InitPropVariantFromBuffer(&did, sizeof(did), &v);
   }
   if( pkey == PKEY_Volume_IsRoot ) {
      return ::InitPropVariantFromBoolean(IsRoot(), &v);
   }
#if _WINVER > _WIN32_WINNT_LONGHORN
   if( pkey == PKEY_NamespaceCLSID ) {
      return ::InitPropVariantFromCLSID(CLSID_ShellFolder, &v);
   }
   if( pkey == PKEY_IsPinnedToNameSpaceTree ) {
      return ::InitPropVariantFromBoolean(IsRoot(), &v);
   }
#endif // _WIN32_WINNT_LONGHORN
   return E_FAIL;
}

/**
 * Set property information.
 */
HRESULT CNseBaseItem::SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v)
{
   return E_FAIL;
}

/**
 * Return whether the NSE Item is the root item.
 */
BOOL CNseBaseItem::IsRoot()
{
   return ::ILIsEmpty(m_pidlFolder) && ::ILIsEmpty(m_pidlItem);
}

/**
 * Return whether the NSE Item is a folder or not.
 */
BOOL CNseBaseItem::IsFolder()
{
   return IsBitSet(GetSFGAOF(SFGAO_FOLDER), SFGAO_FOLDER);
}

/**
 * Return the folder settings.
 * The Folder Settings structure contains optional values for the initial
 * display of the folder.
 */
VFS_FOLDERSETTINGS CNseBaseItem::GetFolderSettings()
{
   VFS_FOLDERSETTINGS Settings = { 0 };
   return Settings;
}

/**
 * Get the item PIDL.
 */
PCITEMID_CHILD CNseBaseItem::GetITEMID()
{
   return m_pidlItem;
}

/**
 * Return the property state.
 */
VFS_PROPSTATE CNseBaseItem::GetPropertyState(REFPROPERTYKEY pk)
{
   return VFS_PROPSTATE_NORMAL;
}

/**
 * Return the menu-items for the NSE Item.
 * Several named popup sub-menus can exist:
 *   - ExplorerMenu  = the menu for the NSE root 
 *   - ContextMenu   = the menu for an item/folder
 *   - ViewMenu      = the background view menu
 *   - CommandMenu   = items for the Explorer Command Bar
 */
HMENU CNseBaseItem::GetMenu()
{
   return NULL;
}

/**
 * Enable or disable menu-items based on the current state of the NSE Item.
 */
HRESULT CNseBaseItem::SetMenuState(const VFS_MENUSTATE& State)
{
   return E_NOTIMPL;
}

/**
 * Execute a menu command.
 * The source of menu-ids can be:
 *   - Menu
 *   - Explorer Command Bar
 *   - Standard verb (DFM_CMD_XXX)
 * The menu-id may not always be given, only a verb string.
 */
HRESULT CNseBaseItem::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   return E_NOTIMPL;
}

/**
 * Return a named child item.
 * The ParseType can be one of the following values:
 *   - SHGDN_FORPARSING  = the parsing name
 *   - SHGDN_FOREDITING  = the display name
 */
HRESULT CNseBaseItem::GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem)
{
   // We provide a default implementation if none was defined as an override. This
   // will be somewhat slower than a direct lookup, so consider writing the override in
   // the derived class. This method is used for name parsing and rename/delete operations.
   ATLASSERT(_ShellModule.GetConfigBool(VFS_HAVE_UNIQUE_NAMES));
   ATLASSERT(*pItem==NULL);
   ATLASSERT(IsFolder());
   CNseItemArray aChildren;
   HR( EnumChildren(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, aChildren) );
   REFPROPERTYKEY pkey = IsBitSet(ParseType, SHGDN_FORPARSING) ? PKEY_ParsingName : PKEY_ItemName;
   for( int i = 0; i < aChildren.GetSize(); i++ ) {
      CComPropVariant v;
      aChildren[i]->GetProperty(pkey, v);
      if( v.vt == VT_LPWSTR && v.pwszVal != NULL && wcscmp(v.pwszVal, pstrName) == 0 ) {
         *pItem = aChildren[i];
         return aChildren.RemoveAt(i) ? S_OK : E_UNEXPECTED;
      }
   }
   return E_FAIL;
}

/**
 * Return a list of child items.
 */
HRESULT CNseBaseItem::EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList)
{
   // TODO: You're a folder; override and implement this...
   ATLASSERT(false);
   return E_NOTIMPL;
}

/**
 * Return a list of child items (for async).
 */
HRESULT CNseBaseItem::EnumChildren(HWND hwndOwner, SHCONTF grfFlags, HANDLE hAsync, ADDENUMOBJECTCB fnCallback)
{
   // TODO: You're a folder; override and implement this if ASYNC enums are used...
   ATLASSERT(false);
   return E_NOTIMPL;
}

/**
 * Creates a NSE Item from static data.
 */
CNseItem* CNseBaseItem::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem)
{
   // TODO: You're a folder; override and implement this...
   ATLASSERT(false);
   return NULL;
}

/**
 * Create a NSE Item from static data.
 */
CNseItem* CNseBaseItem::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd)
{
   // TODO: You're a folder; override and implement this...
   ATLASSERT(false);
   return NULL;
}

/**
 * Create a file-stream.
 */
HRESULT CNseBaseItem::GetStream(const VFS_STREAM_REASON& Reason, CNseFileStream** ppFile)
{
   return E_NOTIMPL;
}

/**
 * Returns the alias identity of the NSE Item.
 */ 
HRESULT CNseBaseItem::GetIdentity(LPWSTR pstrFilename)
{
   return E_NOTIMPL;
}

/**
 * Physically create the NSE Item as a folder.
 */
HRESULT CNseBaseItem::CreateFolder()
{
   return E_NOTIMPL;
}

/**
 * Physically rename this NSE Item.
 */
HRESULT CNseBaseItem::Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName)
{
   return E_NOTIMPL;
}

/**
 * Physically delete this NSE Item.
 */
HRESULT CNseBaseItem::Delete()
{
   return E_NOTIMPL;
}

/**
 * Force a refresh of children list.
 */
HRESULT CNseBaseItem::Refresh(VFS_REFRESH Reason)
{
   return E_NOTIMPL;
}

// Static members

/**
 * Serialize data to a SHITEMID item.
 * This is a helper function designed to ease the process of serializing internal
 * item structures to a Simple PIDL structure.
 */
PCITEMID_CHILD CNseBaseItem::GenerateITEMID(LPVOID pData, SIZE_T cbData)
{
   // NOTE: This method assumes that pData points to a structure
   //       which derives (or has same layout) as the SHITEMID
   //       structure (= a basic PIDL).
   // TODO: Type-safety?
   ATLASSERT(pData);
   ATLASSERT(cbData>=sizeof(SHITEMID));
   LPSHITEMID pSHID = reinterpret_cast<LPSHITEMID>(pData);
   pSHID->cb = (USHORT) cbData;
   CPidl pidl;
   pidl.Create(pSHID);
   return (PCITEMID_CHILD) pidl.Detach();
}

// Implementation

/**
 * Returns the full item IDLIST.
 * The full list includes the root path, the relative folder and the child item.
 */
CPidl CNseBaseItem::_GetFullPidl() const
{
   return CPidl(m_pFolder->m_pidlRoot, m_pidlFolder, m_pidlItem);
}

/**
 * Update root of Shell Extension.
 */
HRESULT CNseBaseItem::_RefreshRootView()
{
   ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, m_pFolder->m_pidlRoot);
   return S_OK;
}

/**
 * Update entire folder (item's parent) in Shell View.
 */
HRESULT CNseBaseItem::_RefreshFolderView()
{
   ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, m_pFolder->m_pidlMonitor);
   return S_OK;
}

/**
 * Update item in Shell View.
 */
HRESULT CNseBaseItem::_RefreshItemInView()
{
   ::SHChangeNotify(IsFolder() ? SHCNE_UPDATEDIR : SHCDF_UPDATEITEM, SHCNF_IDLIST | SHCNF_FLUSH, _GetFullPidl());
   return S_OK;
}

/**
 * Paste a data-object as a buch of files.
 * Assumes that the passed IDataObject contains files to paste into
 * our virtual file-system.
 */
HRESULT CNseBaseItem::_DoPasteFiles(VFS_MENUCOMMAND& Cmd)
{
   ATLASSERT(IsFolder());
   ATLASSERT(Cmd.pDataObject);
   if( Cmd.pDataObject == NULL ) return E_FAIL;
   // Do paste operation...
   if( Cmd.pFO == NULL ) {
      HR( ::SHCreateFileOperation(Cmd.hWnd, FOF_NOCOPYSECURITYATTRIBS | FOF_NOCONFIRMMKDIR | FOFX_NOSKIPJUNCTIONS, &Cmd.pFO) );
   }
   // FIX: The Shell complains about E_INVALIDARG on IFileOperation::MoveItems() so we'll
   //      do the file-operation in two steps.
   CComPtr<IShellItem> spTargetFolder;
   HR( ::SHCreateItemFromIDList(_GetFullPidl(), IID_PPV_ARGS(&spTargetFolder)) );
   Cmd.pFO->CopyItems(Cmd.pDataObject, spTargetFolder);
   if( Cmd.dwDropEffect == DROPEFFECT_MOVE ) Cmd.pFO->DeleteItems(Cmd.pDataObject);
   // We handled this operation successfully for all items in selection
   return NSE_S_ALL_DONE;
}

/**
 * Show file properties.
 * Launches the default File Properties window for the selected item.
 * This method can only handle 1 item and will only show properties
 * for the first item in the passed selection.
 */
HRESULT CNseBaseItem::_DoShowProperties(VFS_MENUCOMMAND& Cmd)
{
   // Get first item or view from selection
   CComPtr<IShellItem> spItem;
   if( Cmd.pShellItems != NULL ) Cmd.pShellItems->GetItemAt(0, &spItem);
   else ::SHCreateItemFromIDList(m_pFolder->m_pidlMonitor, IID_PPV_ARGS(&spItem));
   if( spItem == NULL ) return E_FAIL;
   CPidl pidl;
   HR( pidl.CreateFromObject(spItem) );
   // Show properties for this item
   SHELLEXECUTEINFO sei = { 0 };
   sei.cbSize = sizeof(sei);
   sei.hwnd = Cmd.hWnd;
   sei.fMask = SEE_MASK_INVOKEIDLIST;
   sei.lpVerb = _T("properties");
   sei.lpIDList = pidl;
   sei.nShow = SW_SHOW;
   ::ShellExecuteEx(&sei);
   // We handled this operation successfully for all items in selection
   return NSE_S_ALL_DONE;
}

/**
 * Create the item as a new folder in the virtual file-system.
 */
HRESULT CNseBaseItem::_DoNewFolder(VFS_MENUCOMMAND& Cmd, UINT uLabelRes)
{
   ATLASSERT(IsFolder());
   // The new folder's label
   CComBSTR bstrLabel;
   bstrLabel.LoadString(uLabelRes);
   // Create a new folder.
   // TODO: Support FOF_RENAMEONCOLLISION flag.
   CComPtr<IFileOperation> spFO;
   HR( ::SHCreateFileOperation(Cmd.hWnd, FOF_SILENT | FOF_NOCONFIRMATION | FOFX_NOSKIPJUNCTIONS, &spFO) );
   CComPtr<IShellItem> spTargetFolder;
   HR( ::SHCreateItemFromIDList(m_pFolder->m_pidlMonitor, IID_PPV_ARGS(&spTargetFolder)) );
   spFO->NewItem(spTargetFolder, FILE_ATTRIBUTE_DIRECTORY, bstrLabel, NULL, NULL);
   HR( spFO->PerformOperations() );
   // Go into edit-mode for new item
   HR( _AddSelectEdit(Cmd, bstrLabel) );
   // We handled this operation successfully for all items in selection
   return NSE_S_ALL_DONE;
}

/**
 * Add new item, select it and go into rename-mode.
 * This is a helper function for the NewFolder operation. After creating the folder
 * in the virtual file-system, we add the item temporarily to the Shell view and
 * go right into edit-mode, allowing the user to rename the item.
 */
HRESULT CNseBaseItem::_AddSelectEdit(const VFS_MENUCOMMAND& Cmd, LPCWSTR pszLabel, DWORD dwFlags /*= SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE | SVSI_SELECT | SVSI_EDIT*/)
{
   ATLASSERT(IsFolder());
   // Validate label
   if( pszLabel == NULL ) pszLabel = L"";
   // Get to the IFolderView2 interface
   CComQIPtr<IServiceProvider> spService = Cmd.punkSite;
   if( spService == NULL ) return E_NOINTERFACE;
   CComPtr<IFolderView2> spFV2;
   HR( spService->QueryService(SID_SFolderView, IID_PPV_ARGS(&spFV2)) );
   // Tell View to add item and rename it at once
   CNseItemPtr spNewItem;
   HR( GetChild(pszLabel, SHGDN_FOREDITING, &spNewItem) );
   PCUITEMID_CHILD pidlNewItem = spNewItem->GetITEMID();
   ShellFolderView_AddObject(m_pFolder->m_hwndOwner, ::ILCloneChild(pidlNewItem));
   POINT pt = { 0, 0 };
   HR( spFV2->SelectAndPositionItems(1, &pidlNewItem, &pt, dwFlags) );
   return S_OK;
}
