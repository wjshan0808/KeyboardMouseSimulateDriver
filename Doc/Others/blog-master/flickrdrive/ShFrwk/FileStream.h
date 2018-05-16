/////////////////////////////////////////////////////////////////////////////
// Shell Extension classes
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

class CShellFolder;


///////////////////////////////////////////////////////////////////////////////
// CFileStream

class ATL_NO_VTABLE CFileStream : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public IStream
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CNseFileStream* m_pStream;
   CPidl m_pidlItem;
   bool m_bNeedCommit;

   CFileStream();
   void FinalRelease();

   BEGIN_COM_MAP(CFileStream)
      COM_INTERFACE_ENTRY(IStream)
      COM_INTERFACE_ENTRY(ISequentialStream)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, PCUITEMID_CHILD pidlItem);

   // IStream

   STDMETHOD(Read)(LPVOID pv, ULONG cb, ULONG* pcbRead);
   STDMETHOD(Write)(LPCVOID pv, ULONG cb, ULONG* pcbWritten);
   STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition);
   STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
   STDMETHOD(CopyTo)(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten);
   STDMETHOD(Commit)(DWORD grfCommitFlags);
   STDMETHOD(Revert)();
   STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
   STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
   STDMETHOD(Stat)(STATSTG* pstatstg, DWORD grfStatFlag);
   STDMETHOD(Clone)(IStream** ppstm);

   // Implementation

   HRESULT _OpenFileNow(UINT uAccess);
};

