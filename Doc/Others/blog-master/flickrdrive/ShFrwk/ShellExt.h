/////////////////////////////////////////////////////////////////////////////
// Shell Extension wrappers
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

#if !defined(STRICT_TYPED_ITEMIDS)
#error You must define STRICT_TYPED_ITEMIDS in the stdafx.h header
#endif

#include <propkey.h>
#include <propvarutil.h>
#pragma comment (lib, "propsys.lib")


///////////////////////////////////////////////////////////////////////////////
// CPidl

class CPidl
{
public:
   PIDLIST_ABSOLUTE m_p;

   // Constructor/Destructor
   
   CPidl() : m_p(NULL)
   {
   }

   ~CPidl()
   {
      Delete();
   }

   CPidl(const CPidl& pidl)
   {
      m_p = ILCloneFull(pidl.m_p);
   }

   CPidl(PCIDLIST_ABSOLUTE pidl)
   {
      m_p = ILCloneFull(pidl);
   }

   CPidl(PCUIDLIST_RELATIVE pidl)
   {
      m_p = ILCloneFull((PCUIDLIST_ABSOLUTE) pidl);
   }

   CPidl(PCUITEMID_CHILD pidl)
   {
      m_p = (PIDLIST_ABSOLUTE) ILCloneChild(pidl);
   }

   CPidl(PCIDLIST_ABSOLUTE pidl, PCUIDLIST_RELATIVE pidlPath)
   {
      m_p = ::ILCombine(pidl, pidlPath);
   }

   CPidl(PCIDLIST_ABSOLUTE pidl, PCUIDLIST_RELATIVE pidlPath, PCUITEMID_CHILD pidlChild)
   {
      m_p = ::ILCombine(pidl, pidlPath);
      if( pidlChild != NULL ) m_p = (PIDLIST_ABSOLUTE) ::ILAppendID(m_p, (LPSHITEMID) pidlChild, TRUE);
   }

   // Operations

   void Delete()
   {
      if( m_p != NULL ) ::ILFree(m_p);
      m_p = NULL;
   }

   BOOL Create(LPSHITEMID pSrcItem)
   {
      Delete();
      m_p = (PIDLIST_ABSOLUTE) ::ILAppendID(NULL, pSrcItem, TRUE);
      return m_p != NULL;
   }

   BOOL CreateDelegate(IMalloc* pMalloc, LPSHITEMID pSrcItem)
   {
      Delete();
      // Returns a PDELEGATEITEMID
      LPBYTE pDelegate = (LPBYTE) pMalloc->Alloc(pSrcItem->cb);
      if( pDelegate == NULL ) return FALSE;
      ::CopyMemory(pDelegate + 4, pSrcItem, pSrcItem->cb);
      m_p = reinterpret_cast<PIDLIST_ABSOLUTE>(pDelegate);
      return TRUE;
   }

   BOOL CreateFromPath(LPCSTR pstrFilename)
   {
      Delete();
      m_p = ::ILCreateFromPathA(pstrFilename);
      return m_p != NULL;
   }

   BOOL CreateFromPath(LPCWSTR pstrFilename)
   {
      Delete();
      m_p = ::ILCreateFromPathW(pstrFilename);
      return m_p != NULL;
   }

   BOOL CreateFromObject(IUnknown* pUnk)
   {
      Delete();
      return ::SHGetIDListFromObject(pUnk, &m_p) == S_OK;
   }

   BOOL CreateFromDisplayName(PCWSTR pszName, IBindCtx* pbc = NULL)
   {
      Delete();
      SFGAOF FlagsOut = 0;
      return ::SHParseDisplayName(pszName, pbc, &m_p, 0, &FlagsOut) == S_OK;
   }

   BOOL CreateFromKnownFolder(REFKNOWNFOLDERID FolderId, DWORD dwFlags = 0, HANDLE hToken = NULL)
   {
      Delete();
      return ::SHGetKnownFolderIDList(FolderId, dwFlags, hToken, &m_p) == S_OK;
   }

