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
// CDropTarget

class ATL_NO_VTABLE CDropTarget : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public CComCoClass<CDropTarget, &CLSID_DropTarget>,
   public IDropTarget,
   public IPersistFile,
   public IShellExtInit
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CComPtr<IDataObject> m_spDataObject;
   HWND m_hwndTarget;
   CPidl m_pidlItem;
   bool m_bAcceptFmt;

   BEGIN_COM_MAP(CDropTarget)
      COM_INTERFACE_ENTRY(IDropTarget)
      COM_INTERFACE_ENTRY(IPersistFile)
      COM_INTERFACE_ENTRY(IShellExtInit)
   END_COM_MAP()

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   CDropTarget();

   static HRESULT WINAPI UpdateRegistry(BOOL bRegister) throw();

   HRESULT Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl, HWND hWnd);

   // IDropTarget

   STDMETHOD(DragEnter)(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);       
   STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);      
   STDMETHOD(DragLeave)(VOID);
   STDMETHOD(Drop)(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

   // IPersist

   STDMETHOD(GetClassID)(CLSID* pClassID);

   // IPersistFile

   STDMETHOD(IsDirty)(VOID);
   STDMETHOD(Load)(LPCOLESTR pwstrFileName, DWORD dwMode);
   STDMETHOD(Save)(LPCOLESTR pwstrFileName, BOOL fRemember);
   STDMETHOD(SaveCompleted)(LPCOLESTR pszFileName);
   STDMETHOD(GetCurFile)(LPOLESTR* ppszFileName);

   // IShellExtInit

   STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObject, HKEY hkeyProgID);

   // Implementation

   bool _AcceptDropFormat(IDataObject* pDataObj) const;
   DWORD _QueryDrop(DWORD dwKeyState, DWORD dwEffect) const;
   DWORD _GetDropEffectFromKeyState(DWORD dwKeyState) const;
};
