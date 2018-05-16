
#include "stdafx.h"

#include "xtar.h"


///////////////////////////////////////////////////////////////////////////////
// TAR helper functions

static DWORD tar_getoct(LPCSTR pstr, SIZE_T cchMax)
{
   DWORD dwValue = 0;
   while( --cchMax > 0 && *pstr != '\0' ) {
      char ch = *pstr++;
      if( ch == ' ' ) continue;
      if( ch < '0' || ch >= '8' ) break;
      dwValue *= 8;
      dwValue += ch - '0';
   }
   return dwValue;
}

static void tar_putoct(LPSTR pstr, SIZE_T cchMax, DWORD dwValue)
{
   ATLASSERT(cchMax>0);
   pstr[--cchMax] = '\0';
   while( cchMax > 0 ) {
      pstr[--cchMax] = '0' + (dwValue % 8);
      dwValue /= 8;
   }
}

static DWORD tar_roundnearestblock(DWORD dwPos)
{
   if( (dwPos % TAR_BLOCKSIZE) != 0 ) dwPos += TAR_BLOCKSIZE - (dwPos % TAR_BLOCKSIZE);
   return dwPos;
}

static void tar_getfullpath(const TAR_HEADER& Header, LPSTR pstrPath, char chSep)
{
   // TAR filenames can have all kinds of UNIX prefix formats
   // One goal is to clean out entries such as:
   //  ./  ~/  /root
   // as they don't appear meaningfull on Windows.
   SIZE_T iNameOffset = 0;
   if( Header.name[0] == '/' ) iNameOffset = 1;
   if( Header.name[0] == '.' && Header.name[1] == '/' ) iNameOffset = 2;
   if( Header.name[0] == '~' && Header.name[1] == '/' ) iNameOffset = 2;
   // Concat filename from 'prefix' and 'name' members
   strncpy(pstrPath, Header.prefix, sizeof(Header.prefix));
   strncat(pstrPath, Header.name + iNameOffset, sizeof(Header.name) - iNameOffset);
   // Directories have trailing slash
   size_t cchPath = strlen(pstrPath);
   if( cchPath > 0 && pstrPath[ cchPath - 1 ] == '/' ) pstrPath[ cchPath - 1 ] = '\0';
   // Did caller request Windows path format?
   for( LPSTR pstrConvert = pstrPath; chSep != '/' && *pstrConvert != '\0'; pstrConvert++ ) {
      if( *pstrConvert == '/' ) *pstrConvert = chSep;
   }
}

static void tar_getfolderpath(const TAR_HEADER& Header, LPSTR pstrPath, char chSep)
{
   tar_getfullpath(Header, pstrPath, chSep);
   LPSTR pstrSep = strrchr(pstrPath, chSep);
   if( pstrSep != NULL ) *pstrSep = '\0'; 
   else pstrPath[0] = '\0';
}

static void tar_getfilename(const TAR_HEADER& Header, LPSTR pstrName)
{
   char szFullPath[TAR_MAXPATHLEN + 1] = { 0 };
   tar_getfullpath(Header, szFullPath, '/');
   LPSTR pstrSep = strrchr(szFullPath, '/');
   if( pstrSep != NULL ) strcpy_s(pstrName, TAR_MAXNAMELEN, pstrSep + 1);
   else strcpy_s(pstrName, TAR_MAXNAMELEN, szFullPath);
}

