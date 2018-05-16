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
// CContextMenu

class ATL_NO_VTABLE CContextMenu : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public CComCoClass<CContextMenu, &CLSID_ContextMenu>,
   public IObjectWithSiteImpl<CContextMenu>,
   public IContextMenu3,
   public IShellExtInit
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CComPtr<IContextMenu3> m_spMaster;

   BEGIN_COM_MAP(CContextMenu)
      COM_INTERFACE_ENTRY(IContextMenu)
      COM_INTERFACE_ENTRY(IContextMenu2)
      COM_INTERFACE_ENTRY(IContextMenu3)
      COM_INTERFACE_ENTRY(IShellExtInit)
      COM_INTERFACE_ENTRY(IObjectWithSite)
   END_COM_MAP()

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   static HRESULT WINAPI UpdateRegistry(BOOL bRegister) throw();

   // IContextMenu

   STDMETHOD(QueryContextMenu)(HMENU hMenu, UINT iIndexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
   STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO pcmi);
   STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, LPUINT, LPSTR pszName, UINT cchMax);

   // IContextMenu2

   STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);
        
   // IContextMenu3

   STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

   // IShellExtInit

   STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObject, HKEY hkeyProgID);
};

