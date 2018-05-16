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
// CImagePreviewHandler

class ATL_NO_VTABLE CImagePreviewHandler : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public CComCoClass<CImagePreviewHandler, &CLSID_Preview>,
   public IObjectWithSiteImpl<CImagePreviewHandler>,
   public IInitializeWithStream,
   public IPreviewHandler,
   public IOleWindow
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CComPtr<IStream> m_spStream;
   CWindow m_wndPreview;
   CWindow m_wndParent;
   HBITMAP m_hBitmap;
   RECT m_rcParent;

   BEGIN_COM_MAP(CImagePreviewHandler)
      COM_INTERFACE_ENTRY(IPreviewHandler)
      COM_INTERFACE_ENTRY(IObjectWithSite)
      COM_INTERFACE_ENTRY(IOleWindow)
      COM_INTERFACE_ENTRY(IInitializeWithStream)
   END_COM_MAP()

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   static HRESULT WINAPI UpdateRegistry(BOOL bRegister) throw();

   CImagePreviewHandler();
   void FinalRelease();

   // IPreviewHandler

   STDMETHOD(SetWindow)(HWND hwnd, const RECT* prc);
   STDMETHOD(SetRect)(const RECT* prc);
   STDMETHOD(DoPreview)();
   STDMETHOD(Unload)();
   STDMETHOD(SetFocus)();
   STDMETHOD(QueryFocus)(HWND* phwnd);
   STDMETHOD(TranslateAccelerator)(MSG* pmsg);

   // IInitializeWithStream

   STDMETHOD(Initialize)(IStream *pstream, DWORD grfMode);

   // IOleWindow

   STDMETHOD(GetWindow)(HWND* phwnd);
   STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
};

