
#include "stdafx.h"

#include "TarFileSystem.h"


///////////////////////////////////////////////////////////////////////////////
// CTarFileStream

CTarFileStream::CTarFileStream(CTarFileSystem* pFS, LPCWSTR pstrFilename, UINT uAccess) :
   m_uAccess(uAccess), m_pData(NULL), m_dwCurPos(0), m_dwFileSize(0), m_dwAllocSize(10L * 1024L)
{
   m_spFS = pFS;
   wcscpy_s(m_wszFilename, lengthof(m_wszFilename), pstrFilename);
}

CTarFileStream::~CTarFileStream()
{
   Close();
}

HRESULT CTarFileStream::Init()
{
   return S_OK;
}

HRESULT CTarFileStream::Seek(DWORD dwPos)
{
   if( m_uAccess == GENERIC_WRITE ) return E_ACCESSDENIED;
   if( dwPos > m_dwFileSize ) dwPos = m_dwFileSize;
   m_dwCurPos = dwPos;
   return S_OK;
}

HRESULT CTarFileStream::GetCurPos(DWORD* pdwPos)
{
   *pdwPos = m_dwCurPos;
   return S_OK;
}

HRESULT CTarFileStream::GetFileSize(DWORD* pdwFileSize)
{
   *pdwFileSize = m_dwFileSize;
   return S_OK;
}

HRESULT CTarFileStream::SetFileSize(DWORD dwSize)
{
   return S_OK;
}

HRESULT CTarFileStream::Read(LPVOID pData, ULONG dwSize, ULONG& dwBytesRead)
{
   dwBytesRead = 0;
   // We cannot mix reads and writes on same stream
   if( m_uAccess != GENERIC_READ ) return E_ACCESSDENIED;
   // Read the entire file now and keep the contents in a memory buffer
   if( m_pData == NULL ) {
      HR( tar_readfile(m_spFS->m_pArchive, m_wszFilename, &m_pData, m_dwFileSize) );
   }
   // Transfer data from memory buffer
   dwBytesRead = dwSize;
   if( m_dwCurPos + dwSize > m_dwFileSize ) dwBytesRead = m_dwFileSize - m_dwCurPos;
   ::CopyMemory(pData, m_pData + m_dwCurPos, dwBytesRead);
   m_dwCurPos += dwBytesRead;
   return dwBytesRead > 0 ? S_OK : S_FALSE;
}

HRESULT CTarFileStream::Write(LPCVOID pData, ULONG dwSize, ULONG& dwBytesWritten)
{
   dwBytesWritten = 0;
   // Cannot mix reads and writes on same stream
   if( m_uAccess != GENERIC_WRITE ) return E_ACCESSDENIED;
   // Grow the buffer?
   if( m_dwCurPos + dwSize > m_dwAllocSize || m_pData == NULL ) {
      while( m_dwCurPos + dwSize > m_dwAllocSize ) m_dwAllocSize *= 2;
      m_pData = (LPBYTE) realloc(m_pData, m_dwAllocSize);
      if( m_pData == NULL ) return E_OUTOFMEMORY;
   }
   // NOTE: We limit the filesize because we're currently using
   //       a memory buffer to hold it all. No need to deplete the
   //       system for resources.
   const DWORD MAX_FILESIZE_IN_MB = 80L;
   if( m_dwAllocSize > MAX_FILESIZE_IN_MB * 1024L * 1024L ) {
      return AtlHresultFromWin32(ERROR_FILE_TOO_LARGE);
   }
   // Transfer data to memory buffer
   ::CopyMemory(m_pData + m_dwCurPos, pData, dwSize);
   // Update file pointers
   m_dwCurPos += dwSize;
   if( m_dwCurPos > m_dwFileSize ) m_dwFileSize = m_dwCurPos;
   dwBytesWritten = dwSize;
   return dwBytesWritten > 0 ? S_OK : S_FALSE;
}

HRESULT CTarFileStream::Commit()
{
   if( m_pData == NULL ) return S_OK;
   if( m_uAccess != GENERIC_WRITE ) return E_FAIL;
   // On file creation, we write file contents to memory first, then
   // commit everything to disk on file close. This is the commit phase.
   HR( tar_writefile(m_spFS->m_pArchive, m_wszFilename, m_pData, m_dwFileSize, FILE_ATTRIBUTE_NORMAL) );
   return S_OK;
}

HRESULT CTarFileStream::Close()
{
   if( m_pData != NULL ) free(m_pData);
   m_pData = NULL;
   return S_OK;
}

