/////////////////////////////////////////////////////////////////////////////
// Windows Registry Shell Extension
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


#define REG_MAGICID_KEY      0xA0
#define REG_MAGICID_VALUE    0xA1

#define MAX_REGPATH          MAX_PATH


#if !defined(_M_X64) && !defined(_M_IA64)
#include <pshpack1.h>
#endif
typedef struct tagREGKEYPIDLINFO
{
   // SHITEMID 
   USHORT cb;
   // Type identifiers
   BYTE magic;
   BYTE reserved;
   // Registry info
   WCHAR cName[MAX_REGPATH];
   WCHAR cPath[MAX_REGPATH];
   DWORD dwValueType;
   FILETIME ftLastWrite;
   BOOL bIsSpawned;
   BOOL reserved2;
} REGKEYPIDLINFO;
#if !defined(_M_X64) && !defined(_M_IA64)
#include <poppack.h>
#endif


///////////////////////////////////////////////////////////////////////////////
// CRegShellModule

class CRegShellModule : public CNseModule
{
public:
   // CNseModule

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
// CRegFileSystem

class CRegFileSystem : public CNseFileSystem
{
public:
   volatile LONG m_cRef;                         // Reference count

   // Constructor

   CRegFileSystem();

   // CNseFileSystem

   VOID AddRef();
   VOID Release();

   CNseItem* GenerateRoot(CShellFolder* pFolder);
};


///////////////////////////////////////////////////////////////////////////////
// CRegItemKey

class CRegItemKey : public CNseBaseItem
{
public:
   const REGKEYPIDLINFO* m_pRegInfo;             // Reference to data inside PIDL item

   CRegItemKey(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   
   // CNseItem

   BYTE GetType();
   SFGAOF GetSFGAOF(SFGAOF Mask);
   VFS_FIND_DATA GetFindData();
   VFS_FOLDERSETTINGS GetFolderSettings();

   HRESULT GetSysIcon(UINT uIconFlags, int* pIconIndex);
   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   
   HRESULT GetIdentity(LPWSTR pstrFilename);

   HMENU GetMenu();
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);

   HRESULT CreateFolder();
   HRESULT Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName);
   HRESULT Delete();


   // Implementation

   typedef enum REGOPEN_ACC { REGACC_OPEN,  REGACC_CREATE };
   typedef enum REGOPEN_PATH { REGPATH_FULLITEM,  REGPATH_PARENT };

   HRESULT _DoNewKey(const VFS_MENUCOMMAND& Cmd, UINT uLabelRes);
   HRESULT _DoNewValue(const VFS_MENUCOMMAND& Cmd, UINT uLabelRes, DWORD dwType);
   BOOL _IsTopLevel() const;

   // Static members

   static PCITEMID_CHILD GenerateITEMID(const REGKEYPIDLINFO& src);
   static PCITEMID_CHILD GenerateITEMID(const WIN32_FIND_DATA& wfd);
};


///////////////////////////////////////////////////////////////////////////////
// CRegItemValue

class CRegItemValue : public CNseBaseItem
{
public:
   const REGKEYPIDLINFO* m_pRegInfo;             // Reference to data inside PIDL item

   CRegItemValue(CShellFolder* m_pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseItem

   BYTE GetType();
   SFGAOF GetSFGAOF(SFGAOF Mask);
   VFS_FIND_DATA GetFindData();

   HRESULT GetSysIcon(UINT uIconFlags, int* pIconIndex);
   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v);

   HRESULT Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName);
   HRESULT Delete();

   // Implementation

   HRESULT GetIdentity(LPWSTR pstrFilename);

   // Static members

   static PCITEMID_CHILD GenerateITEMID(const REGKEYPIDLINFO& src);
   static PCITEMID_CHILD GenerateITEMID(const WIN32_FIND_DATA& wfd);
};

