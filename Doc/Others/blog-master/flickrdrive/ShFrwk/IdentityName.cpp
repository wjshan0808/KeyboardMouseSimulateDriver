
#include "stdafx.h"

#include "IdentityName.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// SaveAs Monitor Thread

typedef struct tagMONITORDATA
{
   WCHAR szTempFilename[MAX_PATH];
   WCHAR szNewName[MAX_PATH];
   CPidl pidlTargetPath;
   HANDLE hStartEvent;
} MONITORDATA;


static DWORD WINAPI MonitorThread(LPVOID pData)
{
   MONITORDATA data = * (MONITORDATA*) pData;
   delete (MONITORDATA*) pData;

   FILETIME ftWrite = GetFileModifiedTime(data.szTempFilename);

   WCHAR szPath[MAX_PATH] = { 0 };
   wcscpy_s(szPath, lengthof(szPath), data.szTempFilename);
   ::PathRemoveFileSpec(szPath);

   // We should make sure this file is eventually deleted!
   // We are aware that this thread runs inside the context of the remote process
   // and that it terminates without ever warning us. Thus the shutdown/cleanup 
   // procedure of this thread is a bit uncertain.
   ::MoveFileEx(data.szTempFilename, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

   // Get a handle to a file change notification object
   HANDLE hChange = ::FindFirstChangeNotification(szPath, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);   
   
   // Signal to owner that we're started
   ::SetEvent(data.hStartEvent);
   
   // Monitor file for changes...
   while( hChange != INVALID_HANDLE_VALUE )
   {
      if( ::WaitForSingleObject(hChange, INFINITE) != WAIT_OBJECT_0 ) break;
      ::Sleep(1000L);
      // Get rid of similar events...
      DWORD dwRes;
      do {
         ::FindNextChangeNotification(hChange);
         dwRes = ::WaitForSingleObject(hChange, 100UL);
      } while( dwRes == WAIT_OBJECT_0 );

      // Test the file timestamp and update the file if needed
      FILETIME ftAfter = GetFileModifiedTime(data.szTempFilename);
      if( ::CompareFileTime(&ftAfter, &ftWrite) > 0 ) {
         // Write file back to the virtual folder.
         // BUG: If the process terminates at this point we will be left
         //      with a corrupted file. A real solution could be to notify a
         //      remote process under our control to take care of the
         //      file operation instead.
         CComPtr<IFileOperation> spFO;
         HRESULT Hr = ::SHCreateFileOperation(NULL, FOF_NOCOPYSECURITYATTRIBS | FOF_NOCONFIRMATION | FOFX_NOSKIPJUNCTIONS, &spFO);
         if( FAILED(Hr) ) break;
         CComPtr<IShellItem> spSourceFile, spTargetFile;
         ::SHCreateItemFromParsingName(data.szTempFilename, NULL, IID_PPV_ARGS(&spSourceFile));
         ::SHCreateItemFromIDList(data.pidlTargetPath, IID_PPV_ARGS(&spTargetFile));
         spFO->CopyItem(spSourceFile, spTargetFile, data.szNewName, NULL);
         spFO->PerformOperations();
         // Remember the new file-time of the temporary file
         ftWrite = GetFileModifiedTime(data.szTempFilename);
      }
      ::FindNextChangeNotification(hChange);
   }
   ::FindCloseChangeNotification(hChange);

   ::DeleteFile(data.szTempFilename);

   return 0;
};


///////////////////////////////////////////////////////////////////////////////
// CIdentityName

HRESULT CIdentityName::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl)
{
   // Get the NSE Item of the virtual file
   CNseItemPtr spItem = pFolder->GenerateChildItem(pidl);
   if( spItem == NULL ) return E_UNEXPECTED;

   // We can support an alias identity for all items, or we may just wish
   // to make use of the SaveAs dialog hack.
   if( _ShellModule.GetConfigBool(VFS_HAVE_IDENTITY) )
   {
      HR( spItem->GetIdentity(m_szFilename) );
      wcscpy_s(pFolder->m_spFS->m_wszOpenSaveAsFilename, lengthof(pFolder->m_spFS->m_wszOpenSaveAsFilename), m_szFilename);
      return S_OK;
   }
   else if( spItem->GetFindData().cAlternateFileName[1] == VFS_HACK_SAVEAS_JUNCTION ) 
   {
      // Generate a temporary filename that we should save the file
      // to. This is where the process calling the SaveAs dialog will save
      // its data to. We shall pick up the new contents when it closes the file.
      TCHAR szPath[MAX_PATH] = { 0 };
      ::GetTempPath(MAX_PATH, szPath);
      ::PathAppend(szPath, _T("vfolder"));
      ::SHCreateDirectory(NULL, szPath);
      ::GetTempFileName(szPath, _T("vfs"), 0, m_szFilename);
      ::DeleteFile(m_szFilename);

      // Store the temporary filename in case we get queried on the ShellFolder.
      // The thing is, our files live in virtual space and *only* when we specifially see the
      // need to transfer them to disk, we will do so.
      wcscpy_s(pFolder->m_spFS->m_wszOpenSaveAsFilename, lengthof(pFolder->m_spFS->m_wszOpenSaveAsFilename), m_szFilename);

      // Thread start event...
      CHandle hEvent( ::CreateEvent(NULL, FALSE, FALSE, NULL) );

      // Launch a thread that will monitor file changes.
      // Whenever the file is completely saved, we copy it back to the virtual folder.
      MONITORDATA* pData = new MONITORDATA;
      pData->hStartEvent = hEvent;
      pData->pidlTargetPath = CPidl(pFolder->m_pidlRoot, pFolder->m_pidlFolder);
      wcscpy_s(pData->szNewName, lengthof(pData->szNewName), ::PathFindFileName(m_szFilename));
      wcscpy_s(pData->szTempFilename, lengthof(pData->szTempFilename), m_szFilename);
      if( !::SHCreateThread(MonitorThread, pData, CTF_COINIT, NULL) ) return E_FAIL;

      // Wait for thread to start and begin to monitor path...
      ::WaitForSingleObject(hEvent, 200);
      return S_OK;
   }

   return E_NOTIMPL;
}

// IIdentityName

STDMETHODIMP CIdentityName::GetItemIDList(PIDLIST_ABSOLUTE* ppidl)
{
   ATLTRACE(L"CIdentityName::GetItemIDList\n");
   return ::SHParseDisplayName(m_szFilename, NULL, ppidl, 0, NULL);
}

STDMETHODIMP CIdentityName::GetItem(IShellItem** ppsi)
{
   ATLTRACE(L"CIdentityName::GetItem\n");
   CComPtr<IBindCtx> spBind;
   HR( ::CreateBindCtx(0, &spBind) );
   BIND_OPTS Opts = { sizeof(Opts), 0, STGM_CREATE, 0 };
   HR( spBind->SetBindOptions(&Opts) );
   return ::SHCreateItemFromParsingName(m_szFilename, spBind, IID_PPV_ARGS(ppsi));
}

