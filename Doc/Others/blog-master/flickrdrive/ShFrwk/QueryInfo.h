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
// CQueryInfo

class ATL_NO_VTABLE CQueryInfo : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public IQueryInfo
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CPidl m_pidlItem;

   BEGIN_COM_MAP(CQueryInfo)
      COM_INTERFACE_ENTRY(IQueryInfo)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl);

   // IQueryInfo

   STDMETHOD(GetInfoTip)(DWORD dwFlags, LPWSTR* ppwszTip);
   STDMETHOD(GetInfoFlags)(DWORD* pdwFlags);
};

