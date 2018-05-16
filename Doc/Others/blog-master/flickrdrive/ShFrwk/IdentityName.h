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
// CIdentityName

class ATL_NO_VTABLE CIdentityName :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IIdentityName
{
public:
   TCHAR m_szFilename[MAX_PATH];

   BEGIN_COM_MAP(CIdentityName)
      COM_INTERFACE_ENTRY(IIdentityName)
      COM_INTERFACE_ENTRY(IRelatedItem)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl);

   // IIdentityName

   STDMETHOD(GetItemIDList)(PIDLIST_ABSOLUTE* ppidl);
   STDMETHOD(GetItem)(IShellItem** ppsi);
};

