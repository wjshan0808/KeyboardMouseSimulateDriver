
#include "stdafx.h"

#include "LaunchFile.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// Launch Thread

typedef struct tagLAUNCHDATA
{
   HWND hWnd;
   WCHAR szVerb[20];
   WCHAR szDestPath[MAX_PATH];
   WCHAR szDestFile[MAX_PATH];
   WCHAR szNewName[MAX_PATH];
   CPidl pidlPath;
   CPidl pidlFile;
} LAUNCHDATA;


DWORD WINAPI LaunchThread(LPVOID pData)
{
   LAUNCHDATA data = * (LAUNCHDATA*) pData;
   delete (LAUNCHDATA*) pData;

   ATLTRACE(L"LaunchThread  filename='%s'\n", data.szDestFile);

   bool bLaunch = true;

   // Copy file from virtual drive to disk...
   if( bLaunch && !IsFileTimeValid(GetFileModifiedTime(data.szDestFile)) )
   {
      CComPtr<IFileOperation> spFO;
      HRESULT Hr = ::SHCreateFileOperation(data.hWnd, FOF_NOCOPYSECURITYATTRIBS | FOFX_NOSKIPJUNCTIONS | FOF_NOCONFIRMATION | FOFX_DONTDISPLAYDESTPATH, &spFO);
      if( FAILED(Hr) ) return 0;
      CComPtr<IShellItem> spSourceFile;
      CComPtr<IShellItem> spTargetFile;
      ::SHCreateItemFromIDList(data.pidlFile, IID_PPV_ARGS(&spSourceFile));
      ::SHCreateItemFromParsingName(data.szDestPath, NULL, IID_PPV_ARGS(&spTargetFile));
      spFO->CopyItem(spSourceFile, spTargetFile, data.szNewName, NULL);
      Hr = spFO->PerformOperations();
      if( FAILED(Hr) ) bLaunch = false;
   }

   // Check security first. Use Windows AttachmentServices to determine
   // and display warning prompt.
   if( bLaunch && _ShellModule.GetConfigBool(VFS_CAN_ATTACHMENTSERVICES) )
   {
      LPCTSTR pstrExt = ::PathFindExtension(data.szNewName);
      BOOL bIsDangerous = ::AssocIsDangerous(pstrExt);
      if( bIsDangerous ) {
         CComPtr<IAttachmentExecute> spExecute;
         spExecute.CoCreateInstance(CLSID_AttachmentServices);
         if( spExecute != NULL ) {
            CComBSTR bstrTitle, bstrVendor;
            bstrVendor.LoadString(IDS_NSE_VENDOR);
            bstrTitle.LoadString(IDS_NSE_DISPLAYNAME);
            spExecute->SetSource(bstrVendor);
            spExecute->SetClientTitle(bstrTitle);
            spExecute->SetClientGuid(CLSID_ShellFolder);
            if( SUCCEEDED( spExecute->SetLocalPath(data.szDestFile) ) ) {               
               ATTACHMENT_ACTION ac = ATTACHMENT_ACTION_CANCEL;
               spExecute->Prompt(data.hWnd, ATTACHMENT_PROMPT_EXEC, &ac);
               bLaunch = (ac == ATTACHMENT_ACTION_EXEC);
            }
         }
      }
   }

   // Launch the file now?
   if( bLaunch )
   {
      FILETIME ftWrite = GetFileModifiedTime(data.szDestFile);
      DWORD dwTick = ::GetTickCount();

      SHELLEXECUTEINFO sei = { 0 };
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT; 
      sei.hwnd = data.hWnd;
      sei.lpVerb = data.szVerb; 
      sei.lpFile = data.szDestFile;
      sei.nShow = SW_SHOWNORMAL;
      if( ::ShellExecuteEx(&sei) ) {
         // FIX: Post empty message to parent to speed up destruction of Progress dialog
         if( data.hWnd != NULL && ::IsWindow(data.hWnd) ) ::PostMessage(data.hWnd, WM_NULL, 0, 0L);
         // Wait for the launched process to die
         if( ::WaitForInputIdle(sei.hProcess, 3000L) == WAIT_FAILED ) ::Sleep(5000L);
         DWORD dwCode = 0;
         while( ::GetExitCodeProcess(sei.hProcess, &dwCode) && dwCode == STILL_ACTIVE ) {
            ::Sleep(1000L);
         }
         ::CloseHandle(sei.hProcess);
      }

      // Let a quick-dying launch process linger a bit...
      if( ::GetTickCount() - dwTick < 4000UL ) ::Sleep(3000UL);

      // Write changes to file back to virtual file-system...
      FILETIME ftAfter = GetFileModifiedTime(data.szDestFile);
      if( IsFileTimeValid(ftAfter) && ::CompareFileTime(&ftAfter, &ftWrite) > 0 ) {
         ATLTRACE(L"LaunchThread - Saving modified file\n");
         CComPtr<IFileOperation> spFO;
         HRESULT Hr = ::SHCreateFileOperation(data.hWnd, FOF_NOCOPYSECURITYATTRIBS | FOFX_NOSKIPJUNCTIONS | FOF_NOCONFIRMATION | FOFX_DONTDISPLAYSOURCEPATH, &spFO);
         if( SUCCEEDED(Hr) ) {
            CComPtr<IShellItem> spSourceFile;
            CComPtr<IShellItem> spTargetFile;
            ::SHCreateItemFromParsingName(data.szDestFile, NULL, IID_PPV_ARGS(&spSourceFile));
            ::SHCreateItemFromIDList(data.pidlPath, IID_PPV_ARGS(&spTargetFile));
            spFO->CopyItem(spSourceFile, spTargetFile, data.szNewName, NULL);
            spFO->PerformOperations();
         }
      }
   }

   // Attempt to delete the file again; we should try really hard.
   BOOL bRes = ::DeleteFile(data.szDestFile);
   if( !bRes ) ::MoveFileEx(data.szDestFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT); 

   ATLTRACE(L"LaunchThread - Done\n");

   return 0;
};