   BOOL Copy(PCIDLIST_ABSOLUTE pidl)
   {
      Delete();
      m_p = ILCloneFull(pidl);
      return m_p != NULL;
   }

   BOOL Copy(PCIDLIST_RELATIVE pidl)
   {
      Delete();
      m_p = ILCloneFull((PCIDLIST_ABSOLUTE) pidl);
      return m_p != NULL;
   }

   BOOL Copy(PCUITEMID_CHILD_ARRAY rgpidl, int cidl)
   {
      Delete();
      for( int i = 0; i < cidl; i++ ) m_p = (PIDLIST_ABSOLUTE) ::ILAppendID(m_p, (LPSHITEMID) rgpidl[i], TRUE);
      return m_p != NULL;
   }

   BOOL Append(PCUIDLIST_RELATIVE pidl)
   {
      PIDLIST_ABSOLUTE p = ::ILCombine(m_p, pidl);
      if( m_p != NULL ) ::ILFree(m_p);
      m_p = p;
      return p != NULL;
   }

   BOOL Append(PCUITEMID_CHILD pidl)
   {
      ATLASSERT(pidl);
      if( pidl != NULL ) m_p = (PIDLIST_ABSOLUTE) ::ILAppendID(m_p, (LPSHITEMID) pidl, TRUE);
      return m_p != NULL;
   }

   BOOL Append(const CPidl& pidl)
   {
      return Append((PCUIDLIST_RELATIVE) pidl.m_p);
   }

   BOOL AppendSHItemID(LPSHITEMID pSHItemID, BOOL bAppend = TRUE)
   {
      ATLASSERT(pSHItemID != NULL);
      if( pSHItemID != NULL ) m_p = (PIDLIST_ABSOLUTE) ::ILAppendID(m_p, pSHItemID, bAppend);
      return m_p != NULL;
   }

   BOOL IsEqual(PCIDLIST_ABSOLUTE pidl) const
   {
      ATLASSERT(m_p != NULL);
      return ::ILIsEqual(m_p, pidl);
   }

   BOOL IsSamePIDL(PCIDLIST_ABSOLUTE pidl) const
   {
      IShellFolder* pFolder = NULL;
      if( FAILED( ::SHGetDesktopFolder(&pFolder) ) ) return FALSE;
      HRESULT Hr = pFolder->CompareIDs(0, (PCUIDLIST_RELATIVE) m_p, (PCUIDLIST_RELATIVE) pidl);
      pFolder->Release();
      return (SUCCEEDED(Hr) && HRESULT_CODE(Hr) == 0);
   }

   CPidl& operator =(const CPidl& pidl)
   {
      Copy(pidl.m_p);
      return *this;
   }

   CPidl& operator =(PCIDLIST_ABSOLUTE pidl)
   {
      Copy(pidl);
      return *this;
   }

   bool operator ==(const CPidl& pidl) const
   {
      return IsEqual(pidl.m_p) == TRUE;
   }

   bool operator ==(PCIDLIST_ABSOLUTE pidl) const
   {
      return IsEqual(pidl) == TRUE;
   }

   bool operator !=(const CPidl& pidl) const
   {
      return IsEqual(pidl.m_p) != TRUE;
   }

   bool operator !=(PCIDLIST_ABSOLUTE pidl) const
   {
      return IsEqual(pidl) != TRUE;
   }

   CPidl& operator +=(const CPidl& pidl)
   {
      Append(pidl.m_p);
      return *this;
   }

   CPidl& operator +=(PCUIDLIST_ABSOLUTE pidl)
   {
      Append(pidl);
      return *this;
   }

   CPidl& operator +=(PCUIDLIST_RELATIVE pidl)
   {
      Append(pidl);
      return *this;
   }

   PIDLIST_RELATIVE* operator&()
   {
      return (PIDLIST_RELATIVE*) &m_p;
   }

   operator PIDLIST_RELATIVE() const
   {
      return m_p;
   }

   operator PIDLIST_ABSOLUTE() const
   {
      return m_p;
   }

   bool operator!() const
   {
      return IsEmpty() == TRUE;
   }

