/////////////////////////////////////////////////////////////////////////////
// Shell Extension helper functions
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


///////////////////////////////////////////////////////////////////////////////
// Macros

#ifndef lengthof
   #define lengthof(x)  (sizeof(x)/sizeof(x[0]))
#endif  // lengthof

#ifndef offsetof
  #define offsetof(type, field)  ((int)&((type*)0)->field)
#endif  // offsetof

#ifndef IsBitSet
   #define IsBitSet(val, bit)  (((val)&(bit))!=0)
#endif // IsBitSet

#ifdef _DEBUG
   #define HR(expr)  { HRESULT _hr; if(FAILED(_hr=(expr))) { _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #expr); _CrtDbgBreak(); return _hr; } }  
#else
   #define HR(expr)  { HRESULT _hr; if(FAILED(_hr=(expr))) return _hr; }
#endif // _DEBUG


///////////////////////////////////////////////////////////////////////////////
// Debug functions

#ifdef _DEBUG
   LPCWSTR DbgGetIID(REFIID riid);
   LPCTSTR DbgGetCF(CLIPFORMAT cf);
   LPCWSTR DbgGetPKEY(REFPROPERTYKEY pk);
   LPCTSTR DbgGetSHGDNF(SHGDNF Flags);
   LPCTSTR DbgGetSHCONTF(SHCONTF Flags);
   LPCTSTR DbgGetSFGAOF(SFGAOF Flags);
   LPCTSTR DbgGetGPSF(GETPROPERTYSTOREFLAGS Flags);
#else
   #define DbgGetIID(x) (void(0))
   #define DbgGetCF(x) (void(0))
   #define DbgGetPKEY(x) (void(0))
   #define DbgGetSHGDNF(x) (void(0))
   #define DbgGetSHCONTF(x) (void(0))
   #define DbgGetSFGAOF(x) (void(0))
   #define DbgGetGPSF(x) (void(0))
#endif // _DEBUG


///////////////////////////////////////////////////////////////////////////////
// Helpers

GUID UuidFromString(LPOLESTR pwstr);
BOOL ILIsAligned64(PCUIDLIST_RELATIVE pidl);
HMENU GetSubMenu(HMENU hMenu, LPCTSTR pstrTitle);
FILETIME GetFileModifiedTime(LPCTSTR pstrFilename);
BOOL IsFileTimeValid(const FILETIME& ft);


///////////////////////////////////////////////////////////////////////////////
// COM Registration helpers

HRESULT UpdatePropertySchemaFromResource(UINT uRes, BOOL bRegister);
HRESULT UpdateSendToFromProject(LPCWSTR pstrDisplayName, LPCWSTR pstrProjectName, BOOL bRegister);
HRESULT UpdateStartMenuLink(LPCWSTR pstrDisplayName, LPCWSTR pstrDescription, KNOWNFOLDERID FolderId, BOOL bRegister);


///////////////////////////////////////////////////////////////////////////////
// Shell helpers

HRESULT StrToSTRRET(LPCWSTR pwstr, STRRET* pSrRet);
HRESULT SHFilterEnumItem(SHCONTF grfFlags, const WIN32_FIND_DATA& wfd);
HRESULT SHGetFirstIDListFromDataObject(IDataObject* pDataObj, PIDLIST_ABSOLUTE* ppidl);
HRESULT SHGetBindCtxParam(IBindCtx* pbc, LPOLESTR pstrParam, REFIID riid, LPVOID* ppRetVal);
HRESULT SHCreateItemFromRelativeName(IShellFolder* pFolder, LPCWSTR pszName, IBindCtx* pbc, REFIID riid, LPVOID* ppv);
HRESULT SHGetModuleIconOverlayIndex(int iIconIndex, int* pOverlayIndex);
HRESULT SHCreateModuleExtractIcon(int iIconIndex, REFIID riid, LPVOID* ppRetVal);
HRESULT SHGetFileSysIcon(LPCTSTR pstrFileName, DWORD dwFileAttributes, UINT uIconFlags, int* pIconIndex);
HRESULT SHCreateFileOperation(HWND hWnd, DWORD dwFlags, IFileOperation** ppRetVal);
HRESULT SHCreateFolderInfoTip(IShellFolder* pShellFolder, PCUITEMID_CHILD pidlItem, IQueryInfo* pInnerInfo, REFIID riid, LPVOID* ppRetVal);


///////////////////////////////////////////////////////////////////////////////
// IDataObject helpers

class _U_STRINGorCF
{
public:
   _U_STRINGorCF(LPCTSTR lpString) : m_cf((CLIPFORMAT)::RegisterClipboardFormat(lpString))
   { }
   _U_STRINGorCF(CLIPFORMAT cf) : m_cf(cf)
   { }
   CLIPFORMAT m_cf;
};

