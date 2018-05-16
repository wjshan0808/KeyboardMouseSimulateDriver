/////////////////////////////////////////////////////////////////////////////
// FlickDrive Shell Extension
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
#include "rest.h"


///////////////////////////////////////////////////////////////////////////////
// Definitions

#define FLICKRFS_MAGIC_ID_ROOT      0xC0
#define FLICKRFS_MAGIC_ID_ACCOUNT   0xC1
#define FLICKRFS_MAGIC_ID_IMAGESET  0xC2
#define FLICKRFS_MAGIC_ID_PHOTO     0xC8


///////////////////////////////////////////////////////////////////////////////
// CFlickrModule

class CFlickrModule : public CNseModule
{
public:
   CFlickrAPI Rest;

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
// CFlickrFileSystem

class CFlickrFileSystem : public CNseFileSystem
{
public:
   volatile LONG m_cRef;

   // Constructor

   CFlickrFileSystem();
   virtual ~CFlickrFileSystem();

   // Operations

   HRESULT Init();

   // CShellFileSystem

   VOID AddRef();
   VOID Release();

   CNseItem* GenerateRoot(CShellFolder* pFolder);
};


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemRoot

class CFlickrItemRoot : public CNseBaseItem
{
public:
   CFlickrItemRoot(CShellFolder* pFolder);

   // CNseBaseItem

   BYTE GetType();
   VFS_FIND_DATA GetFindData();
   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_FOLDERSETTINGS GetFolderSettings();

   HRESULT IsDropDataAvailable(IDataObject* pDataObj);
   HRESULT GetPropertyPage(CNsePropertyPage** ppPage);
   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);

   HMENU GetMenu();
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

  
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);

   // Implementation

   HRESULT _DoPasteSendTo(VFS_MENUCOMMAND& Cmd);
};


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemAccount

#if !defined(_M_X64) && !defined(_M_IA64)
#include <pshpack1.h>
#endif
typedef struct tagFLICKRFS_PIDL_ACCOUNT
{
   // SHITEMID 
   USHORT cb;
   // Type identifiers
   BYTE magic;
   BYTE reserved;
   // Account ID
   WCHAR cNSID[62];
} FLICKRFS_PIDL_ACCOUNT;
#if !defined(_M_X64) && !defined(_M_IA64)
#include <poppack.h>
#endif


class CFlickrItemAccount : public CNseBaseItem
{
public:
   TFlickrAccount* m_pAccount;

   // Constructor 

   CFlickrItemAccount(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseBaseItem

   BYTE GetType();
   VFS_FIND_DATA GetFindData();
   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_FOLDERSETTINGS GetFolderSettings();

   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);

   HMENU GetMenu();
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);

   HRESULT Delete();
   HRESULT Refresh(VFS_REFRESH Reason);

   // Static members

   static PCITEMID_CHILD GenerateITEMID(const FLICKRFS_PIDL_ACCOUNT& src);
   static PCITEMID_CHILD GenerateITEMID(const TFlickrAccount* pAccount);
   static PCITEMID_CHILD GenerateITEMID(const WIN32_FIND_DATA& wfd);

   // Implementation

   HRESULT _EnsureFlickrRef();
   HRESULT _DoCreatePhotoSet(VFS_MENUCOMMAND& Cmd);
   HRESULT _DoCreateTagFolder(VFS_MENUCOMMAND& Cmd);
};


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemImageset

#if !defined(_M_X64) && !defined(_M_IA64)
#include <pshpack1.h>
#endif
typedef struct tagFLICKRFS_PIDL_IMAGESET
{
   // SHITEMID 
   USHORT cb;
   // Type identifiers
   BYTE magic;
   BYTE reserved;
   // Imageset ID
   WCHAR cImagesetID[62];
} FLICKRFS_PIDL_IMAGESET;
#if !defined(_M_X64) && !defined(_M_IA64)
#include <poppack.h>
#endif


class CFlickrItemImageset : public CNseBaseItem
{
public:
   TFlickrImageset* m_pImageset;

   // Constructor

   CFlickrItemImageset(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseBaseItem

   BYTE GetType();
   VFS_FIND_DATA GetFindData();
   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_FOLDERSETTINGS GetFolderSettings();
   VFS_PROPSTATE GetPropertyState(REFPROPERTYKEY pk);

   HRESULT IsDropDataAvailable(IDataObject* pDataObj);
   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v);

   HMENU GetMenu();
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);

   HRESULT Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName);
   HRESULT Delete();
   HRESULT Refresh(VFS_REFRESH Reason);

   // Static members

   static PCITEMID_CHILD GenerateITEMID(const FLICKRFS_PIDL_IMAGESET& src);
   static PCITEMID_CHILD GenerateITEMID(const TFlickrImageset* pImageset);
   static PCITEMID_CHILD GenerateITEMID(const WIN32_FIND_DATA& wfd);

   // Implementation

   HRESULT _EnsureFlickrRef();
   HRESULT _DoPastePhotos(VFS_MENUCOMMAND& Cmd);
};


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemImageset

#if !defined(_M_X64) && !defined(_M_IA64)
#include <pshpack1.h>
#endif
typedef struct tagFLICKRFS_PIDL_PHOTO
{
   // SHITEMID 
   USHORT cb;
   // Type identifiers
   BYTE magic;
   BYTE reserved;
   // Photo ID
   WCHAR cPhotoID[50];
} FLICKRFS_PIDL_PHOTO;
#if !defined(_M_X64) && !defined(_M_IA64)
#include <poppack.h>
#endif


class CFlickrItemPhoto : public CNseBaseItem
{
public:
   TFlickrPhoto* m_pPhoto;

   // Constructor 

   CFlickrItemPhoto(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseBaseItem

   BYTE GetType();
   VFS_FIND_DATA GetFindData();
   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_PROPSTATE GetPropertyState(REFPROPERTYKEY pkey);

   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetThumbnail(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v);

   HMENU GetMenu();
   HRESULT SetMenuState(const VFS_MENUSTATE& State);
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   HRESULT Delete();

   HRESULT GetStream(const VFS_STREAM_REASON& Reason, CNseFileStream** ppFile);

   // Static members

   static PCITEMID_CHILD GenerateITEMID(const FLICKRFS_PIDL_PHOTO& src);
   static PCITEMID_CHILD GenerateITEMID(const TFlickrPhoto* pPhoto);
   static PCITEMID_CHILD GenerateITEMID(const WIN32_FIND_DATA& wfd);

   // Implementation

   HRESULT _EnsureFlickrRef();
   TFlickrImageset* _FindParentImageset() const;
};


///////////////////////////////////////////////////////////////////////////////
// CFlickrItemFileStream

class CFlickrItemFileStream : public CNseFileStream
{
public:
   VFS_STREAM_REASON m_Reason;
   CRefPtr<CShellFolder> m_spFolder;
   TFlickrImageset* m_pImageset;
   CString m_sPhotoID;
   HINTERNET m_hConnect;
   HINTERNET m_hRequest;
   DWORD m_dwCurPos;
   DWORD m_dwFileSize;
   DWORD m_cbPostFooter;
   CString m_sPostHeader;
   CString m_sPostFooter;
   CAutoVectorPtr<CHAR> m_aPostHeader;
   CAutoVectorPtr<CHAR> m_aPostFooter;

   // Constructor

   CFlickrItemFileStream(const VFS_STREAM_REASON& Reason, CShellFolder* pFolder, TFlickrPhoto* pPhoto, TFlickrImageset* pImageset);
   virtual ~CFlickrItemFileStream();

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

   // Implementation

   HRESULT _WriteFileHeader();
};