   PIDLIST_ABSOLUTE GetData() const
   {
      return m_p;
   }

   PCUIDLIST_RELATIVE GetNextItem() const
   {
      ATLASSERT(m_p != NULL);
      return ::ILGetNext((PCUIDLIST_RELATIVE) m_p);
   }

   BOOL IsNull() const
   {
      return m_p == NULL;
   }

   BOOL IsEmpty() const
   {
      return ILIsEmpty((PCUIDLIST_RELATIVE) m_p);
   }

   UINT GetByteSize() const
   {
      return ::ILGetSize((PCUIDLIST_RELATIVE) m_p);
   }

   BOOL RemoveLastItem()
   {
      ATLASSERT(m_p != NULL);
      return ::ILRemoveLastID((PUIDLIST_RELATIVE) m_p);
   }

   BOOL IsParent(PCIDLIST_ABSOLUTE pidl, BOOL bImmediate = TRUE) const
   {
      ATLASSERT(m_p != NULL);
      return ::ILIsParent(m_p, pidl, bImmediate);
   }

   PUITEMID_CHILD GetLastItem() const
   {
      ATLASSERT(m_p != NULL);
      return ::ILFindLastID((PCUIDLIST_RELATIVE) m_p);
   }

   // Returns a pointer to the pidl(s) beginning at the position.
   // Actual return-type is PUIDLIST_RELATIVE!
   PUITEMID_CHILD GetItem(int iIndex) const
   {
      if( iIndex < 0 ) return NULL;
      if( ::ILIsEmpty(m_p) ) return NULL;
      if( iIndex == 0 ) return (PUITEMID_CHILD) m_p;
      PUIDLIST_RELATIVE pidlItem = ::ILGetNext((PCUIDLIST_RELATIVE) m_p);
      while( !::ILIsEmpty(pidlItem) ) {
         if( --iIndex <= 0 ) return (PUITEMID_CHILD) pidlItem;
         pidlItem = ::ILGetNext(pidlItem);
      }
      return NULL;
   }

   HRESULT CopyTo(PIDLIST_ABSOLUTE* ppPidl) const
   {
      ATLASSERT(ppPidl != NULL);
      ATLASSERT(*ppPidl == NULL);
      *ppPidl = ILCloneFull(m_p);
      return S_OK;
   }

   BOOL IsChild() const
   {
      return ILIsChild((PCUIDLIST_RELATIVE) m_p);
   }

   PUIDLIST_RELATIVE FindChild(PCIDLIST_ABSOLUTE pidl) const
   {
      ATLASSERT(m_p != NULL);
      return ::ILFindChild(m_p, pidl);
   }

   UINT GetItemCount() const
   {
      if( IsEmpty() ) return 0;
      UINT nCount = 0;
      PCUIDLIST_RELATIVE pidlItem = m_p;
      while( (pidlItem = ::ILGetNext(pidlItem)) != NULL ) nCount++;
      return nCount;
   }

   void Attach(PIDLIST_ABSOLUTE pidl)
   {
      Delete();
      m_p = pidl;
   }

   void Attach(PIDLIST_RELATIVE pidl)
   {
      Delete();
      m_p = (PIDLIST_ABSOLUTE) pidl;
   }

   PIDLIST_ABSOLUTE Detach()
   {
      PIDLIST_ABSOLUTE pidlItem = m_p;
      m_p = NULL;
      return pidlItem;
   }

   BOOL IsAligned() const
   {
      return ILIsAligned((PCUIDLIST_RELATIVE) m_p);
   }

   PITEMID_CHILD CloneItem(int iIndex)
   {
      PUIDLIST_RELATIVE pidlItem = (PUIDLIST_RELATIVE) m_p;
      while( --iIndex >= 0 && pidlItem != NULL ) pidlItem = ::ILGetNext(pidlItem);
      if( pidlItem != NULL ) return ILCloneChild( (PUITEMID_CHILD) pidlItem );
      return NULL;
   }

