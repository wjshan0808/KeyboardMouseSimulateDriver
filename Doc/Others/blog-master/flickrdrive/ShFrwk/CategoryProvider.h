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


//////////////////////////////////////////////////////////////////////////////
// CCategoryProvider

class ATL_NO_VTABLE CCategoryProvider : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public ICategoryProvider
{
public:
   CRefPtr<CShellFolder> m_spFolder;

   BEGIN_COM_MAP(CCategoryProvider)
      COM_INTERFACE_ENTRY(ICategoryProvider)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder);

   // ICategoryProvider

   STDMETHOD(CanCategorizeOnSCID)(const SHCOLUMNID* pscid);
   STDMETHOD(GetDefaultCategory)(GUID* pguid, SHCOLUMNID* pscid);   
   STDMETHOD(GetCategoryForSCID)(const SHCOLUMNID* pscid, GUID* pguid);
   STDMETHOD(EnumCategories)(IEnumGUID** penum);
   STDMETHOD(GetCategoryName)(const GUID* pguid, LPWSTR pszName, UINT cch);
   STDMETHOD(CreateCategory)(const GUID* pguid, REFIID riid, LPVOID* ppv);
};

