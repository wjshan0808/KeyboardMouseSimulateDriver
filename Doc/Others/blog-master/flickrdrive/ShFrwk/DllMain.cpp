
#include "stdafx.h"

#include "NseFileSystem.h"


//////////////////////////////////////////////////////////////////////
//

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
   switch( dwReason ) {
   case DLL_PROCESS_ATTACH:
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
      if( !_AtlModule.DllMain(dwReason, lpReserved) ) return FALSE;
      if( !_ShellModule.DllMain(dwReason, lpReserved) ) return FALSE;
      return TRUE;
   }
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// REGSVR32 support

STDAPI DllCanUnloadNow()
{
   return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
   return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer()
{
   ATLTRACE(L"DllRegisterServer\n");
   HR( _AtlModule.DllRegisterServer(FALSE) );
   HR( _ShellModule.DllInstall() );
   return S_OK;
}

STDAPI DllUnregisterServer()
{
   ATLTRACE(L"DllUnregisterServer\n");
   HR( _ShellModule.DllUninstall() );
   HR( _AtlModule.DllUnregisterServer(FALSE) );
   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// RUNDLL32 support

void CALLBACK ShellNew(HWND hwndStub, HINSTANCE hInstance, LPSTR pszCmdLine, int nCmdShow)
{
   ATLTRACE(L"ShellNew\n");
   // Called with VFS_INSTALL_SHELLNEW integration in place
   CComBSTR bstrCmdline = pszCmdLine;
   _ShellModule.ShellAction(L"ShellNew", bstrCmdline);
}
