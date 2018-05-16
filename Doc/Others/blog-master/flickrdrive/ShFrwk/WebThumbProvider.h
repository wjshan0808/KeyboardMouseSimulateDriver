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
// CWebThumbnailProvider

class ATL_NO_VTABLE CWebThumbnailProvider : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public IThumbnailProvider
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CAxWindow m_wndPreview;
   CComBSTR m_bstrURL;

   BEGIN_COM_MAP(CWebThumbnailProvider)
      COM_INTERFACE_ENTRY(IThumbnailProvider)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, LPCTSTR pstrURL);
   void FinalRelease();

   // IThumbnailProvider

   STDMETHOD(GetThumbnail)(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);

   // Implementation

   HRESULT _WaitForIeBrowser(IWebBrowser2* pBrowser, DWORD dwTimeoutMS);
};