BOOL DataObj_HasFileClipFormat(IDataObject* pDataObj);
BOOL DataObj_HasClipFormat(IDataObject* pDataObj, _U_STRINGorCF cf, DWORD tymed = TYMED_HGLOBAL);
HRESULT DataObj_SetData(IDataObject* pDataObj, _U_STRINGorCF cf, LPCVOID pValue, SIZE_T cbSize);
HRESULT DataObj_GetData(IDataObject* pDataObj, _U_STRINGorCF cf, LPVOID pValue, SIZE_T cbSize);


///////////////////////////////////////////////////////////////////////////////
// String classes

// Stack based string buffer. Loads strings from the application resource.
template<int nSize>
class CResString
{
public:
   TCHAR m_szStr[nSize];

   CResString()
   {
      m_szStr[0] = '\0';
   }

   CResString(UINT ID)
   {
      LoadString(ID);
   }

   ATLINLINE void LoadString(UINT uID)
   {
      ::LoadString(_pModule->GetResourceInstance(), uID, m_szStr, lengthof(m_szStr));
   }

   ATLINLINE bool IsEmpty()
   {
      return m_szStr[0] == '\0';
   }

   operator LPCTSTR() const
   {
      return m_szStr;
   }
};


// String allocated by CoTaskMemAlloc
class CCoTaskString
{
public:
   LPWSTR m_pstr;

   CCoTaskString() : m_pstr(NULL)
   {
   }

   CCoTaskString(LPCWSTR pstrText) : m_pstr(NULL)
   {
      if( pstrText == NULL ) return;
      m_pstr = (LPWSTR) ::CoTaskMemAlloc((1 + wcslen(pstrText)) * sizeof(WCHAR));
      if( m_pstr != NULL ) wcscpy(m_pstr, pstrText);
   }

   ~CCoTaskString()
   {
      if( m_pstr != NULL ) ::CoTaskMemFree(m_pstr);
   }

   void Attach(LPWSTR pstr)
   {
      if( m_pstr != NULL ) ::CoTaskMemFree(m_pstr);
      m_pstr = pstr;
   }

   LPWSTR Detach()
   {
      LPWSTR pstr = m_pstr;
      m_pstr = NULL;
      return pstr;
   }

   bool IsEmpty() const
   {
      return m_pstr == NULL || m_pstr[0] == '\0';
   }

   operator LPWSTR() const
   {
      return m_pstr == NULL ? L"" : m_pstr;
   }

   LPWSTR* operator&()
   {
      ATLASSERT(m_pstr==NULL);
      return &m_pstr;
   }
};


///////////////////////////////////////////////////////////////////////////////
// Helper classes

// A little hourglass helper
class CWaitCursor
{
public:
   HCURSOR m_hOrigCursor;

   CWaitCursor()
   { 
      m_hOrigCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT)); 
   }

   ~CWaitCursor() 
   { 
      ::SetCursor(m_hOrigCursor); 
   }
};


// A wrapper for LoadLibrary/FreeLibrary
class CWin32Library
{
public:
   HINSTANCE m_hInst;

   CWin32Library(HINSTANCE hInst = NULL) : m_hInst(hInst)
   {
   }

   ~CWin32Library()
   { 
      if( m_hInst != NULL ) ::FreeLibrary(m_hInst);
   }

   HINSTANCE Load(LPCTSTR pstrFilename) 
   { 
      if( m_hInst != NULL ) ::FreeLibrary(m_hInst);
      m_hInst = ::LoadLibrary(pstrFilename);
      return m_hInst;
   }

   operator HINSTANCE() const 
   { 
      return m_hInst; 
   }
};


// OleInitialize wrapper
class COleInitialize
{
public:
   COleInitialize()
   {
      ::OleInitialize(NULL);
   }

   ~COleInitialize()
   {
      ::OleUninitialize();
   }
};


// Reference-counted pointer
template< typename T >
class CRefPtr
{
public:
   T* m_p;

   CRefPtr() : m_p(NULL)
   {
   }

   CRefPtr(T* p)
   {
      m_p = p;
      if( m_p != NULL ) m_p->AddRef();
   }

   CRefPtr(const CRefPtr<T>& p)
   {
      m_p = p.m_p;
      if( m_p != NULL ) m_p->AddRef();
   }

   ~CRefPtr()
   {
      Release();
   }

   operator T*() const
   {
      return m_p;
   }

   T& operator*() const
   {
      ATLENSURE(m_p!=NULL);
      return *m_p;
   }

   T** operator&()
   {
      ATLASSERT(m_p==NULL);
      return &m_p;
   }

   T* operator=(T* p)
   {
      if( m_p == p ) return *this;
      if( m_p != NULL ) m_p->Release();
      m_p = p;
      if( m_p != NULL ) m_p->AddRef();
      return *this;
   }

   T* operator=(CRefPtr<T>& p)
   {
      return operator=(p.m_p);
   }

   T* operator->() const
   {
      ATLASSERT(m_p!=NULL);
      return m_p;
   }

   bool operator!() const
   {
      return m_p == NULL;
   }

   bool operator==(T* p) const
   {
      return m_p == p;
   }

   void Release()
   {
      T* pTemp = m_p;
      if( pTemp != NULL ) {
         m_p = NULL;
         pTemp->Release();
      }
   }
};