///////////////////////////////////////////////////////////////////////////////
// Launch File

HRESULT LoadAndLaunchFile(HWND hWnd, CShellFolder* pFolder, PCUITEMID_CHILD pidlItem, LPCTSTR pstrVerb)
{
   CNseItemPtr spItem = pFolder->GenerateChildItem(pidlItem);
   if( spItem == NULL ) return E_FAIL;
   const WIN32_FIND_DATA wfd = spItem->GetFindData();

   // Generate a temporary filename that we should save the file to.
   // We'll then copy it to disk and launch this local copy.
   TCHAR szPath[MAX_PATH] = { 0 };
   TCHAR szFilename[MAX_PATH] = { 0 };
   ::GetTempPath(MAX_PATH, szPath);
   ::PathAppend(szPath, _T("vfolder"));
   ::SHCreateDirectory(hWnd, szPath);
   _tcscpy_s(szFilename, lengthof(szFilename), szPath);
   ::PathAppend(szFilename, wfd.cFileName);

   LAUNCHDATA* pData = new LAUNCHDATA;
   pData->hWnd = hWnd;
   _tcscpy_s(pData->szVerb, lengthof(pData->szVerb), pstrVerb);
   _tcscpy_s(pData->szDestPath, lengthof(pData->szDestPath), szPath);
   _tcscpy_s(pData->szDestFile, lengthof(pData->szDestFile), szFilename);
   _tcscpy_s(pData->szNewName, lengthof(pData->szNewName), wfd.cFileName);
   pData->pidlFile = pFolder->m_pidlMonitor + pidlItem;
   pData->pidlPath = pFolder->m_pidlMonitor;
   if( !::SHCreateThread(LaunchThread, pData, CTF_COINIT | CTF_PROCESS_REF, NULL) ) return E_FAIL;

   return S_OK;
}

