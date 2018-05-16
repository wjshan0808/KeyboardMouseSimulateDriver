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


///////////////////////////////////////////////////////////////////////////////
// Shell Module internals

class CShellFolder;
class CNsePropertyPage;
class CNseFileSystem;
class CNseItem;
class CNseFileStream;

#define VFS_HACK_SAVEAS_JUNCTION  1                        // Used during SaveAs file-dialog processing

#define NSE_S_ALL_DONE  _HRESULT_TYPEDEF_(0x00040E01L)     // MenuCommand was processed for entire selection

typedef VOID (CALLBACK ADDENUMOBJECTCB)(HANDLE hOwner, PCITEMID_CHILD pidl);


///////////////////////////////////////////////////////////////////////////////
// File System configuration

typedef enum VFS_CONFIG {
   VFS_NONE,                                         // Null entry
   VFS_CAN_ATTACHMENTSERVICES,                       // Files can be unsafe; use Windows security prompt before execute
   VFS_CAN_SLOW_ENUM,                                // File may enumerate slowly and async should be preferred
   VFS_CAN_SLOW_COPY,                                // File may copy slowly and progress should be shown
   VFS_CAN_PROGRESSUI,                               // Use default progress dialog for drag'n'drop
   VFS_INSTALL_SENDTO,                              // Has SendTo shell integration
   VFS_INSTALL_PREVIEW,                             // Has Preview shell integration
   VFS_INSTALL_SHELLNEW,                            // Has NewItem shell integration
   VFS_INSTALL_PROPSHEET,                           // Has PropertySheet shell integration
   VFS_INSTALL_DROPTARGET,                          // Has DropTarget shell integration
   VFS_INSTALL_PROPERTIES,                          // Has custom Properties shell integration
   VFS_INSTALL_CONTEXTMENU,                         // Has ContextMenu shell integration
   VFS_INSTALL_URLPROTOCOL,                         // Has URL Protocol search integration
   VFS_INSTALL_CUSTOMSCRIPT,                        // Has a custom registry script
   VFS_INSTALL_STARTMENU_LINK,                      // Installs a shortcut in Start Menu
   VFS_HAVE_CUSTOMPROPERTIES,                        // Our NSE has custom properties
   VFS_HAVE_VIRTUAL_FILES,                           // Our NSE represents virtual files
   VFS_HAVE_IDENTITY,                                // Items have alias identity (ie. maps to real file)
   VFS_HAVE_UNIQUE_NAMES,                            // Items in any folder have unique names
   VFS_HAVE_INFOTIPS,                                // Our NSE has custom info tips
   VFS_HAVE_NAMEOF,                                  // Our NSE wants GetNameOf call
   VFS_HAVE_OBJECTOF,                                // Our NSE wants GetObjectOf call
   VFS_HAVE_SYSICONS,                                // Our NSE responds to the GetSysIcon call
   VFS_HAVE_ICONOVERLAYS,                            // Our NSE uses icon overlays
   VFS_HIDE_DETAILS_PANEL,                           // Hide Details panel in shell view
   VFS_HIDE_PREVIEW_PANEL,                           // Hide Preview panel/option in shell view
   VFS_HIDE_NAVTREE_PANEL,                           // Hide Navigation panel in shell view
   VFS_SHOW_DETAILS_PANEL,                           // Show Details panel in shell view
   VFS_SHOW_PREVIEW_PANEL,                           // Show Preview panel/option in shell view
   VFS_IGNORE_TRANSFER_NOTIFY,                       // We don't send Shell notification during file operations
   VFS_INT_LOCATION,                                 // Location of Shell Extension
   VFS_INT_SHELLROOT_SFGAO,                          // Capabilities of root folder
   VFS_INT_MAX_FILENAME_LENGTH,                      // Maximum filename length
   VFS_INT_MAX_PATHNAME_LENGTH,                      // Maximum pathname length
   VFS_STR_FILENAME_CHARS_ALLOWED,                   // String of characters allowed in a filename
   VFS_STR_FILENAME_CHARS_NOTALLOWED,                // String of characters not allowed in filename
};

