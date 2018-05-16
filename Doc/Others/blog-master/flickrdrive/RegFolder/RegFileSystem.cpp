
#include "StdAfx.h"

#include "RegFileSystem.h"


///////////////////////////////////////////////////////////////////////////////
// CRegShellModule

/**
 * Return capabilities of this Shell Extension.
 * The GetConfigXXX methods are called by the NSE to query what functionality our
 * file-system supports.
 */
BOOL CRegShellModule::GetConfigBool(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_INSTALL_PROPERTIES:
      return TRUE;

   case VFS_SHOW_DETAILS_PANEL:
      return TRUE;

   case VFS_HAVE_IDENTITY:
	   return TRUE;

   case VFS_HAVE_SYSICONS:
   case VFS_HAVE_UNIQUE_NAMES:
   case VFS_HAVE_VIRTUAL_FILES:
	   return TRUE;

   }
   return FALSE;
}

/**
 * Return capabilities of this Shell Extension.
 * The GetConfigXXX methods are called by the NSE to query what functionality our
 * file-system supports.
 */
LONG CRegShellModule::GetConfigInt(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_INT_LOCATION:
      return VFS_LOCATION_MYCOMPUTER;

   case VFS_INT_MAX_FILENAME_LENGTH:
      return MAX_REGPATH;

   case VFS_INT_MAX_PATHNAME_LENGTH:
      return MAX_REGPATH;

   case VFS_INT_SHELLROOT_SFGAO:
	   return SFGAO_CANCOPY
		   | SFGAO_CANMOVE
		   | SFGAO_CANRENAME
		   | SFGAO_DROPTARGET
		   | SFGAO_FOLDER
		   | SFGAO_BROWSABLE
		   | SFGAO_HASSUBFOLDER
		   | SFGAO_HASPROPSHEET
		   | SFGAO_FILESYSANCESTOR;

   }
   return 0;
}

/**
 * Return capabilities of this Shell Extension.
 * The GetConfigXXX methods are called by the NSE to query what functionality our
 * file-system supports.
 */
LPCWSTR CRegShellModule::GetConfigStr(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_STR_FILENAME_CHARS_NOTALLOWED:
      return L"\\";

   }
   return NULL;
}

/**
 * Called during installation (dll registration)
 * Use this to do extra work during the module registration. Recall that this
 * method is run as Admin and only called during installation.
 */
HRESULT CRegShellModule::DllInstall()
{
   return S_OK;
}

/**
 * Called during uninstall (dll de-registration)
 */
HRESULT CRegShellModule::DllUninstall()
{
   return S_OK;
}

/**
 * Called on ShellNew integration.
 */
HRESULT CRegShellModule::ShellAction(LPCWSTR pstrType, LPCWSTR pstrCmdLine)
{
   return S_OK;
}

/**
 * Called at process/thread startup/shutdown.
 */
BOOL CRegShellModule::DllMain(DWORD dwReason, LPVOID lpReserved)
{
   return TRUE;
}

/**
 * Create a FileSystem instance.
 */
HRESULT CRegShellModule::CreateFileSystem(PCIDLIST_ABSOLUTE pidlRoot, CNseFileSystem** ppFS)
{
   *ppFS = new CRegFileSystem();
   return *ppFS != NULL ? S_OK : E_OUTOFMEMORY;
}


///////////////////////////////////////////////////////////////////////////////
// CRegFileSystem

CRegFileSystem::CRegFileSystem() : m_cRef(1)
{
}

VOID CRegFileSystem::AddRef()
{
   ::InterlockedIncrement(&m_cRef);
}

VOID CRegFileSystem::Release()
{
   if( ::InterlockedDecrement(&m_cRef) == 0 ) delete this;
}

/**
 * Create the root NSE Item instance.
 */
CNseItem* CRegFileSystem::GenerateRoot(CShellFolder* pFolder)
{
   return new CRegItemKey(pFolder, NULL, NULL, FALSE);
}