   HRESULT LoadFromStream(IStream* pStream)
   {
      ATLASSERT(pStream != NULL);
      Delete();
      return ::ILLoadFromStream(pStream, (PIDLIST_RELATIVE*) &m_p);
   }

   HRESULT SaveToStream(IStream* pStream)
   {
      ATLASSERT(pStream != NULL);
      return ::ILSaveToStream(pStream, (PIDLIST_RELATIVE) m_p);
   }

   HRESULT GetName(SIGDN sigdnName, LPWSTR* ppszName)
   {
      ATLASSERT(m_p != NULL);
      return ::SHGetNameFromIDList(m_p, sigdnName, ppszName);
   }

   friend CPidl __stdcall operator +(const CPidl& string1, const CPidl& string2);
};

// CPidl "operator +" function

inline CPidl __stdcall operator +(const CPidl& pidl1, const CPidl& pidl2)
{
   CPidl pidl(pidl1);
   pidl.Append(pidl2);
   return pidl;
}

// CPidl helpers

inline VOID SHChangeNotify(LONG wEventId, UINT uFlags, const CPidl& pidl)
{
   ATLASSERT((uFlags & SHCNF_TYPE) == SHCNF_IDLIST);
   ATLASSERT(!pidl.IsNull());
   ::SHChangeNotify(wEventId, uFlags, pidl.GetData(), NULL);
}

inline VOID SHChangeNotify(LONG wEventId, UINT uFlags, const CPidl& pidl1, const CPidl& pidl2)
{
   ATLASSERT((uFlags & SHCNF_TYPE) == SHCNF_IDLIST);
   ATLASSERT(!pidl1.IsNull() && !pidl2.IsNull());
   ::SHChangeNotify(wEventId, uFlags, pidl1.GetData(), pidl2.GetData());
}


/////////////////////////////////////////////////////////////////////////////
// CPidlEnum

class ATL_NO_VTABLE CPidlEnum : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public IEnumIDList
{
public:
   CPidl m_pidl;
   PCUIDLIST_RELATIVE m_pCur;
   ULONG m_lCount; 
   ULONG m_lPos;

   // Constructor
   
   CPidlEnum() : m_lCount(0), m_lPos(0), m_pCur(NULL)
   {
   }

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CPidlEnum)
   COM_INTERFACE_ENTRY(IEnumIDList)
END_COM_MAP()

   // Operations
   
   HRESULT Init(PCIDLIST_ABSOLUTE pidl)
   {      
      m_pidl.Copy(pidl);
      m_lCount = m_pidl.GetItemCount();
      m_lPos = 0;
      m_pCur = m_pidl;
      return S_OK;
   }

   // IEnumIDList
   
   STDMETHOD(Next)(ULONG celt, PITEMID_CHILD* rgelt, ULONG* pceltFetched)
   { 
      ATLTRACE2(atlTraceCOM, 0, _T("IEnumIDList::Next\n"));
      *rgelt = NULL; 
      if( pceltFetched != NULL ) *pceltFetched = 0;
      if( pceltFetched == NULL && celt != 1 ) return E_POINTER;
      ULONG nCount = 0;
      while( m_lPos < m_lCount && nCount < celt ) {
         rgelt[nCount++] = ::ILCloneFirst(m_pCur);
         m_pCur = ::ILGetNext(m_pCur);
         m_lPos++;
      }
      if( pceltFetched != NULL ) *pceltFetched = nCount;
      return celt == nCount ? S_OK : S_FALSE;
   }

   STDMETHOD(Reset)(void)
   { 
      ATLTRACE2(atlTraceCOM, 0, _T("IEnumIDList::Reset\n"));
      m_lPos = 0;
      m_pCur = m_pidl;
      return S_OK; 
   }

   STDMETHOD(Skip)(ULONG celt)
   {
      ATLTRACE2(atlTraceCOM, 0, _T("IEnumIDList::Skip\n"));
      ULONG nCount = 0;
      while( m_lPos < m_lCount && nCount < celt ) {
         m_pCur = ::ILGetNext(m_pCur);
         m_lPos++; nCount++;
      }
      return nCount == celt ? S_OK : S_FALSE;
   }

   STDMETHOD(Clone)(IEnumIDList** /*ppEnum*/)
   {
      ATLTRACENOTIMPL(_T("IEnumIDList::Clone"));
   }
};