enum {
   VFS_LOCATION_JUNCTION      = 0,                   // NSE is extending a file-extension
   VFS_LOCATION_DESKTOP       = 1,                   // NSE is placed on the Desktop
   VFS_LOCATION_MYCOMPUTER    = 2,                   // NSE is placed in the MyComputer folder
   VFS_LOCATION_USERFILES     = 3,                   // NSE is placed on the Document area
   VFS_LOCATION_NETHOOD       = 4,                   // NSE is placed in the Network Neighborhood
   VFS_LOCATION_REMOTE        = 5,                   // NSE is placed under a remote computer
   VFS_LOCATION_PRINTERS      = 6,                   // NSE is placed in the Printers folder
};

enum {
   VFS_COLF_WRITEABLE         = 1,                   // Column is writeable
   VFS_COLF_NOTCOLUMN         = 2,                   // Column is only for property sets
};

enum {
   VFS_PROPSTATE_NORMAL       = 0x0000,              // Property has default state (same as column)
   VFS_PROPSTATE_READONLY     = 0x0001,              // Property is read-only
};

typedef enum VFS_OBJECTOF {
   VFS_OBJECTOF_UI            = 0,                   // IShellFolder::GetUIObjectOf
   VFS_OBJECTOF_VIEW          = 1,                   // IShellFolder::GetViewObject
   VFS_OBJECTOF_BIND          = 2,                   // IShellFolder::BindToObject
   VFS_OBJECTOF_STORAGE       = 3,                   // IShellFolder::BindToStorage
};

typedef enum VFS_REFRESH {
   VFS_REFRESH_VALIDATE       = 0,                   // Request was triggered by GetAttributesOf(SFGAO_VALIDATE)
   VFS_REFRESH_USERFORCED     = 1,                   // Request was triggered by user (F5 key)
};

typedef enum VFS_STREAMTYPE {
   VFS_STREAMTYPE_FILE        = 0,                   // Asking for the file stream
   VFS_STREAMTYPE_THUMBNAIL   = 1,                   // Asking for the thumbnail stream
};

typedef struct tagVFS_STREAM_REASON {
   VFS_STREAMTYPE Type;                              // Data-stream to access in file
   UINT uAccess;                                     // Read- or write-operation (GENERIC_READ, GENERIC_WRITE)
   UINT cxyThumb;                                    // If thumbnail-mode, what size?
} VFS_STREAM_REASON;

typedef DWORD VFS_PROPSTATE;

typedef WIN32_FIND_DATA VFS_FIND_DATA;

typedef struct tagVFS_FOLDERSETTINGS
{
   ULONG uDefaultDisplayColumn;                      // Default display column index
   ULONG uDefaultSortColumn;                         // Default sort column index
   FOLDERLOGICALVIEWMODE ViewMode;                   // Default view-mode for Folder
   PROPERTYKEY GroupByKey;                           // Default GroupBy property
   UINT nGroupVisible;                               // Default GroupBy visible subset rows
   BOOL bGroupByAsc;                                 // Default GroupBy Ascending/descending sort
   UINT cxyIcon;                                     // Default Icon size
   FOLDERFLAGS FlagsMask;                            // Folder-flags mask
   FOLDERFLAGS FlagsValue;                           // Folder-flags
} VFS_FOLDERSETTINGS;

typedef struct tagVFS_COLUMNINFO
{
   PROPERTYKEY pkey;                                 // Property key for column
   SHCOLSTATEF dwFlags;                              // Shell Column State flags
   DWORD dwAttributes;                               // Colunm VFS_COLF flags
} VFS_COLUMNINFO;

typedef struct tagVFS_MENUCOMMAND
{
   HWND hWnd;                                        // Window handle for shell
   UINT wMenuID;                                     // Menu ID that was invoked
   LPCSTR pstrVerb;                                  // Verb that was invoked
   DWORD dwDropEffect;                               // Effect used during "paste" operation
   IDataObject* pDataObject;                         // Selected items (data object)
   IShellItemArray* pShellItems;                     // Selected items (collection)
   IFileOperation* pFO;                              // Optional file operation
   IUnknown* punkSite;                               // Object Site
   LPVOID pUserData;                                 // Optional user data (use malloc)
} VFS_MENUCOMMAND;

