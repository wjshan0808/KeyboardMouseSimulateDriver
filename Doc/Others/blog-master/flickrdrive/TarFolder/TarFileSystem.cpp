
#include "stdafx.h"

#include "TarFileSystem.h"


///////////////////////////////////////////////////////////////////////////////
// CTarShellModule

/**
 * Return capabilities of this Shell Extension.
 * The GetConfigXXX methods are called by the NSE to query what functionality our
 * file-system supports.
 */
BOOL CTarShellModule::GetConfigBool(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_INSTALL_PREVIEW:
   case VFS_INSTALL_SHELLNEW:
   case VFS_INSTALL_PROPSHEET:
   case VFS_INSTALL_DROPTARGET:
   case VFS_INSTALL_CONTEXTMENU:
      return TRUE;

   case VFS_CAN_PROGRESSUI:
   case VFS_CAN_ATTACHMENTSERVICES:
      return TRUE;

   case VFS_HAVE_SYSICONS:
   case VFS_HAVE_UNIQUE_NAMES:
   case VFS_HAVE_VIRTUAL_FILES:
      return TRUE;

   case VFS_HIDE_PREVIEW_PANEL:
      return TRUE;

   }
   return FALSE;
}

/**
 * Return capabilities of this Shell Extension.
 * The GetConfigXXX methods are called by the NSE to query what functionality our
 * file-system supports.
 */
LONG CTarShellModule::GetConfigInt(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_INT_LOCATION:
      return VFS_LOCATION_JUNCTION;

   case VFS_INT_MAX_FILENAME_LENGTH:
      return TAR_MAXNAMELEN;

   case VFS_INT_MAX_PATHNAME_LENGTH:
      return TAR_MAXPATHLEN;

   case VFS_INT_SHELLROOT_SFGAO:
       return SFGAO_CANCOPY 
              | SFGAO_CANMOVE 
              | SFGAO_CANRENAME 
              | SFGAO_DROPTARGET
              | SFGAO_STREAM
              | SFGAO_FOLDER 
              | SFGAO_BROWSABLE 
              | SFGAO_HASSUBFOLDER 
              | SFGAO_HASPROPSHEET 
              | SFGAO_FILESYSANCESTOR
              | SFGAO_STORAGEANCESTOR;

   }
   return 0;
}

/**
 * Return capabilities of this Shell Extension.
 * The GetConfigXXX methods are called by the NSE to query what functionality our
 * file-system supports.
 */
LPCWSTR CTarShellModule::GetConfigStr(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_STR_FILENAME_CHARS_NOTALLOWED:
      return L":<>\\/|\"'*?[]";

   }
   return NULL;
}

/**
 * Called during installation (dll registration)
 * Use this to do extra work during the module registration. Remember that this
 * method is run as Admin and only called during installation.
 */
HRESULT CTarShellModule::DllInstall()
{
   return S_OK;
}

/**
 * Called during uninstall (dll de-registration)
 */
HRESULT CTarShellModule::DllUninstall()
{
   return S_OK;
}

/**
 * Called on ShellNew integration.
 * Invoked by user through Desktop ContextMenu -> New -> Tar Folder.
 */
HRESULT CTarShellModule::ShellAction(LPCWSTR pstrType, LPCWSTR pstrCmdLine)
{
   if( wcscmp(pstrType, L"ShellNew") == 0 ) return tar_createarchive(pstrCmdLine);
   return S_OK;
}

/**
 * Called at process/thread startup/shutdown.
 */
BOOL CTarShellModule::DllMain(DWORD dwReason, LPVOID lpReserved)
{
   return TRUE;
}

/**
 * Create a FileSystem instance from a IDLIST (Windows file).
 */
HRESULT CTarShellModule::CreateFileSystem(PCIDLIST_ABSOLUTE pidlRoot, CNseFileSystem** ppFS)
{
   // Create a new file-system instance with the source .tar filename.
   CTarFileSystem* pFS = new CTarFileSystem();
   HRESULT Hr = pFS->Init(pidlRoot);
   if( SUCCEEDED(Hr) ) *ppFS = pFS;
   else delete pFS; 
   return Hr;
}


///////////////////////////////////////////////////////////////////////////////
// CTarFileSystem

CTarFileSystem::CTarFileSystem() : m_cRef(1), m_pArchive(NULL)
{
}

CTarFileSystem::~CTarFileSystem()
{
   tar_closearchive(m_pArchive);
}

HRESULT CTarFileSystem::Init(PCIDLIST_ABSOLUTE pidlRoot)
{
   // Get the filename of the Windows source file.
   WCHAR wszTarFilename[MAX_PATH] = { 0 };
   if( !::SHGetPathFromIDListW(pidlRoot, wszTarFilename) ) return E_FAIL;
   // Initialize the .tar archive; this operation doesn't actually touch
   // the file because a file-system is spawned for many operations
   // that doesn't need the physical files.
   HR( tar_openarchive(wszTarFilename, &m_pArchive) );
   return S_OK;
}

VOID CTarFileSystem::AddRef()
{
   ::InterlockedIncrement(&m_cRef);
}

VOID CTarFileSystem::Release()
{
   if( ::InterlockedDecrement(&m_cRef) == 0 ) delete this;
}

/**
 * Create the root NSE Item instance.
 */
CNseItem* CTarFileSystem::GenerateRoot(CShellFolder* pFolder)
{
   return new CTarFileItem(pFolder, NULL, NULL, FALSE);
}

