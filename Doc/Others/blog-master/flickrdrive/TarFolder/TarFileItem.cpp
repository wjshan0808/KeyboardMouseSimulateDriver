
#include "stdafx.h"

#include "TarFileSystem.h"
#include "ShellFolder.h"
#include "LaunchFile.h"
#include "EnumIDList.h"


///////////////////////////////////////////////////////////////////////////////
// CTarFileItem

CTarFileItem::CTarFileItem(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem) :
   CNseFileItem(pFolder, pidlFolder, pidlItem, bReleaseItem)
{
}

/**
 * Return SFGAOF flags for this item.
 * These flags tell the Shell about the capabilities of this item. Here we
 * can toggle the ability to copy/delete/rename an item.
 */
SFGAOF CTarFileItem::GetSFGAOF(SFGAOF dwMask)
{
   return CNseFileItem::GetSFGAOF(dwMask)
          | SFGAO_CANCOPY
          | SFGAO_CANMOVE
          | SFGAO_CANDELETE
          | SFGAO_CANRENAME
          | SFGAO_HASPROPSHEET;
}

/**
 * Return item information.
 */
HRESULT CTarFileItem::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   if( pkey == PKEY_Volume_FileSystem ) {
      return ::InitPropVariantFromString(L"TarFS", &v);
   }
   return CNseFileItem::GetProperty(pkey, v);
}

/**
 * Set item information.
 */
HRESULT CTarFileItem::SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v)
{
   // In the Property Page, the user is allowed to
   // modify the file-attributes.
   if( pkey == PKEY_FileAttributes ) {
      ULONG uFileAttribs = 0;
      HR( ::PropVariantToUInt32(v, &uFileAttribs) );
      WCHAR wszFilename[MAX_PATH] = { 0 };
      HR( _GetPathnameQuick(m_pidlFolder, m_pidlItem, wszFilename) );
      HR( tar_setfileattribs(_GetTarArchivePtr(), wszFilename, uFileAttribs) );
      // Update properties of our NSE Item
      m_pWfd->dwFileAttributes = uFileAttribs;
      return S_OK;
   }
   return CNseFileItem::SetProperty(pkey, v);
}

/**
 * Create an NSE Item instance from a child PIDL.
 */
CNseItem* CTarFileItem::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem)
{
   // Validate that the child PIDL really is ours
   if( !CPidlMemPtr<NSEFILEPIDLDATA>(pidlItem).IsType(TARFILE_MAGIC_ID) ) return NULL;
   // Spawn new NSE Item instance
   return new CTarFileItem(pFolder, pidlFolder, pidlItem, bReleaseItem);
}

/**
 * Create an NSE Item instance from static data.
 */
CNseItem* CTarFileItem::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd)
{
   NSEFILEPIDLDATA data = { sizeof(NSEFILEPIDLDATA), TARFILE_MAGIC_ID, 1, wfd };
   return new CTarFileItem(pFolder, pidlFolder, GenerateITEMID(&data, sizeof(data)), TRUE);
}

/**
 * Look up a single item by filename.
 * Validate its existance outside any cache.
 */
HRESULT CTarFileItem::GetChild(LPCWSTR pwstrName, SHGNO ParseType, CNseItem** pItem)
{
   WCHAR wszFilename[MAX_PATH] = { 0 };
   HR( _GetPathnameQuick(m_pidlFolder, m_pidlItem, wszFilename) );
   ::PathAppend(wszFilename, pwstrName);
   WIN32_FIND_DATA wfd = { 0 };
   HR( tar_getfindinfo(_GetTarArchivePtr(), wszFilename, &wfd) );
   *pItem = GenerateChild(m_pFolder, m_pFolder->m_pidlFolder, wfd);
   return *pItem != NULL ? S_OK : E_OUTOFMEMORY;
}

/**
 * Retrieve the list of children of the current folder item.
 */
HRESULT CTarFileItem::EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aItems)
{
   // Only directories have sub-items
   if( !IsFolder() ) return E_HANDLE;
   // Get actual path and retrieve a list of sub-items
   WCHAR wszPath[MAX_PATH] = { 0 };
   HR( _GetPathnameQuick(m_pidlFolder, m_pidlItem, wszPath) );
   CSimpleValArray<WIN32_FIND_DATA> aList;
   HR( tar_getfilelist(_GetTarArchivePtr(), wszPath, aList) );
   for( int i = 0; i < aList.GetSize(); i++ ) {
      // Filter item according to the 'grfFlags' argument
      if( SHFilterEnumItem(grfFlags, aList[i]) != S_OK ) continue;
      // Create an NSE Item from the file-info data
      aItems.Add( GenerateChild(m_pFolder, m_pFolder->m_pidlFolder, aList[i]) );
   }
   return S_OK;
}

/**
 * Produce a file-stream instance.
 */
HRESULT CTarFileItem::GetStream(const VFS_STREAM_REASON& Reason, CNseFileStream** ppFile)
{
   WCHAR wszFilename[MAX_PATH] = { 0 };
   HR( _GetPathnameQuick(m_pidlFolder, m_pidlItem, wszFilename) );
   *ppFile = new CTarFileStream(static_cast<CTarFileSystem*>(m_pFolder->m_spFS.m_p), wszFilename, Reason.uAccess);
   return *ppFile != NULL ? S_OK : E_OUTOFMEMORY;
}

/**
 * Create a new directory.
 */