//////////////////////////////////////////////////////////////////////////////
// CComEnum helpers

template<>
class _Copy<STATSTG>
{
public:
   static HRESULT copy(STATSTG* p1, const STATSTG* p2)
   {
      ATLENSURE(p1 != NULL && p2 != NULL);
      *p1 = *p2;
      if( p2->pwcsName == NULL ) return S_OK;
      ULONG len = ocslen(p2->pwcsName) + 1;
      p1->pwcsName = (LPOLESTR) ::ATL::AtlCoTaskMemCAlloc(len, (ULONG) sizeof(OLECHAR));
      if( p1->pwcsName == NULL ) return E_OUTOFMEMORY;
      if( ocscpy_s(p1->pwcsName, len, p2->pwcsName) == 0 ) return E_FAIL;
      return S_OK;
   }
   static void init(STATSTG* p) { p->pwcsName = NULL; }
   static void destroy(STATSTG* p) { ::CoTaskMemFree(p->pwcsName); }
};


/////////////////////////////////////////////////////////////////////////////
// CFileSystemBindData

class ATL_NO_VTABLE CFileSystemBindData : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public IFileSystemBindData
{
public:
   WIN32_FIND_DATAW m_wfd;

   // Constructor

   CFileSystemBindData()
   {
      ::ZeroMemory(&m_wfd, sizeof(m_wfd));
   }

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CFileSystemBindData)
   COM_INTERFACE_ENTRY(IFileSystemBindData)
END_COM_MAP()

   // IFileSystemBindData

   STDMETHOD(SetFindData)(const WIN32_FIND_DATAW* pfd)
   {
      m_wfd = *pfd;
      return S_OK;
   }

   STDMETHOD(GetFindData)(WIN32_FIND_DATAW* pfd)
   {
      *pfd = m_wfd;
      return S_OK;
   }
};


//////////////////////////////////////////////////////////////////////////////
// IShellFolderViewCBImpl

template< class T >
class ATL_NO_VTABLE IShellFolderViewCBImpl : public IShellFolderViewCB
{
public:
   STDMETHOD(MessageSFVCB)(UINT uMsg, WPARAM wParam, LPARAM lParam)
   {
      ATLTRACE2(atlTraceCOM, 0, _T("IShellFolderViewCB::MessageSFVCB  msg=%ld wp=0x%X lp=0x%X\n"), uMsg, wParam, lParam);
      LRESULT lResult = 0;
      T* pT = static_cast<T*>(this); pT;
      BOOL bResult = pT->ProcessWindowMessage(NULL, uMsg, wParam, lParam, lResult, 0);
      return bResult ? (HRESULT) lResult : E_NOTIMPL;
   }
};


///////////////////////////////////////////////////////////////////////////////
// Propertykey

#ifndef _PROPERTYKEY_EQUALITY_OPERATORS_
#define _PROPERTYKEY_EQUALITY_OPERATORS_

inline int operator ==(REFPROPERTYKEY pkeyOne, REFPROPERTYKEY pkeyOther)
{
    return IsEqualPropertyKey(pkeyOne, pkeyOther);
}

inline int operator !=(REFPROPERTYKEY pkeyOne, REFPROPERTYKEY pkeyOther)
{
    return !(pkeyOne == pkeyOther);
}

#endif // _PROPERTYKEY_EQUALITY_OPERATORS_


///////////////////////////////////////////////////////////////////////////////
// CComPropVariant

class CComPropVariant : public tagPROPVARIANT
{
public:
   CComPropVariant()
   {
      ::PropVariantInit(this);
   }

   ~CComPropVariant()
   {
      Clear();
   }

   CComPropVariant(const PROPVARIANT& varSrc)
   {
      vt = VT_EMPTY;
      InternalCopy(&varSrc);
   }

