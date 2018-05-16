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
// CEnumIDList

class ATL_NO_VTABLE CEnumIDList :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IEnumIDList
{
public:
   CRefPtr<CShellFolder> m_spFolder;             // Reference to the folder
   HWND m_hWnd;                                  // Reference to the owner window
   SHCONTF m_grfFlags;                           // Enum Flags
   CNseItemArray m_aChildren;                    // List of the child items
   CPidl m_pidl;                                 // Child items from m_aChildren cast to PIDL
   PCUIDLIST_RELATIVE m_pCur;                    // Current enum position
   ULONG m_uPos;                                 // Current enum position
   ULONG m_uCount;                               // Number of items in list

   CEnumIDList();

   BEGIN_COM_MAP(CEnumIDList)
      COM_INTERFACE_ENTRY(IEnumIDList)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, HWND hwnd, SHCONTF grfFlags);

   // IEnumIDList

   STDMETHOD(Next)(ULONG celt, PITEMID_CHILD* rgelt, ULONG* pceltFetched);
   STDMETHOD(Reset)();
   STDMETHOD(Skip)(ULONG celt);
   STDMETHOD(Clone)(IEnumIDList** ppEnum);

   // Implementation

   HRESULT _PopulateList();
};

