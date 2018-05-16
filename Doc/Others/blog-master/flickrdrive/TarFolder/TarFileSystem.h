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

#include "NseFileSystem.h"
#include "xtar.h"


///////////////////////////////////////////////////////////////////////////////
// Definitions

#define TARFILE_MAGIC_ID  0xEF


///////////////////////////////////////////////////////////////////////////////
// CTarShellModule

class CTarShellModule : public CNseModule
{
public:
   // CShellModule

   LONG GetConfigInt(VFS_CONFIG Item);
   BOOL GetConfigBool(VFS_CONFIG Item);
   LPCWSTR GetConfigStr(VFS_CONFIG Item);

   HRESULT DllInstall();
   HRESULT DllUninstall();
   HRESULT ShellAction(LPCWSTR pstrType, LPCWSTR pstrCmdLine);

   BOOL DllMain(DWORD dwReason, LPVOID lpReserved);

   HRESULT CreateFileSystem(PCIDLIST_ABSOLUTE pidlRoot, CNseFileSystem** ppFS);
};


///////////////////////////////////////////////////////////////////////////////
// CTarFileSystem

class CTarFileSystem : public CNseFileSystem
{
public:
   TAR_ARCHIVE* m_pArchive;                      // Internal implementation of file-system
   volatile LONG m_cRef;                         // Reference count

   // Constructor

   CTarFileSystem();
   virtual ~CTarFileSystem();

   // Operations

   HRESULT Init(PCIDLIST_ABSOLUTE pidlRoot);

   // CShellFileSystem
   
   VOID AddRef();
   VOID Release();

   CNseItem* GenerateRoot(CShellFolder* pFolder);
};


///////////////////////////////////////////////////////////////////////////////
// CTarFileItem

class CTarFileItem : public CNseFileItem
{
public:
   CTarFileItem(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseFileItem

   SFGAOF GetSFGAOF(SFGAOF Mask);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v);

   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);
   
   HRESULT GetStream(const VFS_STREAM_REASON& Reason, CNseFileStream** ppFile);

   HRESULT CreateFolder();
   HRESULT Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName);
   HRESULT Delete();

   HMENU GetMenu();
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   // Implementation

   TAR_ARCHIVE* _GetTarArchivePtr() const;
   HRESULT _ExtractToFolder(VFS_MENUCOMMAND& Cmd);
};


///////////////////////////////////////////////////////////////////////////////
// CTarFileStream

class CTarFileStream : public CNseFileStream
{
public:
   CRefPtr<CTarFileSystem> m_spFS;               // Reference to source archive
   WCHAR m_wszFilename[TAR_MAXPATHLEN + 1];      // Local path to file
   LPBYTE m_pData;                               // Memory buffer
   DWORD m_dwAllocSize;                          // Allocated memory so far
   DWORD m_dwCurPos;                             // Current position in memory buffer
   DWORD m_dwFileSize;                           // Known size of file in memory
   UINT m_uAccess;                               // Stream opened for read or write access?

   // Constructor

   CTarFileStream(CTarFileSystem* pFS, LPCWSTR pstrFilename, UINT uAccess);
   virtual ~CTarFileStream();

   // CNseFileStream

   HRESULT Init();
   HRESULT Read(LPVOID pData, ULONG dwSize, ULONG& dwRead);
   HRESULT Write(LPCVOID pData, ULONG dwSize, ULONG& dwWritten);
   HRESULT Seek(DWORD dwPos);
   HRESULT GetCurPos(DWORD* pdwPos);
   HRESULT GetFileSize(DWORD* pdwFileSize);
   HRESULT SetFileSize(DWORD dwSize);
   HRESULT Commit();
   HRESULT Close();
};