static DWORD tar_findeof(const TAR_ARCHIVE* pArchive)
{
   // Most TAR files are terminated with 2 empty blocks; however the size of the
   // blocks and number actually varies between implementations. We'll have to
   // look for the real EOF marker.
   DWORD dwExpectedPos = 0;
   if( pArchive->aFiles.GetSize() > 0 ) {
      const TAR_FILEINFO& Info = pArchive->aFiles[pArchive->aFiles.GetSize() - 1];
      DWORD dwFileSize = tar_getoct(Info.Header.size, sizeof(Info.Header.size));
      dwExpectedPos = tar_roundnearestblock(Info.dwFilePos + TAR_BLOCKSIZE + dwFileSize);
   }
   DWORD dwFilePos = ::SetFilePointer(pArchive->hFile, 0, NULL, FILE_END);
   if( dwFilePos == INVALID_SET_FILE_POINTER ) return INVALID_SET_FILE_POINTER;
   if( (dwFilePos % TAR_BLOCKSIZE) != 0 ) return INVALID_SET_FILE_POINTER;
   TAR_HEADER Empty = { 0 };
   while( dwFilePos > 0 ) {
      dwFilePos -= TAR_BLOCKSIZE;
      dwFilePos = ::SetFilePointer(pArchive->hFile, dwFilePos, NULL, FILE_BEGIN);
      if( dwFilePos == INVALID_SET_FILE_POINTER ) return INVALID_SET_FILE_POINTER;
      TAR_HEADER Block = { 0 };
      DWORD dwBytesRead = 0;
      ::ReadFile(pArchive->hFile, &Block, sizeof(Block), &dwBytesRead, NULL);
      if( dwBytesRead != sizeof(Block) ) return INVALID_SET_FILE_POINTER;
      // If this block wasn't empty, then the EOF marker must be
      // at the previous block, because that was empty!
      if( memcmp(&Block, &Empty, sizeof(Block)) != 0 ) return dwFilePos + TAR_BLOCKSIZE;
      // If we arrived at where we expected the EOF marker should be, then we're also done
      if( dwFilePos == dwExpectedPos ) return dwFilePos;
   }
   return 0;
}

static time_t tar_getunixtime()
{
   SYSTEMTIME stTime = { 0 }; 
   ::GetSystemTime(&stTime); 
   FILETIME ftTime = { 0 }; 
   ::SystemTimeToFileTime(&stTime, &ftTime);
   LONGLONG llTime = ((LONGLONG) ftTime.dwHighDateTime) << 32; 
   llTime |= ftTime.dwLowDateTime; 
   llTime -= 116444736000000000; 
   llTime /= 10000000;
   return (time_t) llTime;
}

static void tar_makeheader(TAR_HEADER& Header, BOOL bIsNew, LPCSTR pstrFilename, DWORD dwFileSize, DWORD dwAttributes)
{
   // Create a .tar file header.
   // NOTE: We don't really support ustar in this version!
   BOOL bIsDir = ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
   memset(Header.name, 0, sizeof(Header.name));
   strncpy(Header.name, pstrFilename, sizeof(Header.name) - (bIsDir ? 1 : 0));
   if( bIsDir ) strncat(Header.name, "/", 1);
   for( size_t i = 0; i < sizeof(Header.name); i++ ) if( Header.name[i] == '\\' ) Header.name[i] = '/';
   // Apply default properties for a new file...
   if( bIsNew ) 
   {
      // Determine file-type
      Header.typeflag = '0';
      if( IsBitSet(dwAttributes, FILE_ATTRIBUTE_REPARSE_POINT) ) Header.typeflag = '1';
      if( IsBitSet(dwAttributes, FILE_ATTRIBUTE_DIRECTORY) ) Header.typeflag = '5';
      // Set file-size
      tar_putoct(Header.size, sizeof(Header.size), dwFileSize);
      // Set the user id
      DWORD dwGid = 500;
      DWORD dwUid = 500;
      tar_putoct(Header.gid, sizeof(Header.gid), dwGid);
      tar_putoct(Header.uid, sizeof(Header.uid), dwUid);
   }
   // Set the file-mode
   DWORD dwMode = !IsBitSet(dwAttributes, FILE_ATTRIBUTE_DIRECTORY) ? 0100664 : 040775;
   if( IsBitSet(dwAttributes, FILE_ATTRIBUTE_READONLY) ) dwMode -= 0220;
   if( IsBitSet(dwAttributes, FILE_ATTRIBUTE_HIDDEN) ) dwMode -= 0444;
   tar_putoct(Header.mode, sizeof(Header.mode), dwMode);
   // Stamp filetime with current time
   tar_putoct(Header.mtime, sizeof(Header.mtime), (DWORD) tar_getunixtime());
   // Calculate the new check (sum algorithm)
   DWORD dwChecksum = 0;
   memset(Header.chksum, ' ', sizeof(Header.chksum));
   LPBYTE pHeader = (LPBYTE) &Header;
   for( DWORD i = 0; i < sizeof(Header); i++ ) dwChecksum += pHeader[i];
   tar_putoct(Header.chksum, sizeof(Header.chksum) - 1, dwChecksum);
}

