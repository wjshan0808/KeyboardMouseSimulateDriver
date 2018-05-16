
#include "stdafx.h"

#include "FileStream.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CFileStream

CFileStream::CFileStream() : m_pStream(NULL), m_bNeedCommit(false)
{
   ATLTRACE(L"CFileStream::CFileStream\n");
}

void CFileStream::FinalRelease()
{
   ATLTRACE(L"CFileStream::FinalRelease\n");
   if( m_pStream != NULL && m_bNeedCommit ) m_pStream->Commit();
   delete m_pStream;
}

HRESULT CFileStream::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidlItem)
{
   m_spFolder = pFolder;
   m_pidlItem = pidlItem;
   return S_OK;
}

// IStream

STDMETHODIMP CFileStream::Read(LPVOID pv, ULONG cb, ULONG* pcbRead)
{
   ATLTRACE(L"CFileStream::Read  cb=%lu\n", cb);
   if( pcbRead != NULL ) *pcbRead = 0;
   HR( _OpenFileNow(GENERIC_READ) );
   ULONG dwDummy = 0;
   ULONG& dwBytesRead = pcbRead == NULL ? dwDummy : *pcbRead;
   return m_pStream->Read(pv, cb, dwBytesRead);
}

STDMETHODIMP CFileStream::Write(LPCVOID pv, ULONG cb, ULONG* pcbWritten)
{
   ATLTRACE(L"CFileStream::Write  cb=%lu\n", cb);
   if( pcbWritten != NULL ) *pcbWritten = 0;
   HR( _OpenFileNow(GENERIC_WRITE) );
   ULONG dwDummy = 0;
   ULONG& dwBytesWritten = pcbWritten == NULL ? dwDummy : *pcbWritten;
   HRESULT Hr = m_pStream->Write(pv, cb, dwBytesWritten);
   if( SUCCEEDED(Hr) ) m_bNeedCommit = true;
   return Hr;   
}

STDMETHODIMP CFileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
{
   ATLTRACE(L"CFileStream::Seek  origin=%lu pos=%lu\n", dwOrigin, dlibMove.LowPart);   
   HR( _OpenFileNow(GENERIC_READ) );
   DWORD dwCurPos = 0, dwFileSize = 0;
   m_pStream->GetCurPos(&dwCurPos);
   m_pStream->GetFileSize(&dwFileSize);
   HRESULT Hr = S_OK;
   switch( dwOrigin ) {
   case STREAM_SEEK_SET:  Hr = m_pStream->Seek(dlibMove.LowPart); break;
   case STREAM_SEEK_CUR:  Hr = m_pStream->Seek(dwCurPos + dlibMove.LowPart); break;
   case STREAM_SEEK_END:  Hr = m_pStream->Seek(dwFileSize - dlibMove.LowPart); break;
   default: return E_INVALIDARG;
   }
   if( plibNewPosition != NULL ) {
      plibNewPosition->QuadPart = 0;
      m_pStream->GetCurPos(&plibNewPosition->LowPart);
   }
   return Hr;
}

STDMETHODIMP CFileStream::SetSize(ULARGE_INTEGER libNewSize)
{
   ATLTRACE(L"CFileStream::SetSize  size=%lu\n", libNewSize.LowPart); 
   HR( _OpenFileNow(GENERIC_WRITE) );
   return m_pStream->SetFileSize(libNewSize.LowPart);
}

STDMETHODIMP CFileStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
{
   ATLTRACE(L"CFileStream::CopyTo\n");
   HR( _OpenFileNow(GENERIC_READ) );
   ULARGE_INTEGER size = { 0 };
   if( pcbRead != NULL ) pcbRead->QuadPart = 0;
   if( pcbWritten != NULL ) pcbWritten->QuadPart = 0;
   const ULONG MAX_BUFFER = 8000;
   CAutoVectorPtr<BYTE> buffer( new BYTE[MAX_BUFFER] );
   if( buffer == NULL ) return E_OUTOFMEMORY;
   while( cb.QuadPart > 0 ) {
      ULONG dwRead = 0, dwWritten = 0;
      HR( m_pStream->Read(buffer, MAX_BUFFER, dwRead) );
      if( dwRead == 0 ) break; // No more to read! Stop!
      HR( pstm->Write(buffer, dwRead, &dwWritten) );
      ATLASSERT(dwRead==dwWritten);
      if( dwRead != dwWritten ) return E_FAIL;
      cb.QuadPart -= dwWritten;
      size.QuadPart += dwWritten;
   }
   if( pcbRead != NULL ) pcbRead->QuadPart = size.QuadPart;
   if( pcbWritten != NULL ) pcbWritten->QuadPart = size.QuadPart;
   return S_OK;
}