typedef struct tagVFS_MENUSTATE
{
   HMENU hMenu;                                      // Submenu to update state for
   IShellItemArray* pShellItems;                     // Selected items (collection)
} VFS_MENUSTATE;

#define VFS_MNUCMD_NOVERB  NULL


///////////////////////////////////////////////////////////////////////////////
// Shell Module

class CNseModule
{
public:
   virtual ~CNseModule() { };

   virtual LONG GetConfigInt(VFS_CONFIG Item) = 0;
   virtual BOOL GetConfigBool(VFS_CONFIG Item) = 0;
   virtual LPCWSTR GetConfigStr(VFS_CONFIG Item) = 0;

   virtual HRESULT CreateFileSystem(PCIDLIST_ABSOLUTE pidlRoot, CNseFileSystem** pFS) = 0;

   virtual HRESULT DllInstall() = 0;
   virtual HRESULT DllUninstall() = 0;
   virtual HRESULT ShellAction(LPCWSTR pstrType, LPCWSTR pstrCmdLine) = 0;

   virtual BOOL DllMain(DWORD dwReason, LPVOID lpReserved) = 0;
};


///////////////////////////////////////////////////////////////////////////////
// NSE File System

class CNseFileSystem
{
public:
   virtual ~CNseFileSystem() { };

   virtual VOID AddRef() = 0;
   virtual VOID Release() = 0;

   virtual CNseItem* GenerateRoot(CShellFolder* pShellFolder) = 0;

   static WCHAR m_wszOpenSaveAsFilename[MAX_PATH];       // Temp. filename used during FileSave dialog
};


///////////////////////////////////////////////////////////////////////////////
// NSE Item

class CNseItem
{
public:
   virtual ~CNseItem() { };

   // General Item support

   virtual BYTE GetType() = 0;
   virtual BOOL IsRoot() = 0;
   virtual BOOL IsFolder() = 0;

   virtual HRESULT GetNameOf(SHGDNF uFlags, LPWSTR* ppstrRetVal) = 0;
   virtual HRESULT GetObjectOf(VFS_OBJECTOF Type, HWND hwndOwner, REFIID riid, LPVOID* ppRetVal) = 0;

   virtual HRESULT GetPropertyPage(CNsePropertyPage** ppPage) = 0;
   virtual HRESULT GetThumbnail(REFIID riid, LPVOID* ppRetVal) = 0;
   virtual HRESULT GetPreview(REFIID riid, LPVOID* ppRetVal) = 0;
   virtual HRESULT GetSysIcon(UINT uIconFlags, int* pIconIndex) = 0;
   virtual HRESULT GetMoniker(IMoniker** ppMoniker) = 0;
   virtual HRESULT GetIconOverlay(int* pIconIndex) = 0;
   virtual HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal) = 0;
   virtual HRESULT GetInfoTip(DWORD dwFlags, LPWSTR* ppstr) = 0;

   virtual HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v) = 0;
   virtual HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v) = 0;
   virtual VFS_PROPSTATE GetPropertyState(REFPROPERTYKEY pkey) = 0;

   virtual HRESULT SetDataObject(IDataObject* pDataObj) = 0;
   virtual HRESULT IsDropDataAvailable(IDataObject* pDataObj) = 0;
   virtual HRESULT TransferDone() = 0;

   // Folder Item support

   virtual HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column) = 0;
   virtual HRESULT GetPaneState(REFEXPLORERPANE ep, EXPLORERPANESTATE* peps) = 0;
   virtual VFS_FOLDERSETTINGS GetFolderSettings() = 0;

   virtual CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem) = 0;
   virtual CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd) = 0;

   virtual HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem) = 0;
   virtual HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleArray<CNseItem*>& aList) = 0;
   virtual HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, HANDLE hAsync, ADDENUMOBJECTCB fnCallback) = 0;

   // Item data support

   virtual SFGAOF GetSFGAOF(SFGAOF dwMask) = 0;
   virtual VFS_FIND_DATA GetFindData() = 0;
   virtual PCUITEMID_CHILD GetITEMID() = 0;

   virtual HMENU GetMenu() = 0;
   virtual HRESULT SetMenuState(const VFS_MENUSTATE& State) = 0;
   virtual HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd) = 0;
 
   virtual HRESULT GetStream(const VFS_STREAM_REASON& Reason, CNseFileStream** ppFile) = 0;
   virtual HRESULT GetIdentity(LPWSTR pstrFilename) = 0;

   virtual HRESULT CreateFolder() = 0;
   virtual HRESULT Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName) = 0;
   virtual HRESULT Delete() = 0;
   virtual HRESULT Refresh(VFS_REFRESH Reason) = 0;
};