   CComPropVariant(const CComPropVariant& varSrc)
   {
      vt = VT_EMPTY;
      InternalCopy(&varSrc);
   }

   CComPropVariant(const VARIANT& varSrc)
   {
      vt = VT_EMPTY;
      ::VariantToPropVariant(&varSrc, this);
   }

   CComPropVariant(const CComVariant& varSrc)
   {
      vt = VT_EMPTY;
      ::VariantToPropVariant(&varSrc, this);
   }

   CComPropVariant(bool bSrc)
   {
      ::InitPropVariantFromBoolean(bSrc ? TRUE : FALSE, this);
   }

   CComPropVariant(BYTE iSrc)
   {
      vt = VT_UI1;
      bVal = iSrc;
   }

   CComPropVariant(SHORT iSrc)
   {
      ::InitPropVariantFromInt16(iSrc, this);
   }

   CComPropVariant(USHORT wSrc)
   {
      ::InitPropVariantFromUInt16(wSrc, this);
   }

   CComPropVariant(LONG lSrc)
   {
      ::InitPropVariantFromInt32(lSrc, this);
   }

   CComPropVariant(ULONG ulSrc)
   {
      ::InitPropVariantFromUInt32(ulSrc, this);
   }

   CComPropVariant(INT iSrc)
   {
      ::InitPropVariantFromInt32(iSrc, this);
   }

   CComPropVariant(UINT uiSrc)
   {
      ::InitPropVariantFromUInt32(uiSrc, this);
   }

   CComPropVariant(LONGLONG llSrc)
   {
      ::InitPropVariantFromInt64(llSrc, this);
   }   

   CComPropVariant(ULONGLONG ullSrc)
   {
      ::InitPropVariantFromUInt64(ullSrc, this);
   }   

   CComPropVariant(DOUBLE dblSrc, VARTYPE vtSrc = VT_R8)
   {
      vt = vtSrc;
      dblVal = dblSrc;
   }

   CComPropVariant(const FILETIME ftSrc)
   {
      if( FAILED( ::InitPropVariantFromFileTime(&ftSrc, this) ) ) vt = VT_EMPTY;
   }

   CComPropVariant(REFCLSID clsidSrc)
   {
      if( FAILED( ::InitPropVariantFromCLSID(clsidSrc, this) ) ) vt = VT_EMPTY;
   }

   CComPropVariant(BSTR bstrSrc)
   {
      if( FAILED( ::InitPropVariantFromString(bstrSrc, this) ) ) vt = VT_EMPTY;
   }

   CComPropVariant(LPCWSTR pwstrSrc)
   {
      if( FAILED( ::InitPropVariantFromString(pwstrSrc, this) ) ) vt = VT_EMPTY;
   }

   CComPropVariant(LPCVOID pBuffer, UINT cb)
   {
      if( FAILED( ::InitPropVariantFromBuffer(pBuffer, cb, this) ) ) vt = VT_EMPTY;
   }

   bool operator ==(const PROPVARIANT& varSrc) const
   {
      if( vt == VT_NULL && varSrc.vt == VT_NULL ) return true;
      if( vt != varSrc.vt ) return false;
      return ::PropVariantCompare(*this, varSrc) == 0;
   }

   bool operator !=(const PROPVARIANT& varSrc) const
   {
      return !operator==(varSrc);
   }

   int CompareTo(const PROPVARIANT& varSrc, PROPVAR_COMPARE_UNIT Unit = PVCU_DEFAULT, PROPVAR_COMPARE_FLAGS Flags = PVCF_DEFAULT) const
   {
      return ::PropVariantCompareEx(*this, varSrc, Unit, Flags);
   }

   CComPropVariant& operator=(const PROPVARIANT& varSrc)
   {
      if( &varSrc != this ) InternalCopy(&varSrc);
      return *this;
   }

   CComPropVariant& operator=(const CComPropVariant& varSrc)
   {
      if( &varSrc != this ) InternalCopy(&varSrc);
      return *this;
   }

