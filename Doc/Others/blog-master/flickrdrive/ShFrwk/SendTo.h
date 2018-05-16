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
// CSendTo

class ATL_NO_VTABLE CSendTo : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public CComCoClass<CSendTo, &CLSID_SendTo>,
   public IDropTarget,
   public IPersistFile
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CComPtr<IDataObject> m_spDataObject;
   bool m_bAcceptFmt;

   BEGIN_COM_MAP(CSendTo)
      COM_INTERFACE_ENTRY(IDropTarget)
      COM_INTERFACE_ENTRY(IPersistFile)
   END_COM_MAP()

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   CSendTo();

   static HRESULT WINAPI UpdateRegistry(BOOL bRegister) throw();

   HRESULT Init(CShellFolder* pFolder);

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

   // Implementation

   bool _AcceptDropFormat(IDataObject* pDataObj) const;
   DWORD _QueryDrop(DWORD dwKeyState, DWORD dwEffect) const;
   DWORD _GetDropEffectFromKeyState(DWORD dwKeyState) const;
};