///////////////////////////////////////////////////////////////////////////////
// NSE File Stream

class CNseFileStream
{
public:
   virtual ~CNseFileStream() { };

   virtual HRESULT Init() = 0;
   virtual HRESULT Read(LPVOID pData, ULONG dwSize, ULONG& dwRead) = 0;
   virtual HRESULT Write(LPCVOID pData, ULONG dwSize, ULONG& dwWritten) = 0;
   virtual HRESULT Seek(DWORD dwPos) = 0;
   virtual HRESULT GetCurPos(DWORD* pdwPos) = 0;
   virtual HRESULT GetFileSize(DWORD* pdwFileSize) = 0;
   virtual HRESULT SetFileSize(DWORD dwSize) = 0;
   virtual HRESULT Commit() = 0;
   virtual HRESULT Close() = 0;
};


///////////////////////////////////////////////////////////////////////////////
// NSE Base Item - default item implementation

class CNseBaseItem : public CNseItem
{
public:
   CShellFolder* m_pFolder;                // Reference to shell folder
   PCIDLIST_RELATIVE m_pidlFolder;         // Path PIDL of item
   PCITEMID_CHILD m_pidlItem;              // PIDL of item
   BOOL m_bReleaseItem;                    // Release PIDL item data on destruction?

   CNseBaseItem(CShellFolder* pShellFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   virtual ~CNseBaseItem();

   // CNseItem

   BOOL IsRoot();
   BOOL IsFolder();

   HRESULT GetNameOf(SHGDNF uFlags, LPWSTR* ppstrRetVal);
   HRESULT GetObjectOf(VFS_OBJECTOF Type, HWND hwndOwner, REFIID riid, LPVOID* ppRetVal);

   HRESULT GetPropertyPage(CNsePropertyPage** ppPage);
   HRESULT GetThumbnail(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetPreview(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetSysIcon(UINT uIconFlags, int* pIconIndex);
   HRESULT GetMoniker(IMoniker** ppMoniker);
   HRESULT GetIconOverlay(int* pIconIndex);
   HRESULT GetInfoTip(DWORD dwFlags, LPWSTR* ppstr);

   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v);
   VFS_PROPSTATE GetPropertyState(REFPROPERTYKEY pkey);

   HRESULT SetDataObject(IDataObject* pDataObj);
   HRESULT IsDropDataAvailable(IDataObject* pDataObj);
   HRESULT TransferDone();

   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column);
   HRESULT GetPaneState(REFEXPLORERPANE ep, EXPLORERPANESTATE* peps);
   VFS_FOLDERSETTINGS GetFolderSettings();

   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, HANDLE hAsync, ADDENUMOBJECTCB fnCallback);

   PCITEMID_CHILD GetITEMID();

   HMENU GetMenu();
   HRESULT SetMenuState(const VFS_MENUSTATE& State);
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   HRESULT GetStream(const VFS_STREAM_REASON& Reason, CNseFileStream** ppFile);
   HRESULT GetIdentity(LPWSTR pstrFilename);

   HRESULT CreateFolder();
   HRESULT Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName);
   HRESULT Delete();
   HRESULT Refresh(VFS_REFRESH Reason);

   // Implementation

   CPidl _GetFullPidl() const;

   HRESULT _RefreshRootView();
   HRESULT _RefreshFolderView();
   HRESULT _RefreshItemInView();
   HRESULT _DoPasteFiles(VFS_MENUCOMMAND& Cmd);
   HRESULT _DoShowProperties(VFS_MENUCOMMAND& Cmd);
   HRESULT _DoNewFolder(VFS_MENUCOMMAND& Cmd, UINT uLabelRes);
   HRESULT _AddSelectEdit(const VFS_MENUCOMMAND& Cmd, LPCWSTR pszLabel, DWORD dwFlags = SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE | SVSI_SELECT | SVSI_EDIT);

   // Static members

   static PCITEMID_CHILD GenerateITEMID(LPVOID pData, SIZE_T cbData);
};