static HRESULT tar_addtocache(TAR_ARCHIVE* pArchive, TAR_FILEINFO& Info)
{
   // Test folder for duplicate entries
   if( Info.Header.typeflag == '5' ) {
      char szFullPath[TAR_MAXPATHLEN + 1] = { 0 };
      tar_getfullpath(Info.Header, szFullPath, '/');
      if( szFullPath[0] == '\0' ) return S_FALSE;
      for( int i = 0; i < pArchive->aFiles.GetSize(); i++ ) {
         if( pArchive->aFiles[i].Header.typeflag != '5' ) continue;
         char szListPath[TAR_MAXPATHLEN + 1] = { 0 };
         tar_getfullpath(pArchive->aFiles[i].Header, szListPath, '/');
         if( strcmp(szFullPath, szListPath) == 0 ) return S_FALSE;
      }
   }
   // Add it to collection
   if( !pArchive->aFiles.Add(Info) ) return E_OUTOFMEMORY;
   return S_OK;
}

static TAR_FILEINFO* tar_getfileptr(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename)
{
   char szFilename[TAR_MAXPATHLEN + 1];
   strcpy_s(szFilename, lengthof(szFilename), CW2A(pwstrFilename));
   for( int i = 0; i < pArchive->aFiles.GetSize(); i++ ) {
      char szName[TAR_MAXPATHLEN + 1] = { 0 };
      tar_getfullpath(pArchive->aFiles[i].Header, szName, '\\');
      if( _stricmp(szFilename, szName) != 0 ) continue;
      return &pArchive->aFiles.GetData()[i];
   }
   return NULL;
}