   CComPropVariant& operator=(const VARIANT& varSrc)
   {
      Clear();
      ::VariantToPropVariant(&varSrc, this);
      return *this;
   }

   CComPropVariant& operator=(const CComVariant& varSrc)
   {
      Clear();
      ::VariantToPropVariant(&varSrc, this);
      return *this;
   }

   CComPropVariant& operator=(bool bSrc)
   {
      Clear();
      ::InitPropVariantFromBoolean(bSrc ? TRUE : FALSE, this);
      return *this;
   }

   CComPropVariant& operator=(BYTE iSrc)
   {
      Clear();
      vt = VT_UI1;
      bVal = iSrc;
      return *this;
   }

   CComPropVariant& operator=(SHORT iSrc)
   {
      Clear();
      ::InitPropVariantFromInt16(iSrc, this);
      return *this;
   }

   CComPropVariant& operator=(USHORT wSrc)
   {
      Clear();
      ::InitPropVariantFromUInt16(wSrc, this);
      return *this;
   }

   CComPropVariant& operator=(LONG lSrc)
   {
      Clear();
      ::InitPropVariantFromInt32(lSrc, this);
      return *this;
   }

   CComPropVariant& operator=(ULONG ulSrc)
   {
      Clear();
      ::InitPropVariantFromUInt32(ulSrc, this);
      return *this;
   }

   CComPropVariant& operator=(INT iSrc)
   {
      Clear();
      ::InitPropVariantFromInt32(iSrc, this);
      return *this;
   }

   CComPropVariant& operator=(UINT uiSrc)
   {
      Clear();
      ::InitPropVariantFromUInt32(uiSrc, this);
      return *this;
   }

   CComPropVariant& operator=(LONGLONG llSrc)
   {
      Clear();
      ::InitPropVariantFromInt64(llSrc, this);
      return *this;
   }

   CComPropVariant& operator=(ULONGLONG ullSrc)
   {
      Clear();
      ::InitPropVariantFromUInt64(ullSrc, this);
      return *this;
   }

   CComPropVariant& operator=(DOUBLE dblSrc)
   {
      Clear();
      ::InitPropVariantFromDouble(dblSrc, this);
      return *this;
   }

   CComPropVariant& operator=(BSTR bstrSrc)
   {
      Clear();
      ::InitPropVariantFromString(bstrSrc, this);
      return *this;
   }

   CComPropVariant& operator=(LPCWSTR pwstrSrc)
   {
      Clear();
      ::InitPropVariantFromString(pwstrSrc, this);
      return *this;
   }

   CComPropVariant& operator=(const FILETIME ftSrc)
   {
      Clear();
      ::InitPropVariantFromFileTime(&ftSrc, this);
      return *this;
   }

   CComPropVariant& operator=(CLSID clsidSrc)
   {
      Clear();
      ::InitPropVariantFromCLSID(clsidSrc, this);
      return *this;
   }

   HRESULT SetByScode(SCODE scSrc, VARTYPE vtSrc = VT_ERROR)
   {
      Clear();
      vt = vtSrc;
      scode = scSrc;
      return S_OK;
   }

   HRESULT SetByBuffer(LPCVOID pBuffer, UINT cb)
   {
      HRESULT Hr = Clear();
      if( FAILED(Hr) ) return Hr;
      return ::InitPropVariantFromBuffer(pBuffer, cb, this);
   }

   HRESULT SetByStrRet(STRRET* pStrRet, PCUITEMID_CHILD pidl)
   {
      HRESULT Hr = Clear();
      if( FAILED(Hr) ) return Hr;
      return ::InitPropVariantFromStrRet(pStrRet, pidl, this);
   }

   BOOL LoadString(UINT nID)
   {
      Clear();
      return ::InitPropVariantFromResource(_pModule->GetResourceInstance(), nID, this) == S_OK;
   }

   BOOL LoadString(HINSTANCE hInstance, UINT nID)
   {
      Clear();
      return ::InitPropVariantFromResource(hInstance, nID, this) == S_OK;
   }