///////////////////////////////////////////////////////////////////////////////
// NSE File Item - Default virtual folder/file implementation

#if !defined(_M_X64) && !defined(_M_IA64)
#include <pshpack1.h>
#endif // _M_X64
typedef struct tagNSEFILEPIDLDATA
{
   // SHITEMID 
   USHORT cb;
   // Type identifiers
   BYTE magic;
   BYTE reserved;
   // File data
   WIN32_FIND_DATA wfd;
#if defined(_M_X64) || defined(_M_IA64)
   // Alignment on 64bit platform
   SHORT padding;
#endif // _M_X64
} NSEFILEPIDLDATA;
#if !defined(_M_X64) && !defined(_M_IA64)
#include <poppack.h>
#endif // _M_X64


class CNseFileItem : public CNseBaseItem
{
public:
   WIN32_FIND_DATA* m_pWfd;                // Reference to data inside PIDL

   CNseFileItem(CShellFolder* pShellFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseItem

   BYTE GetType();
   BOOL IsFolder();

   HRESULT GetPropertyPage(CNsePropertyPage** ppPage);
   HRESULT GetSysIcon(UINT uIconFlags, int* pIconIndex);
   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);

   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v);

   HRESULT IsDropDataAvailable(IDataObject* pDataObj);
   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO &Column);
   VFS_FOLDERSETTINGS GetFolderSettings();

   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_FIND_DATA GetFindData();

   // Implementation

   HRESULT _GetPathnameQuick(PCIDLIST_RELATIVE pidlPath, PCITEMID_CHILD pidlChild, LPWSTR pszPath) const;
};


///////////////////////////////////////////////////////////////////////////////
// NSE Helpers

class CNseItemPtr
{
public:
   CNseItem* m_p;

   CNseItemPtr() : m_p(NULL) { };
   CNseItemPtr(CNseItem* p) : m_p(p) { };
   ~CNseItemPtr() { delete m_p; };
   operator CNseItem*() { return m_p; };
   void Free() { delete m_p; m_p = NULL; };
   CNseItem** operator&() { ATLASSERT(m_p==NULL); return &m_p; };
   CNseItem* operator->() { ATLASSERT(m_p); return m_p; };
   CNseItem* operator=(CNseItem* p) { delete m_p; m_p = p; return *this; };

private:
   CNseItemPtr(const CNseItemPtr&);
   CNseItem* operator=(const CNseItemPtr&);
};


class CNseItemArray : public CSimpleValArray<CNseItem*>
{
public:
   ~CNseItemArray() 
   { 
      for( int i = 0; i < GetSize(); i++ ) delete m_aT[i]; 
      RemoveAll();
   }
};


///////////////////////////////////////////////////////////////////////////////
// PIDL Wrapper Helpers

template< typename T >
class CPidlMemPtr
{
public:
   const T* m_p;

   CPidlMemPtr() : m_p(NULL) { };
   CPidlMemPtr(const CPidlMemPtr& p) : m_p(p.m_p) { };
   CPidlMemPtr(PCUITEMID_CHILD pidl) { m_p = reinterpret_cast<const T*>(pidl); ATLASSERT(ILIsAligned64(pidl)); };
   CPidlMemPtr(PCUIDLIST_RELATIVE pidl) { m_p = reinterpret_cast<const T*>(pidl); ATLASSERT(ILIsAligned64(pidl)); };
   ATLINLINE bool IsValid() { return m_p != NULL && m_p->cb >= sizeof(T); };
   ATLINLINE bool IsType(BYTE magic) { return m_p != NULL && m_p->cb >= sizeof(T) && m_p->magic == magic; };
   operator const T*() { return m_p; };
   const T* operator->() { ATLASSERT(m_p); return m_p; };
};