STDMETHODIMP CFileStream::Commit(DWORD grfCommitFlags)
{
   ATLTRACE(L"CFileStream::Commit  flags=0x%X\n", grfCommitFlags); grfCommitFlags;
   if( m_pStream == NULL ) return S_OK;
   if( !m_bNeedCommit ) return S_OK;
   HR( m_pStream->Commit() );
   m_bNeedCommit = false;
   return S_OK;
}

STDMETHODIMP CFileStream::Revert(void)
{
   m_bNeedCommit = false;
   ATLTRACENOTIMPL(L"CFileStream::Revert");
}

STDMETHODIMP CFileStream::LockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/, DWORD /*dwLockType*/)
{
   ATLTRACENOTIMPL(L"CFileStream::LockRegion");
}

STDMETHODIMP CFileStream::UnlockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/, DWORD /*dwLockType*/)
{
   ATLTRACENOTIMPL(L"CFileStream::UnlockRegion");
}

STDMETHODIMP CFileStream::Stat(STATSTG* pstatstg, DWORD grfStatFlag)
{
   ATLTRACE(L"CFileStream::Stat  flags=0x%X\n", grfStatFlag);
   CNseItemPtr spItem = m_spFolder->GenerateChildItem(m_pidlItem.GetItem(0));
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   const WIN32_FIND_DATA wfd = spItem->GetFindData();
   pstatstg->pwcsName = NULL;
   if( !IsBitSet(grfStatFlag, STATFLAG_NONAME) ) ::SHStrDup(wfd.cFileName, &pstatstg->pwcsName);
   pstatstg->cbSize.LowPart = wfd.nFileSizeLow;
   pstatstg->cbSize.HighPart = wfd.nFileSizeHigh;
   // Some virtual files may not know their size before they are actually
   // opened. So we might as well attempt to access the file now.
   if( pstatstg->cbSize.QuadPart == 0 && !IsBitSet(grfStatFlag, STATFLAG_NOOPEN) ) {
      if( SUCCEEDED(_OpenFileNow(GENERIC_READ)) ) m_pStream->GetFileSize(&pstatstg->cbSize.LowPart);
   }
   pstatstg->ctime = wfd.ftCreationTime;
   pstatstg->mtime = wfd.ftLastWriteTime;
   pstatstg->atime = wfd.ftLastAccessTime;
   pstatstg->type = STGTY_STREAM;
   pstatstg->grfLocksSupported = 0;
   pstatstg->grfMode = STGM_READWRITE;
   pstatstg->clsid = CLSID_NULL;
   pstatstg->grfStateBits = 0;
   pstatstg->reserved = 0;
   return S_OK;
}

STDMETHODIMP CFileStream::Clone(IStream** /*ppstm*/)
{
   ATLTRACENOTIMPL(L"CFileStream::Clone");
}

// Implementation

HRESULT CFileStream::_OpenFileNow(UINT uAccess)
{
   // File already opened?
   if( m_pStream != NULL ) return S_OK;
   // Open the file; generate a file-item, then grab the file-stream...
   CNseItemPtr spItem = m_spFolder->GenerateChildItem(m_pidlItem.GetItem(0));
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   VFS_STREAM_REASON Reason = { VFS_STREAMTYPE_FILE, uAccess, 0 };
   HR( spItem->GetStream(Reason, &m_pStream) );
   HR( m_pStream->Init() );
   return S_OK;
}

