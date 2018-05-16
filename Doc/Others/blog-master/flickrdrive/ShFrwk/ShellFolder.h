/////////////////////////////////////////////////////////////////////////////
// Shell Extension classes
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2009 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#pragma once


//////////////////////////////////////////////////////////////////////
// CShellFolder

class ATL_NO_VTABLE CShellFolder :
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CShellFolder, &CLSID_ShellFolder>,
   public IShellFolderViewCBImpl<CShellFolder>,
   public IObjectWithSiteImpl<CShellFolder>,
   public IThumbnailHandlerFactory,
   public IExplorerPaneVisibility,
   public IFolderViewSettings,
   public IShellIconOverlay,
   public IPersistFolder3,
   public IItemNameLimits,
   public IObjectProvider,
   public IPersistIDList,
   public IContextMenuCB,
   public IShellFolder2,
   public IShellIcon,
   public IStorage
{
public:
   CRefPtr<CNseFileSystem> m_spFS;               // Reference to the file-system implementation
   CNseItemPtr m_spFolderItem;                   // Reference to NSE Item for the folder
   CPidl m_pidlMonitor;                          // PIDL of the complete folder path (relative to Desktop)
   CPidl m_pidlFolder;                           // PIDL to the folder path (relative to root)
   CPidl m_pidlParent;                           // PIDL of the parent item
   CPidl m_pidlRoot;                             // PIDL of the root (relative to Desktop)
   HWND m_hwndOwner;                             // Owner window handle
   HMENU m_hMenu;                                // Handle to current context-menu
   HMENU m_hContextMenu;                         // Handle to current popup context-menu

   BEGIN_COM_MAP(CShellFolder)
      COM_INTERFACE_ENTRY(IShellFolder)
      COM_INTERFACE_ENTRY(IShellFolder2)
      COM_INTERFACE_ENTRY(IPersistIDList)
      COM_INTERFACE_ENTRY(IShellIcon)
      COM_INTERFACE_ENTRY(IShellIconOverlay)
      COM_INTERFACE_ENTRY2(IPersist, IPersistFolder)
      COM_INTERFACE_ENTRY(IPersistFolder)
      COM_INTERFACE_ENTRY(IPersistFolder2)
      COM_INTERFACE_ENTRY(IPersistFolder3)
      COM_INTERFACE_ENTRY(IStorage)
      COM_INTERFACE_ENTRY(IContextMenuCB)      
      COM_INTERFACE_ENTRY(IItemNameLimits)
      COM_INTERFACE_ENTRY(IObjectProvider)
      COM_INTERFACE_ENTRY(IObjectWithSite)
      COM_INTERFACE_ENTRY(IShellFolderViewCB)
      COM_INTERFACE_ENTRY(IFolderViewSettings)
      COM_INTERFACE_ENTRY(IExplorerPaneVisibility)
      COM_INTERFACE_ENTRY(IThumbnailHandlerFactory)
   END_COM_MAP()

   BEGIN_MSG_MAP(CShellFolder)
      MESSAGE_HANDLER(SFVM_WINDOWCREATED, OnWindowCreated)
      MESSAGE_HANDLER(SFVM_LISTREFRESHED, OnListRefreshed)
      MESSAGE_HANDLER(SFVM_GETNOTIFY, OnGetNotify)
   END_MSG_MAP()

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   static HRESULT WINAPI UpdateRegistry(BOOL bRegister) throw();

   HRESULT FinalConstruct();
   void FinalRelease();

   // IPersist

   STDMETHOD(GetClassID)(CLSID* pclsid);

   // IPersistFolder

   STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidl);

   // IPersistFolder2

   STDMETHOD(GetCurFolder)(PIDLIST_ABSOLUTE* ppidl);

   // IPersistFolder3

   STDMETHOD(InitializeEx)(IBindCtx* pbc, PCIDLIST_ABSOLUTE pidlRoot, const PERSIST_FOLDER_TARGET_INFO* pPFTI);
   STDMETHOD(GetFolderTargetInfo)(PERSIST_FOLDER_TARGET_INFO* pPFTI);

   // IPersistIDlist

   STDMETHOD(SetIDList)(PCIDLIST_ABSOLUTE pidl);
   STDMETHOD(GetIDList)(PIDLIST_ABSOLUTE* ppidl);

   // IShellFolder

   STDMETHOD(ParseDisplayName)(HWND hwnd, IBindCtx* pbc, PWSTR pszDisplayName, ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes);
   STDMETHOD(EnumObjects)(HWND hwnd, SHCONTF grfFlags, IEnumIDList** ppEnumIDList);
   STDMETHOD(BindToObject)(PCUIDLIST_RELATIVE pidl, IBindCtx* pbc, REFIID riid, LPVOID* ppRetVal);
   STDMETHOD(BindToStorage)(PCUIDLIST_RELATIVE pidl, IBindCtx* pbc, REFIID riid, LPVOID* ppRetVal);
   STDMETHOD(CompareIDs)(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2);
   STDMETHOD(CreateViewObject)(HWND hwndOwner, REFIID riid, LPVOID* ppRetVal);
   STDMETHOD(GetAttributesOf)(UINT cidl, PCUITEMID_CHILD_ARRAY rgpidl, SFGAOF* rgfInOut);
   STDMETHOD(GetUIObjectOf)(HWND hwndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY rgpidl, REFIID riid, UINT* rgfReserved, LPVOID* ppRetVal);
   STDMETHOD(GetDisplayNameOf)(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* psrName);
   STDMETHOD(SetNameOf)(HWND hwnd, PCUITEMID_CHILD pidl, LPCWSTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut);

   // IShellFolder2

   STDMETHOD(GetDefaultSearchGUID)(GUID* pguid);
   STDMETHOD(EnumSearches)(IEnumExtraSearch** ppenum);
   STDMETHOD(GetDefaultColumn)(DWORD dwRes, ULONG* pSort, ULONG* pDisplay);
   STDMETHOD(GetDefaultColumnState)(UINT iColumn, SHCOLSTATEF* pcsFlags);
   STDMETHOD(GetDetailsEx)(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv);
   STDMETHOD(GetDetailsOf)(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS* psd);
   STDMETHOD(MapColumnToSCID)(UINT iColumn, SHCOLUMNID* pscid);

   // IShellIcon

   STDMETHOD(GetIconOf)(PCUITEMID_CHILD pidl, UINT uIconFlags, int* pIconIndex);

   // IShellIconOverlay

   STDMETHOD(GetOverlayIndex)(PCUITEMID_CHILD pidl, int* pIndex);
   STDMETHOD(GetOverlayIconIndex)(PCUITEMID_CHILD pidl, int* pIconIndex);

   // IFolderViewSettings

   STDMETHOD(GetColumnPropertyList)(REFIID riid, LPVOID* ppv);       
   STDMETHOD(GetGroupByProperty)(PROPERTYKEY* pkey, BOOL* pfGroupAscending);      
   STDMETHOD(GetViewMode)(FOLDERLOGICALVIEWMODE* plvm);        
   STDMETHOD(GetIconSize)(UINT* puIconSize);        
   STDMETHOD(GetFolderFlags)(FOLDERFLAGS* pfolderMask, FOLDERFLAGS* pfolderFlags);        
   STDMETHOD(GetSortColumns)(SORTCOLUMN* rgSortColumns, UINT cColumnsIn, UINT* pcColumnsOut);        
   STDMETHOD(GetGroupSubsetCount)(UINT* pcVisibleRows);

   // IExplorerPaneVisibility

   STDMETHOD(GetPaneState)(REFEXPLORERPANE ep, EXPLORERPANESTATE* peps);

   // IThumbnailHandlerFactory

   STDMETHOD(GetThumbnailHandler)(PCUITEMID_CHILD pidlChild, IBindCtx *pbc, REFIID riid, LPVOID* ppv);

   // IItemNameLimits

   STDMETHOD(GetValidCharacters)(LPWSTR* ppwszValidChars, LPWSTR* ppwszInvalidChars);
   STDMETHOD(GetMaxLength)(LPCWSTR pszName, int* piMaxNameLen);

   // IObjectProvider

   STDMETHOD(QueryObject)(REFGUID guidObject, REFIID riid, LPVOID* ppv);

   // IStorage

   STDMETHOD(CreateStream)(LPCWSTR pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream** ppstm);        
   STDMETHOD(OpenStream)(LPCWSTR pwcsName, LPVOID reserved1, DWORD grfMode, DWORD reserved2, IStream** ppstm);       
   STDMETHOD(CreateStorage)(LPCWSTR pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage** ppstg);        
   STDMETHOD(OpenStorage)(LPCWSTR pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstg);        
   STDMETHOD(CopyTo)(DWORD ciidExclude, const IID* rgiidExclude, SNB snbExclude, IStorage* pstgDest);        
   STDMETHOD(MoveElementTo)(LPCWSTR pwcsName, IStorage* pstgDest, LPCWSTR pwcsNewName, DWORD grfFlags);        
   STDMETHOD(Commit)(DWORD grfCommitFlags);       
   STDMETHOD(Revert)();
   STDMETHOD(EnumElements)(DWORD reserved1, LPVOID reserved2, DWORD reserved3, IEnumSTATSTG** ppenum);
   STDMETHOD(DestroyElement)(LPCWSTR pwcsName);
   STDMETHOD(RenameElement)(LPCWSTR pwcsOldName, LPCWSTR pwcsNewName);        
   STDMETHOD(SetElementTimes)(LPCWSTR pwcsName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime);
   STDMETHOD(SetClass)(REFCLSID clsid);        
   STDMETHOD(SetStateBits)(DWORD grfStateBits, DWORD grfMask);        
   STDMETHOD(Stat)(STATSTG* pstatstg, DWORD grfStatFlag);

   // IContextMenuCB

   STDMETHOD(CallBack)(IShellFolder* psf, HWND hwndOwner, IDataObject* pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

   // IShellFolderViewCB messages

   LRESULT OnWindowCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnGetNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnListRefreshed(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   // Operations

   HRESULT BindToFolder(PCUITEMID_CHILD pidl, CRefPtr<CShellFolder>& spFolder);
   HRESULT SetItemProperty(PCUITEMID_CHILD pidl, REFPROPERTYKEY pkey, PROPVARIANT* pv); 
   HRESULT GetItemProperty(PCUITEMID_CHILD pidl, REFPROPERTYKEY pkey, PROPVARIANT* pv);
   HRESULT GetItemProperties(PCUITEMID_CHILD pidl, UINT cKeys, const PROPERTYKEY* pkey, PROPVARIANT* pv);
   HRESULT GetItemPropertyState(PCUITEMID_CHILD pidl, REFPROPERTYKEY pkey, VFS_PROPSTATE& State);
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);
   HRESULT CreateMenu(HWND hwndOwner, LPCTSTR pstrMenuType, IContextMenu3** ppRetVal);
   CNseItem* GenerateChildItem(PCUITEMID_CHILD pidlItem, BOOL bReleaseItem = FALSE);
   CNseItem* GenerateChildItemFromShellItem(IShellItem* pShellItem);

   // Implementation

   HRESULT _ParseDisplayNameWithBind(CNseItemPtr& spItem, PWSTR pszDisplayName, IBindCtx* pbc, const BIND_OPTS& Opts);
   HRESULT _GetColumnDetailsOf(const SHCOLUMNID* pscid, SHELLDETAILS* psd) const;
   HRESULT _SetMenuState(HMENU hMenu, IDataObject* pDataObject);
};