static HRESULT tar_convertfileinfo(TAR_ARCHIVE* pArchive, const TAR_FILEINFO* pInfo, WIN32_FIND_DATA* pData)
{
   // Clear and fill out WIN32_FIND_DATA structure
   ::ZeroMemory(pData, sizeof(WIN32_FIND_DATA));
   if( pInfo == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   // The filename contains the path and may be postfixed with the
   // path-separator.
   char szName[TAR_MAXNAMELEN + 1] = { 0 };
   tar_getfilename(pInfo->Header, szName);
   wcscpy_s(pData->cFileName, lengthof(pData->cFileName), CA2W(szName));
   // File timestamps
   time_t tTime = (time_t) tar_getoct(pInfo->Header.mtime, sizeof(pInfo->Header.mtime));
   LONGLONG llTime = Int32x32To64(tTime, 10000000) + 116444736000000000;
   FILETIME ftFile = { (DWORD) llTime,  llTime >> 32 };
   pData->ftLastWriteTime = pData->ftLastAccessTime = pData->ftCreationTime = ftFile;
   // Rough translation of UNIX chmod to WINDOWS file-attributes
   int iMode = tar_getoct(pInfo->Header.mode, sizeof(pInfo->Header.mode));
   if( (iMode & 030000) != 0 ) pData->dwFileAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
   if( (iMode & 0400) == 0 ) pData->dwFileAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
   if( (iMode & 0400) == 0 ) pData->dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
   if( (iMode & 0200) == 0 ) pData->dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
   switch( pInfo->Header.typeflag ) {
   case '1': pData->dwFileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT; break;
   case '2': pData->dwFileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT; break;
   case '5': pData->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY; break;
   }
   if( pData->dwFileAttributes == 0 ) pData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
   if( !IsBitSet(pData->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ) pData->dwFileAttributes |= FILE_ATTRIBUTE_VIRTUAL;
   // Filesize is available too
   pData->nFileSizeLow = tar_getoct(pInfo->Header.size, sizeof(pInfo->Header.size));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// TAR archive manipulation

/**
 * Creates an empty tar archive.
 * This method is only called by the ShellNew shell integration. 
 * It creates an empty archive. Any tar file must be terminated with two
 * empty blocks of data.
 */
HRESULT tar_createarchive(LPCWSTR pstrFilename)
{
   HANDLE hFile = ::CreateFile(pstrFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if( hFile == INVALID_HANDLE_VALUE ) return AtlHresultFromLastError();
   TAR_HEADER Empty = { 0 };
   DWORD dwBytesWritten = 0;
   ::WriteFile(hFile, &Empty, sizeof(Empty), &dwBytesWritten, NULL);
   ::WriteFile(hFile, &Empty, sizeof(Empty), &dwBytesWritten, NULL);
   ::CloseHandle(hFile);
   return S_OK;
}

/**
 * Opens an existing .tar archive.
 * Here we basically memorize the filename of the .tar achive, and we only
 * open the file later on when it is actually needed the first time.
 */
HRESULT tar_openarchive(LPCWSTR pwstrFilename, TAR_ARCHIVE** ppArchive)
{
   TAR_ARCHIVE* pArchive = new TAR_ARCHIVE();
   if( pArchive == NULL ) return E_OUTOFMEMORY;
   pArchive->hFile = INVALID_HANDLE_VALUE;
   pArchive->ftLastWrite.dwLowDateTime = 0;
   pArchive->ftLastWrite.dwHighDateTime = 0;
   wcscpy_s(pArchive->wszFilename, lengthof(pArchive->wszFilename), pwstrFilename);
   *ppArchive = pArchive;
   return S_OK;
}

/**
 * Commit file changes.
 * Flush and write new file-stamp.
 */
HRESULT tar_commit(TAR_ARCHIVE* pArchive)
{
   if( pArchive == NULL ) return S_OK;
   if( pArchive->hFile == INVALID_HANDLE_VALUE ) return S_OK;
   ::CloseHandle(pArchive->hFile);
   pArchive->hFile = INVALID_HANDLE_VALUE;
   return S_OK;
}

/**
 * Close the archive.
 */
HRESULT tar_closearchive(TAR_ARCHIVE* pArchive)
{
   if( pArchive == NULL ) return E_INVALIDARG;
   HR( tar_commit(pArchive) );
   delete pArchive;
   return S_OK;
}

/**
 * Populates the archive file/folder cache.
 * Reads and caches the directory structures of the entire .tar archive.
 * To keep this file-list fresh we call this method often and test the file-write timestamp
 * to realize when the file might have changed - even by a process outside our scope. 
 * In case of a change we re-read the directory structure. 
 * THIS COULD EASILY BE A VERY SLOW OPERATION!
 */
HRESULT tar_archiveinit(TAR_ARCHIVE* pArchive)
{
   if( pArchive == NULL ) return E_INVALIDARG;
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   // Do we have the file?
   if( pArchive->hFile == INVALID_HANDLE_VALUE ) {
      pArchive->hFile = ::CreateFile(pArchive->wszFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if( pArchive->hFile == INVALID_HANDLE_VALUE ) return AtlHresultFromLastError();
   }
   // Did the archive file change while we were away? 
   // If it did then re-read the directory structures.
   // FIX: We must use FindFirstFile() because GetFileTime() appears to use cached/memory
   //      values and we need to make sure another process didn't change our file too.
   WIN32_FIND_DATA wfd = { 0 };
   HANDLE hFind = ::FindFirstFile(pArchive->wszFilename, &wfd); 
   ::FindClose(hFind);
   if( ::CompareFileTime(&wfd.ftLastWriteTime, &pArchive->ftLastWrite) != 0 ) {
      pArchive->ftLastWrite = wfd.ftLastWriteTime;
      pArchive->aFiles.RemoveAll();
   }
   // If cache already populated, then don't waste time on it
   if( pArchive->aFiles.GetSize() > 0 ) return S_OK;
   // Read entire directory map...
   ::SetFilePointer(pArchive->hFile, 0, NULL, FILE_BEGIN);
   TAR_HEADER Header, Empty = { 0 };
   ATLASSERT(sizeof(TAR_HEADER)==TAR_BLOCKSIZE);
   while( true ) 
   {
      DWORD dwBytesRead = 0;
      if( !::ReadFile(pArchive->hFile, &Header, sizeof(Header), &dwBytesRead, NULL) ) {
         return AtlHresultFromLastError();
      }
      if( dwBytesRead != sizeof(Header) ) {
         if( ::GetFileSize(pArchive->hFile, NULL) == 0 ) break;
         return AtlHresultFromWin32(ERROR_FILE_CORRUPT);
      }
      if( Header.magic[0] != '\0' && Header.magic[0] != ' ' && Header.magic[0] != 'u' ) {
         return AtlHresultFromWin32(ERROR_FILE_CORRUPT);
      }
      // The empty block signals the EOF
      if( memcmp(&Header, &Empty, sizeof(Header)) == 0 ) break;
      // Otherwise this is a file-header preceding the file contents
      DWORD dwFileSize = tar_getoct(Header.size, sizeof(Header.size));
      DWORD dwSpool = tar_roundnearestblock(dwFileSize);
      DWORD dwNewPos = ::SetFilePointer(pArchive->hFile, (LONG) dwSpool, NULL, FILE_CURRENT);
      if( dwNewPos == INVALID_SET_FILE_POINTER ) break;
      // Any directory entry in a TAR archive may contain subpaths; we should create these non-existing
      // entries as if they where actual folder items.
      char szPath[TAR_MAXPATHLEN + 1] = { 0 };
      tar_getfullpath(Header, szPath, '/');
      for( LPSTR pstrSep = strrchr(szPath, '/'); pstrSep != NULL; pstrSep = strrchr(szPath, '/') ) {
         pstrSep[1] = '\0';
         TAR_FILEINFO Folder = { Header, dwNewPos - dwSpool - TAR_BLOCKSIZE, TRUE };
         strncpy(Folder.Header.name, szPath, sizeof(Folder.Header.name));
         Folder.Header.typeflag = '5';
         HR( tar_addtocache(pArchive, Folder) );
         pstrSep[0] = '\0';
      }
      // Add entry to list...
      TAR_FILEINFO File = { Header, dwNewPos - dwSpool - TAR_BLOCKSIZE, FALSE };
      HR( tar_addtocache(pArchive, File) );
   }
   return S_OK;
}

/**
 * Return file information.
 * Convert the archive file information to a Windows WIN32_FIND_DATA structure, which
 * contains the basic information we must know about a virtual file/folder.
 */
HRESULT tar_getfindinfo(TAR_ARCHIVE* pArchive, LPCWSTR pstrFilename, WIN32_FIND_DATA* pData)
{
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   HR( tar_archiveinit(pArchive) );
   const TAR_FILEINFO* pInfo = tar_getfileptr(pArchive, pstrFilename);
   if( pInfo == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   return tar_convertfileinfo(pArchive, pInfo, pData);
}

/**
 * Return the list of children of a sub-folder.
 */
HRESULT tar_getfilelist(TAR_ARCHIVE* pArchive, LPCWSTR pwstrPath, CSimpleValArray<WIN32_FIND_DATA>& aList)
{
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   HR( tar_archiveinit(pArchive) );
   for( int i = 0; i < pArchive->aFiles.GetSize(); i++ ) {
      const TAR_FILEINFO& Info = pArchive->aFiles[i];
      char szFilePath[TAR_MAXPATHLEN + 1] = { 0 };
      tar_getfolderpath(Info.Header, szFilePath, '\\');
      if( _wcsicmp(pwstrPath, CA2W(szFilePath)) != 0 ) continue;
      WIN32_FIND_DATA wfd = { 0 };
      tar_convertfileinfo(pArchive, &Info, &wfd);
      if( !aList.Add(wfd) ) return E_OUTOFMEMORY;
   }
   return S_OK;
}

/**
 * Rename a file or folder in the archive.
 * Notice that we do not support rename of a folder, which contains files, yet.
 */
HRESULT tar_renamefile(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename, LPCWSTR pwstrNewName)
{
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   HR( tar_archiveinit(pArchive) );
   TAR_FILEINFO* pInfo = tar_getfileptr(pArchive, pwstrFilename);
   if( pInfo == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   // BUG: This version doesn't support renaming of folders with files in it!!
   if( pInfo->Header.typeflag == '5' ) {
      CSimpleValArray<WIN32_FIND_DATA> aList;
      HR( tar_getfilelist(pArchive, pwstrFilename, aList) );
      if( aList.GetSize() > 0 ) return AtlHresultFromWin32(ERROR_NOT_SUPPORTED);
      if( pInfo->bProtected ) return E_ACCESSDENIED;
   }
   // Construct the new filename for the header
   char szFilename[TAR_MAXPATHLEN] = { 0 };
   tar_getfolderpath(pInfo->Header, szFilename, '/');
   if( strlen(szFilename) > 0 ) strcat_s(szFilename, lengthof(szFilename), "/");
   strcat_s(szFilename, lengthof(szFilename), CW2A(pwstrNewName));
   // Create a new header block and save it
   WIN32_FIND_DATA wfd = { 0 };
   tar_convertfileinfo(pArchive, pInfo, &wfd);
   TAR_HEADER Header = pInfo->Header;
   tar_makeheader(Header, FALSE, szFilename, wfd.nFileSizeLow, wfd.dwFileAttributes);
   DWORD dwCurPos = ::SetFilePointer(pArchive->hFile, pInfo->dwFilePos, NULL, FILE_BEGIN);
   if( dwCurPos == INVALID_SET_FILE_POINTER ) return AtlHresultFromLastError();
   DWORD dwWritten = 0;
   ::WriteFile(pArchive->hFile, &Header, sizeof(Header), &dwWritten, NULL);
   if( dwWritten != sizeof(pInfo->Header) ) return AtlHresultFromLastError();
   // Commit changes
   HR( tar_commit(pArchive) );
   return S_OK;
}

/**
 * Delete a file or folder.
 */
HRESULT tar_deletefile(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename)
{
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   HR( tar_archiveinit(pArchive) );
   // OK, this is not a very clever way to handle deletes! On multiple deletes
   // we will absolutely kill performance. But this works for now. We simply
   // copy/move the blocks after the deleted file back in position.
   TAR_FILEINFO* pInfo = tar_getfileptr(pArchive, pwstrFilename);
   if( pInfo == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   // BUG: Slowly crawl subfolders and delete recursively = Terrible!
   //      We also assume that traversing the files from the back of the
   //      list will yield a stable fileptr list, even when recursive.
   if( pInfo->Header.typeflag == '5' ) {
      CSimpleValArray<WIN32_FIND_DATA> aList;
      HR( tar_getfilelist(pArchive, pwstrFilename, aList) );
      for( int i = aList.GetSize() - 1; i >= 0; --i ) {
         WCHAR wszFilename[MAX_PATH] = { 0 };
         wcscpy_s(wszFilename, lengthof(wszFilename), pwstrFilename);
         ::PathAppend(wszFilename, aList[i].cFileName);
         HR( tar_deletefile(pArchive, wszFilename) );
      }
      HR( tar_archiveinit(pArchive) );
      pInfo = tar_getfileptr(pArchive, pwstrFilename);
      if( pInfo == NULL ) return S_OK;
      if( pInfo->bProtected ) return S_OK;
   }
   // Lock the .tar file
   OVERLAPPED Overlapped = { 0 };
   BOOL bLocked = ::LockFileEx(pArchive->hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, TAR_BLOCKSIZE, 0, &Overlapped);
   if( !bLocked ) return AtlHresultFromLastError();
   // Find the source and target position
   DWORD dwDstPos = pInfo->dwFilePos;
   DWORD dwFileSize = tar_getoct(pInfo->Header.size, sizeof(pInfo->Header.size));
   DWORD dwSrcPos = tar_roundnearestblock(pInfo->dwFilePos + TAR_BLOCKSIZE + dwFileSize);
   while( true ) {
      BYTE Block[TAR_BLOCKSIZE] = { 0 };
      DWORD dwBytesRead = 0, dwBytesWritten = 0;
      ::SetFilePointer(pArchive->hFile, (LONG) dwSrcPos, 0, FILE_BEGIN);
      if( !::ReadFile(pArchive->hFile, Block, sizeof(Block), &dwBytesRead, NULL) ) break;
      if( dwBytesRead != sizeof(Block) ) break;
      ::SetFilePointer(pArchive->hFile, (LONG) dwDstPos, 0, FILE_BEGIN);
      if( !::WriteFile(pArchive->hFile, Block, sizeof(Block), &dwBytesWritten, NULL) ) break;
      dwSrcPos += TAR_BLOCKSIZE;
      dwDstPos += TAR_BLOCKSIZE;
   }
   // We can now truncate the remaining
   ::SetFilePointer(pArchive->hFile, (LONG) dwDstPos, 0, FILE_BEGIN);
   ::SetEndOfFile(pArchive->hFile);
   ::UnlockFile(pArchive->hFile, 0, 0, TAR_BLOCKSIZE, 0);
   // Commit changes
   HR( tar_commit(pArchive) );
   return S_OK;
}

/**
 * Create a new sub-folder in the archive.
 */
HRESULT tar_createfolder(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename)
{
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   // We can create a folder by writing an empty file with the correct
   // file attributes. The tar_writefile() will know what to do.
   BYTE aEmpty[1] = { 0 };
   return tar_writefile(pArchive, pwstrFilename, aEmpty, 0, FILE_ATTRIBUTE_DIRECTORY);
}

/**
 * Change the file-attributes of a file.
 */
HRESULT tar_setfileattribs(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename, DWORD dwAttributes)
{
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   HR( tar_archiveinit(pArchive) );
   TAR_FILEINFO* pInfo = tar_getfileptr(pArchive, pwstrFilename);
   if( pInfo == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   // Make up a new header for the entry
   TAR_HEADER Header = pInfo->Header;
   tar_makeheader(Header, FALSE, CW2A(pwstrFilename), 0, dwAttributes);
   DWORD dwCurPos = ::SetFilePointer(pArchive->hFile, pInfo->dwFilePos, NULL, FILE_BEGIN);
   if( dwCurPos == INVALID_SET_FILE_POINTER ) return AtlHresultFromLastError();
   DWORD dwWritten = 0;
   ::WriteFile(pArchive->hFile, &Header, sizeof(Header), &dwWritten, NULL);
   if( dwWritten != sizeof(pInfo->Header) ) return AtlHresultFromLastError();
   // Commit changes
   HR( tar_commit(pArchive) );
   return S_OK;
}

/**
 * Create a new file in the archive.
 * This method expects a memory buffer containing the entire file contents.
 */
HRESULT tar_writefile(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename, const LPBYTE pbBuffer, DWORD dwFileSize, DWORD dwAttributes)
{
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   HRESULT Hr = S_OK;
   HR( tar_archiveinit(pArchive) );
   // Must remove file first in case it is already there
   tar_deletefile(pArchive, pwstrFilename);
   // So we must create the .tar file header
   TAR_HEADER Empty = { 0 };
   TAR_HEADER Header = { 0 };
   tar_makeheader(Header, TRUE, CW2A(pwstrFilename), dwFileSize, dwAttributes);
   // New files are always appended to the end of the archive.
   // There are 2 empty blocks at the end of the file.
   DWORD dwFilePos = ::SetFilePointer(pArchive->hFile, tar_findeof(pArchive), NULL, FILE_BEGIN);
   if( dwFilePos == INVALID_SET_FILE_POINTER ) return AtlHresultFromLastError();
   // Lock the .tar file
   OVERLAPPED Overlapped = { 0 };
   BOOL bLocked = ::LockFileEx(pArchive->hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, TAR_BLOCKSIZE, 0, &Overlapped);
   if( !bLocked ) return AtlHresultFromLastError();
   // Write the header before the EOF blocks
   DWORD dwNewPos = ::SetFilePointer(pArchive->hFile, dwFilePos, NULL, FILE_BEGIN);
   if( dwNewPos == INVALID_SET_FILE_POINTER ) Hr = AtlHresultFromLastError();
   if( SUCCEEDED(Hr) ) {
      DWORD dwBytesWritten = 0;
      ::WriteFile(pArchive->hFile, &Header, sizeof(Header), &dwBytesWritten, NULL);
      if( dwBytesWritten != sizeof(Header) ) Hr = AtlHresultFromLastError();
      if( SUCCEEDED(Hr) ) {
         // Write the file contents
         ::WriteFile(pArchive->hFile, pbBuffer, dwFileSize, &dwBytesWritten, NULL);
         if( dwBytesWritten != dwFileSize && dwFileSize > 0 ) Hr = AtlHresultFromLastError();
         // Make sure the file occupies an entire block size
         if( SUCCEEDED(Hr) && (dwFileSize % TAR_BLOCKSIZE) != 0 ) {
            ::WriteFile(pArchive->hFile, &Empty, TAR_BLOCKSIZE - (dwFileSize % TAR_BLOCKSIZE), &dwBytesWritten, NULL);
         }
         // If something failed, attempt to roll back to original size
         if( FAILED(Hr) ) {
            ::SetFilePointer(pArchive->hFile, dwNewPos, NULL, FILE_BEGIN);
         }
         // The entire archive must always be terminated with 2 empty blocks
         ::WriteFile(pArchive->hFile, &Empty, sizeof(Empty), &dwBytesWritten, NULL);
         ::WriteFile(pArchive->hFile, &Empty, sizeof(Empty), &dwBytesWritten, NULL);
      }
      ::SetEndOfFile(pArchive->hFile);
   }
   ::UnlockFile(pArchive->hFile, 0, 0, TAR_BLOCKSIZE, 0);
   // Commit changes
   HR( tar_commit(pArchive) );
   return Hr;
}

/**
 * Reads a file from the archive.
 * The method returns the contents of the entire file in a memory buffer.
 */
HRESULT tar_readfile(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename, LPBYTE* ppbBuffer, DWORD& dwFileSize)
{
   CComCritSecLock<CComCriticalSection> lock(pArchive->csLock);
   HRESULT Hr = S_OK;
   HR( tar_archiveinit(pArchive) );
   TAR_FILEINFO* pInfo = tar_getfileptr(pArchive, pwstrFilename);
   if( pInfo == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   dwFileSize = (DWORD) tar_getoct(pInfo->Header.size, sizeof(pInfo->Header.size));
   DWORD dwFilePos = pInfo->dwFilePos + TAR_BLOCKSIZE;
   DWORD dwNewPos = ::SetFilePointer(pArchive->hFile, dwFilePos, NULL, FILE_BEGIN);
   if( dwNewPos == INVALID_SET_FILE_POINTER ) return AtlHresultFromLastError();
   OVERLAPPED Overlapped = { 0 };
   BOOL bLocked = ::LockFileEx(pArchive->hFile, 0, 0, TAR_BLOCKSIZE, 0, &Overlapped);
   LPBYTE pBuffer = (LPBYTE) malloc(dwFileSize);
   if( pBuffer == NULL ) Hr = E_OUTOFMEMORY, dwFileSize = 0;
   DWORD dwBytesRead = 0;
   ::ReadFile(pArchive->hFile, pBuffer, dwFileSize, &dwBytesRead, NULL);
   if( dwBytesRead != dwFileSize ) Hr = AtlHresultFromLastError();
   if( FAILED(Hr) ) free(pBuffer); else *ppbBuffer = pBuffer;
   if( bLocked ) ::UnlockFile(pArchive->hFile, 0, 0, TAR_BLOCKSIZE, 0);
   return Hr;
}

