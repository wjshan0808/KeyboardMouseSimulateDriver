
#include "stdafx.h"

#include "WebThumbProvider.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CWebThumbnailProvider

HRESULT CWebThumbnailProvider::Init(CShellFolder* pFolder, LPCTSTR pstrURL)
{
   m_spFolder = pFolder;
   m_bstrURL = pstrURL;
   return S_OK;
}

void CWebThumbnailProvider::FinalRelease()
{
   if( m_wndPreview.IsWindow() ) m_wndPreview.DestroyWindow();
}

// IThumbnailProvider

STDMETHODIMP CWebThumbnailProvider::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
   ATLTRACE(L"CWebThumbnailProvider::GetThumbnail  cx=%u\n", cx);
   // Get the thumbnail component
   CComPtr<IThumbnailCapture> spCapture;
   HR( spCapture.CoCreateInstance(L"Shell.ThumbnailExtract.HTML.1") );
   // Create CLSID_WebBrowser control
   if( !m_wndPreview.IsWindow() ) {
      AtlAxWinInit();
      RECT rcWindow = { 0, 0, cx, cx };
      m_wndPreview.Create(NULL, rcWindow, _T("{8856F961-340A-11D0-A96B-00C04FD705A2}"), WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE);
   }
   if( !m_wndPreview.IsWindow() ) return E_FAIL;
   // Get the web-browser control
   CComPtr<IUnknown> spUnk;
   HR( m_wndPreview.QueryControl(IID_PPV_ARGS(&spUnk)) );
   CComQIPtr<IServiceProvider> spSP = spUnk;
   if( spSP == NULL ) return E_UNEXPECTED;
   CComPtr<IWebBrowserApp> spWebApp;
   HR( spSP->QueryService(IID_IWebBrowserApp, &spWebApp) );
   CComQIPtr<IWebBrowser2> spBrowser = spWebApp;
   if( spBrowser == NULL ) return E_UNEXPECTED;
   // Display HTML page
   CComVariant vEmpty;
   CComVariant vURL = m_bstrURL;
   HR( spBrowser->Navigate2(&vURL, &vEmpty, &vEmpty, &vEmpty, &vEmpty) );
   HR( _WaitForIeBrowser(spBrowser, 5000) );
   CComPtr<IDispatch> spDisp;
   HR( spBrowser->get_Document(&spDisp) );
   CComQIPtr<IHTMLDocument2> spDoc = spDisp;
   if( spDoc == NULL ) return E_UNEXPECTED;
   // Generate thumbnail...
   SIZE sizePreview = { cx, cx };
   HR( spCapture->CaptureThumbnail(&sizePreview, spDoc, phbmp) );
   return S_OK;
}

// Implementation

HRESULT CWebThumbnailProvider::_WaitForIeBrowser(IWebBrowser2* pBrowser, DWORD dwTimeoutMS)
{
   if( pBrowser == NULL ) return E_POINTER;
   DWORD dwTick = ::GetTickCount();
   READYSTATE ReadyState = READYSTATE_UNINITIALIZED; 
   do {
      pBrowser->get_ReadyState(&ReadyState);
      ::Sleep(100UL);
      MSG msg = { 0 };
      while( ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
         ::TranslateMessage(&msg);
         ::DispatchMessage(&msg);
      }
      if( ::GetTickCount() - dwTick > dwTimeoutMS ) return E_ABORT;
   } while( ReadyState != READYSTATE_COMPLETE );
   return S_OK;
}
