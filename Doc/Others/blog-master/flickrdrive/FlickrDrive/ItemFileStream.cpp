
#include "stdafx.h"

#include "FileSystem.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemFileStream

CFlickrItemFileStream::CFlickrItemFileStream(const VFS_STREAM_REASON& Reason, CShellFolder* pFolder, TFlickrPhoto* pPhoto, TFlickrImageset* pImageset) :
   m_sPhotoID(pPhoto->sPhotoID), 
   m_pImageset(pImageset),
   m_spFolder(pFolder),
   m_hConnect(NULL), 
   m_hRequest(NULL),
   m_Reason(Reason),
   m_dwFileSize(0),
   m_dwCurPos(0)
{
}

CFlickrItemFileStream::~CFlickrItemFileStream()
{
   Close();
}

HRESULT CFlickrItemFileStream::Init()
{
   ATLTRACE(L"CFlickrItemFileStream::Init\n");
   HR( _ShellModule.Rest.InitHttpService() );
   if( m_Reason.uAccess == GENERIC_READ )
   {
      CString sURI;
      HR( _ShellModule.Rest.GetPhotoThumbUrl(m_sPhotoID, m_Reason.Type, m_Reason.cxyThumb, sURI) );
      CString sHost = sURI.Left(sURI.Find('/'));
      CString sURL = sURI.Mid(sURI.Find('/'));
      m_hConnect = ::WinHttpConnect(_ShellModule.Rest.m_hInternet, sHost, INTERNET_DEFAULT_HTTPS_PORT, 0);
      if( m_hConnect == NULL ) return AtlHresultFromLastError();
      m_hRequest = ::WinHttpOpenRequest(m_hConnect, _T("GET"), sURL, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
      if( m_hRequest == NULL ) return AtlHresultFromLastError();
      BOOL bResult = ::WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
      if( !bResult ) return AtlHresultFromLastError();
      bResult = ::WinHttpReceiveResponse(m_hRequest, NULL);
      if( !bResult ) return AtlHresultFromLastError();
      // Retrieve content-length and assume it to be the file-size...
      DWORD cbHeaderLength = sizeof(m_dwFileSize);
      DWORD dwHeaderIndex = WINHTTP_NO_HEADER_INDEX;
      ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &m_dwFileSize, &cbHeaderLength, &dwHeaderIndex);
   }
   else
   {
      TFlickrAccount* pAccount = NULL;
      HR( _ShellModule.Rest.GetAccountFromPrimary(&pAccount) );
      if( m_pImageset != NULL ) pAccount = m_pImageset->pAccount;
      FLICKR_APIARG ApiUpload[] =
      {
         { "api_key",      FLICKR_API_KEY },
         { "auth_token",   pAccount->sAuthToken },
      };
      int nArgs = lengthof(ApiUpload);
      CString sURI, sTemp, sSignature;
      HR( _ShellModule.Rest.GetPhotoUploadUrl(pAccount, ApiUpload, nArgs, sURI, sSignature) );
      CString sHost = sURI.Left(sURI.Find('/'));
      CString sURL = sURI.Mid(sURI.Find('/'));
      m_hConnect = ::WinHttpConnect(_ShellModule.Rest.m_hInternet, sHost, INTERNET_DEFAULT_HTTPS_PORT, 0);
      if( m_hConnect == NULL ) return AtlHresultFromLastError();
      m_hRequest = ::WinHttpOpenRequest(m_hConnect, _T("POST"), sURL, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
      if( m_hRequest == NULL ) return AtlHresultFromLastError();
      ::WinHttpAddRequestHeaders(m_hRequest, _T("Cache-Control: no-cache"), (DWORD) -1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
      ::WinHttpAddRequestHeaders(m_hRequest, _T("Content-Type: multipart/form-data; boundary=--------------------------7d137f1480712"), (DWORD) -1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
      ::WinHttpAddRequestHeaders(m_hRequest, _T("Accept-Language: en-us,en"), (DWORD) -1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
      // Create Form post header
      for( int i = 0; i < nArgs; i++ ) {
         sTemp.Format(_T("----------------------------7d137f1480712\r\nContent-Disposition: form-data; name=\"%hs\"\r\n\r\n%s\r\n"), ApiUpload[i].pstrName, ApiUpload[i].pstrValue);
         m_sPostHeader += sTemp;
      }
      sTemp.Format(_T("----------------------------7d137f1480712\r\nContent-Disposition: form-data; name=\"api_sig\"\r\n\r\n%s\r\n"), sSignature);
      m_sPostHeader += sTemp;
      // Create Form file header
      CRegKey reg;
      CString sContentType = _T("application/octet-stream");
      if( reg.Open(HKEY_CLASSES_ROOT, ::PathFindExtension(m_sPhotoID), KEY_READ) == ERROR_SUCCESS ) {
         TCHAR szBuffer[100] = { 0 };
         ULONG cbBuffer = lengthof(szBuffer);
         reg.QueryStringValue(_T("Content Type"), szBuffer, &cbBuffer);
         if( szBuffer[0] != '\0' ) sContentType = szBuffer;
      }
      sTemp.Format(_T("----------------------------7d137f1480712\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"%s\"\r\nContent-Transfer-Encoding: binary\r\nContent-Type: %s\r\n\r\n"), m_sPhotoID, sContentType);
      m_sPostHeader += sTemp;
      // Create Form post footer
      m_sPostFooter = _T("\r\n----------------------------7d137f1480712--\r\n");
   }
   return S_OK;
}

HRESULT CFlickrItemFileStream::Seek(DWORD dwPos)
{
   if( dwPos == m_dwCurPos ) return S_OK;
   return E_NOTIMPL;
}

HRESULT CFlickrItemFileStream::GetCurPos(DWORD* pdwPos)
{
   *pdwPos = m_dwCurPos;
   return S_OK;
}

HRESULT CFlickrItemFileStream::GetFileSize(DWORD* pdwFileSize)
{
   *pdwFileSize = m_dwFileSize;
   return S_OK;
}

HRESULT CFlickrItemFileStream::SetFileSize(DWORD dwFileSize)
{
   m_dwFileSize = dwFileSize;
   return S_OK;
}

HRESULT CFlickrItemFileStream::Read(LPVOID pData, ULONG dwSize, ULONG& dwBytesRead)
{
   dwBytesRead = 0;
   if( m_Reason.uAccess != GENERIC_READ ) return E_ACCESSDENIED;
   if( m_hRequest == NULL ) return E_UNEXPECTED;
   BOOL bResult = ::WinHttpReadData(m_hRequest, pData, dwSize, &dwBytesRead);
   if( !bResult ) return AtlHresultFromLastError();
   m_dwCurPos += dwBytesRead;
   return dwBytesRead > 0 ? S_OK : S_FALSE;
}

HRESULT CFlickrItemFileStream::Write(LPCVOID pData, ULONG dwSize, ULONG& dwBytesWritten)
{
   dwBytesWritten = 0;
   if( m_Reason.uAccess != GENERIC_WRITE ) return E_ACCESSDENIED;
   if( m_hRequest == NULL ) return E_UNEXPECTED;
   if( m_dwCurPos == 0 && dwSize > 0 ) HR( _WriteFileHeader() );
   BOOL bResult = ::WinHttpWriteData(m_hRequest, pData, dwSize, &dwBytesWritten);
   if( !bResult ) return AtlHresultFromLastError();
   m_dwCurPos += dwBytesWritten;
   return dwSize == dwBytesWritten ? S_OK : S_FALSE;
}

HRESULT CFlickrItemFileStream::Commit()
{
   ATLTRACE(L"CFlickrItemFileStream::Commit\n");
   if( m_Reason.uAccess != GENERIC_WRITE ) return E_UNEXPECTED;
   ATLASSERT(m_dwCurPos==m_dwFileSize);
   // Write the footer too...
   DWORD dwBytesWritten = 0;
   BOOL bResult = ::WinHttpWriteData(m_hRequest, m_aPostFooter, m_cbPostFooter, &dwBytesWritten);
   if( !bResult ) return AtlHresultFromLastError();
   // Finally we can read the HTTP response...
   bResult = ::WinHttpReceiveResponse(m_hRequest, NULL);
   if( !bResult ) return AtlHresultFromLastError();
   LPSTR pstrData = (LPSTR) malloc(1); 
   CHAR buffer[1024] = { 0 }; 
   DWORD cbBuffer = 0, nLength = 0;
   while( ::WinHttpReadData(m_hRequest, buffer, sizeof(buffer), &cbBuffer) && cbBuffer > 0 ) {
      pstrData = (LPSTR) realloc(pstrData, nLength + cbBuffer + 1);
      if( pstrData == NULL ) return E_OUTOFMEMORY;
      ::CopyMemory(pstrData + nLength, buffer, cbBuffer);
      nLength += cbBuffer;
   }
   pstrData[nLength] = 0;
   // Post-process upload; create cache entry and other stuff...
   HRESULT Hr = _ShellModule.Rest.PostProcessPhotoUpload(m_sPhotoID, m_pImageset, pstrData);
   free(pstrData);
   return Hr;
}

HRESULT CFlickrItemFileStream::Close()
{
   if( m_hRequest != NULL ) ::WinHttpCloseHandle(m_hRequest); m_hRequest = NULL;
   if( m_hConnect != NULL ) ::WinHttpCloseHandle(m_hConnect); m_hConnect = NULL;
   if( m_cbPostFooter > 0 ) ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, m_spFolder->m_pidlMonitor);
   return S_OK;
}

// Implementation

HRESULT CFlickrItemFileStream::_WriteFileHeader()
{
   ATLASSERT(m_dwFileSize>0);
   if( m_dwFileSize == 0 ) return E_INVALIDARG;
   DWORD cbPostHeader = (DWORD) strlen(CT2CAEX<1024>(m_sPostHeader, CP_UTF8));
   DWORD cbPostFooter = (DWORD) strlen(CT2CAEX<1024>(m_sPostFooter, CP_UTF8));
   m_aPostHeader.Allocate(cbPostHeader);
   m_aPostFooter.Allocate(cbPostFooter);
   ::CopyMemory(m_aPostHeader, CT2CAEX<1024>(m_sPostHeader, CP_UTF8), cbPostHeader);
   ::CopyMemory(m_aPostFooter, CT2CAEX<1024>(m_sPostFooter, CP_UTF8), cbPostFooter);
   m_cbPostFooter = cbPostFooter;
   DWORD cbBufferTotal = cbPostHeader + m_dwFileSize + cbPostFooter;
   BOOL bResult = ::WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, cbBufferTotal, 0);
   if( !bResult ) return AtlHresultFromLastError();
   DWORD dwHeaderWritten = 0;
   bResult = ::WinHttpWriteData(m_hRequest, m_aPostHeader, cbPostHeader, &dwHeaderWritten);
   if( !bResult ) return AtlHresultFromLastError();
   return cbPostHeader == dwHeaderWritten ? S_OK : E_FAIL;
}

