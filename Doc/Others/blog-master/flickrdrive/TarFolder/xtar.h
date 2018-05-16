/////////////////////////////////////////////////////////////////////////////
// TAR Folder Shell Extension
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2009 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#pragma once


#define TAR_MAXNAMELEN 100
#define TAR_MAXPATHLEN 255
#define TAR_BLOCKSIZE  512


// The official TAR archive block structure. This structure is found in the .tar
// archive preceding any directory and file entry. There is no actual header structure
// describing the archive itself, and it merely ends with 2 empty blocks.
typedef struct tagTAR_HEADER
{
   char name[100];
   char mode[8];
   char uid[8];
   char gid[8];
   char size[12];
   char mtime[12];
   char chksum[8];
   char typeflag;
   char linkname[100];
   char magic[6];
   char version[2];
   char uname[32];
   char gname[32];
   char devmajor[8];
   char devminor[8];
   char prefix[155];
   char junk[12];
} TAR_HEADER;


typedef struct tagTAR_FILEINFO
{
   TAR_HEADER Header;                            // The actual file-info entry in the archive
   DWORD dwFilePos;                              // Fileposition of file-info in archive
   BOOL bProtected;                              // Folder is "temporary" and does not have its own entry in the source file?
} TAR_FILEINFO;


typedef struct tagTAR_ARCHIVE
{
   HANDLE hFile;                                 // Handle to source file (.tar archive)
   WCHAR wszFilename[MAX_PATH];                  // Filename of source file
   FILETIME ftLastWrite;                         // Cache of last known file write-timestamp
   CSimpleValArray<TAR_FILEINFO> aFiles;         // Cache of files in archive
   CComAutoCriticalSection csLock;               // Thread lock
} TAR_ARCHIVE;


HRESULT tar_createarchive(LPCWSTR pstrFilename);
HRESULT tar_openarchive(LPCWSTR pstrFilename, TAR_ARCHIVE** ppArchive);
HRESULT tar_closearchive(TAR_ARCHIVE* pArchive);
HRESULT tar_getfindinfo(TAR_ARCHIVE* pArchive, LPCWSTR pstrFilename, WIN32_FIND_DATA* pData);
HRESULT tar_getfilelist(TAR_ARCHIVE* pArchive, LPCWSTR pstrPath, CSimpleValArray<WIN32_FIND_DATA>& aList);
HRESULT tar_renamefile(TAR_ARCHIVE* pArchive, LPCWSTR pstrFilename, LPCWSTR pstrNewName);
HRESULT tar_deletefile(TAR_ARCHIVE* pArchive, LPCWSTR pstrFilename);
HRESULT tar_createfolder(TAR_ARCHIVE* pArchive, LPCWSTR pstrFilename);
HRESULT tar_setfileattribs(TAR_ARCHIVE* pArchive, LPCWSTR pstrFilename, DWORD dwAttributes);
HRESULT tar_readfile(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename, LPBYTE* ppbBuffer, DWORD& dwFileSize);
HRESULT tar_writefile(TAR_ARCHIVE* pArchive, LPCWSTR pwstrFilename, const LPBYTE pbBuffer, DWORD dwFileSize, DWORD dwAttributes);