HRESULT CTarFileItem::CreateFolder()
{
   // Create new directory in archive
   WCHAR wszFilename[MAX_PATH] = { 0 };
   HR( _GetPathnameQuick(m_pidlFolder, m_pidlItem, wszFilename) );
   HR( tar_createfolder(_GetTarArchivePtr(), wszFilename) );
   // Update properties of our NSE Item
   tar_getfindinfo(_GetTarArchivePtr(), wszFilename, m_pWfd);
   return S_OK;
}

/**
 * Rename this item.
 */
HRESULT CTarFileItem::Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName)
{
   WCHAR wszFilename[MAX_PATH] = { 0 };
   HR( _GetPathnameQuick(m_pidlFolder, m_pidlItem, wszFilename) );
   // The Shell often doesn't include the filename extension in the
   // renamed filename, so we'll append it now.
   if( wcschr(pstrOutputName, '.') == NULL ) ::PathAddExtension(pstrOutputName, ::PathFindExtension(wszFilename));
   // Rename the item in archive
   HR( tar_renamefile(_GetTarArchivePtr(), wszFilename, pstrOutputName) );
   return S_OK;
}

/**
 * Delete this item.
 */
HRESULT CTarFileItem::Delete()
{
   WCHAR wszFilename[MAX_PATH] = { 0 };
   HR( _GetPathnameQuick(m_pidlFolder, m_pidlItem, wszFilename) );
   HR( tar_deletefile(_GetTarArchivePtr(), wszFilename) );
   return S_OK;
}

/**
 * Returns the menu-items for an item.
 */
HMENU CTarFileItem::GetMenu()
{
   UINT uMenuRes = IDM_FILE;
   if( IsFolder() ) uMenuRes = IDM_FOLDER;
   return ::LoadMenu(_pModule->GetResourceInstance(), MAKEINTRESOURCE(uMenuRes));
}

/**
 * Execute a menucommand.
 */
HRESULT CTarFileItem::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   switch( Cmd.wMenuID ) {
   case ID_FILE_OPEN:        return LoadAndLaunchFile(Cmd.hWnd, m_pFolder, GetITEMID());
   case ID_FILE_EXTRACT:     return _ExtractToFolder(Cmd);
   case ID_COMMAND_EXTRACT:  return _ExtractToFolder(Cmd);
   case DFM_CMD_PASTE:       return _DoPasteFiles(Cmd);
   case DFM_CMD_NEWFOLDER:   return _DoNewFolder(Cmd, IDS_NEWFOLDER);
   case ID_FILE_NEWFOLDER:   return _DoNewFolder(Cmd, IDS_NEWFOLDER);
   case ID_FILE_PROPERTIES:  return _DoShowProperties(Cmd);
   }
   return E_NOTIMPL;
}

// Implementation

TAR_ARCHIVE* CTarFileItem::_GetTarArchivePtr() const
{
   // NOTE: There is a nasty downcast going on here. It is c++ safe since we have
   //       single inheritance only.
   return static_cast<CTarFileSystem*>(m_pFolder->m_spFS.m_p)->m_pArchive;
}

/**
 * Extract a file from tar archive to file-system.
 * This method is invoked from one of the extra menuitems or command buttons
 * we added and prompts the user to extract the selected item(s) to a particular
 * disk folder.
 */
HRESULT CTarFileItem::_ExtractToFolder(VFS_MENUCOMMAND& Cmd)
{
   // Prompt the user to choose the target path first...
   if( Cmd.pUserData == NULL ) {
      CPidl pidlFolder;
      CComBSTR bstrTitle;
      bstrTitle.LoadString(IDS_EXTRACTTITLE);
      BROWSEINFO bi = { Cmd.hWnd, NULL, NULL, bstrTitle, BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_VALIDATE, NULL, 0, 0 };
      pidlFolder.Attach( ::SHBrowseForFolder(&bi) );
      if( pidlFolder.IsNull() ) return E_ABORT;      
      LPTSTR pstrTarget = (LPTSTR) malloc(MAX_PATH * sizeof(TCHAR));
      HR( ::SHGetPathFromIDList(pidlFolder, pstrTarget) );
      Cmd.pUserData = pstrTarget;
   }
   // Extract this item to target folder.
   // Let the Windows CopyEngine do the dirty work simply by copying the
   // file/folder out of this item.
   if( Cmd.pFO == NULL ) {
      HR( ::SHCreateFileOperation(Cmd.hWnd, FOF_NOCOPYSECURITYATTRIBS | FOFX_NOSKIPJUNCTIONS | FOF_NOCONFIRMMKDIR, &Cmd.pFO) );
   }
   if( IsRoot() ) {
      // Item is the archive itself; copy its immediate children
      CNseItemArray aList;
      HR( EnumChildren(Cmd.hWnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, aList) );
      for( int i = 0; i < aList.GetSize(); i++ ) HR( aList[i]->ExecuteMenuCommand(Cmd) );
   }
   else {
      // Item is a file/folder inside the archive
      CComPtr<IShellItem> spSourceFile, spTargetFolder;
      CPidl pidlFile(m_pFolder->m_pidlRoot, m_pidlFolder, m_pidlItem);
      HR( ::SHCreateItemFromIDList(pidlFile, IID_PPV_ARGS(&spSourceFile)) );
      HR( ::SHCreateItemFromParsingName(static_cast<LPCTSTR>(Cmd.pUserData), NULL, IID_PPV_ARGS(&spTargetFolder)) );
      HR( Cmd.pFO->CopyItem(spSourceFile, spTargetFolder, m_pWfd->cFileName, NULL) );
   }
   return S_OK;
}