   HRESULT CoerceToCanonicalValue(REFPROPERTYKEY pkey)
   {
      return ::PSCoerceToCanonicalValue(pkey, this);
   }

   HRESULT Attach(PROPVARIANT* pSrc)
   {
      if( pSrc == NULL ) return E_INVALIDARG;
      HRESULT Hr = Clear();
      if( FAILED(Hr) ) return Hr;
      Checked::memcpy_s(static_cast<PROPVARIANT*>(this), sizeof(PROPVARIANT), pSrc, sizeof(PROPVARIANT));
      pSrc->vt = VT_EMPTY;
      return S_OK;
   }

   HRESULT Detach(PROPVARIANT* pDest)
   {
      ATLASSERT(pDest != NULL);
      if( pDest == NULL ) return E_POINTER;
      HRESULT Hr = ::PropVariantClear(pDest);
      if( FAILED(Hr) ) return Hr;
      Checked::memcpy_s(pDest, sizeof(PROPVARIANT), static_cast<PROPVARIANT*>(this), sizeof(PROPVARIANT));
      vt = VT_EMPTY;
      return S_OK;
   }

   HRESULT Clear()
   {
      return ::PropVariantClear(this);
   }

   HRESULT CopyTo(PROPVARIANT* pDest) const
   {
      return ::PropVariantCopy(pDest, this);
   }

   HRESULT CopyTo(VARIANT* pDest) const
   {
      ATLASSERT(pDest != NULL);
      ATLASSERT(pDest->vt == VT_EMPTY);
      return ::PropVariantToVariant(this, pDest);
   }

   HRESULT CopyTo(BSTR* pDest) const
   {
      ATLASSERT(pDest != NULL);
      ATLASSERT(*pDest == NULL);
      return ::PropVariantToBSTR(*this, pDest);
   }

   HRESULT ChangeType(VARTYPE vtNew, const PROPVARIANT* pSrc = NULL, PROPVAR_CHANGE_FLAGS Flags = PVCHF_DEFAULT)
   {
      if( pSrc != NULL && pSrc != this ) return ::PropVariantChangeType(this, *pSrc, Flags, vtNew);
      PROPVARIANT vDest = { 0 };
      HRESULT Hr = ::PropVariantChangeType(&vDest, *this, Flags, vtNew);
      if( SUCCEEDED(Hr) ) Hr = Attach(&vDest);
      return Hr;
   }

   HRESULT LoadFromStream(IStream* pStream)
   {
      ATLASSERT(pStream != NULL);
      Clear();
      ULONG cb = 0, cbRead = 0;
      HRESULT Hr = pStream->Read(&cb, sizeof(cb), &cbRead);
      if( SUCCEEDED(Hr) && cb > 0 ) {
         SERIALIZEDPROPERTYVALUE* pSPV = (SERIALIZEDPROPERTYVALUE*) malloc(cb);
         Hr = pStream->Read(pSPV, cb, &cbRead);
         if( SUCCEEDED(Hr) ) Hr = ::StgDeserializePropVariant(pSPV, cb, this);
         free(pSPV);
      }
      return Hr;
   }

   HRESULT SaveToStream(IStream* pStream)
   {
      ATLASSERT(pStream != NULL);
      ULONG cb = 0;
      SERIALIZEDPROPERTYVALUE* pSPV = NULL;
      HRESULT Hr = ::StgSerializePropVariant(this, &pSPV, &cb);
      if( FAILED(Hr) ) return Hr;
      ULONG cbWritten = 0;
      Hr = pStream->Write(&cb, sizeof(cb), &cbWritten);
      if( SUCCEEDED(Hr) ) Hr = pStream->Write(pSPV, cb, &cbWritten);
      ::CoTaskMemFree(pSPV);
      return Hr;
   }

   // Implementation

   HRESULT InternalCopy(const PROPVARIANT* pSrc)
   {
      Clear();
      HRESULT Hr = ::PropVariantCopy(this, pSrc);
      if( FAILED(Hr) ) vt = VT_ERROR, scode = Hr;
      return Hr;
   }
};

