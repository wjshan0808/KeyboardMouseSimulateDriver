
#include "stdafx.h"

#include "WebPreviewHandler.h"
#include "ShellFolder.h"

#include <mshtml.h>


///////////////////////////////////////////////////////////////////////////////
// CWebPreviewHandler

CWebPreviewHandler::CWebPreviewHandler()
{
   ATLTRACE(L"CWebPreviewHandler::CWebPreviewHandler\n");
}

HRESULT WINAPI CWebPreviewHandler::UpdateRegistry(BOOL bRegister) throw()
{
   // COM registration is done by CShellFolder instance.
   return S_OK;
}

void CWebPreviewHandler::FinalRelease()
{
   ATLTRACE(L"CWebPreviewHandler::FinalRelease\n");
   if( m_wndPreview.IsWindow() ) m_wndPreview.DestroyWindow();
}

// IPreviewHandler

STDMETHODIMP CWebPreviewHandler::SetWindow(HWND hwnd, const RECT* prc)
{
   ATLTRACE(L"CWebPreviewHandler::SetWindow\n");
   m_wndParent = hwnd;
   m_rcParent = *prc;
   if( m_wndPreview.IsWindow() ) {
      m_wndPreview.SetParent(hwnd);
      m_wndPreview.MoveWindow(prc);
   }
   return S_OK;
}

STDMETHODIMP CWebPreviewHandler::SetRect(const RECT* prc)
{
   ATLTRACE(L"CWebPreviewHandler::SetRect\n");
   m_rcParent = *prc;
   if( m_wndPreview.IsWindow() ) m_wndPreview.MoveWindow(prc);
   return S_OK;
}

STDMETHODIMP CWebPreviewHandler::DoPreview()
{
   ATLTRACE(L"CWebPreviewHandler::DoPreview\n");
   if( m_wndPreview.IsWindow() ) return E_UNEXPECTED;
   if( !m_wndParent.IsWindow() ) return E_UNEXPECTED;
   // Create CLSID_WebBrowser control
   AtlAxWinInit();
   m_wndPreview.Create(m_wndParent, m_rcParent, _T("{8856F961-340A-11D0-A96B-00C04FD705A2}"), WS_CHILD | WS_DISABLED);
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
   // Adjust browser window settings
   spBrowser->put_RegisterAsBrowser(VARIANT_FALSE);
   spBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
   CComPtr<IAxWinAmbientDispatch> spHost;
   HRESULT Hr = m_wndPreview.QueryHost(IID_PPV_ARGS(&spHost));
   if( SUCCEEDED(Hr) ) {
      spHost->put_AllowContextMenu(VARIANT_FALSE);
      spHost->put_DocHostFlags(DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DISABLE_HELP_MENU);
   }
   // Display as HTML page.
   // We must first initialize the HTML Document object and then
   // push our stream to the browser.
   CComVariant vEmpty;
   CComVariant vURL = L"about:blank";
   HR( spBrowser->Navigate2(&vURL, &vEmpty, &vEmpty, &vEmpty, &vEmpty) );
   HR( _WaitForIeBrowser(spBrowser, 3000) );
   CComPtr<IDispatch> spDoc;
   HR( spBrowser->get_Document(&spDoc) );
   CComQIPtr<IPersistStreamInit> spPSI = spDoc;
   if( spPSI == NULL ) return E_UNEXPECTED;
   HR( spPSI->InitNew() );
   HR( spPSI->Load(m_spStream) );
   m_wndPreview.ShowWindow(SW_SHOWNOACTIVATE);
   return S_OK;
}

STDMETHODIMP CWebPreviewHandler::Unload()
{
   ATLTRACE(L"CWebPreviewHandler::Unload\n");
   if( m_wndPreview.IsWindow() ) m_wndPreview.DestroyWindow();
   return S_OK;
}

STDMETHODIMP CWebPreviewHandler::SetFocus()
{
   ATLTRACE(L"CWebPreviewHandler::SetFocus\n");
   if( m_wndPreview.IsWindow() ) m_wndPreview.SetFocus();
   return S_OK;
}

STDMETHODIMP CWebPreviewHandler::QueryFocus(HWND* phwnd)
{
   ATLTRACE(L"CWebPreviewHandler::QueryFocus\n");
   *phwnd = ::GetFocus();
   return S_OK;
}

STDMETHODIMP CWebPreviewHandler::TranslateAccelerator(MSG* pMsg)
{
   ATLTRACE(L"CWebPreviewHandler::TranslateAccelerator\n");
   // Give OCX a chance to translate this message
   if( !m_wndPreview.IsWindow() ) return E_FAIL;
   if( m_wndPreview.SendMessage(WM_FORWARDMSG, 0, (LPARAM) pMsg) ) return S_OK;
   // Let frame translate this message
   CComQIPtr<IPreviewHandlerFrame> spFrame = m_spUnkSite;
   if( spFrame != NULL ) return spFrame->TranslateAccelerator(pMsg);
   return S_FALSE;
}

// IInitializeWithStream

STDMETHODIMP CWebPreviewHandler::Initialize(IStream* pStream, DWORD grfMode)
{
   ATLTRACE(L"CWebPreviewHandler::Initialize\n");
   m_spStream = pStream;
   return S_OK;
}

// IOleWindow

STDMETHODIMP CWebPreviewHandler::GetWindow(HWND* phwnd)
{
   if( !m_wndPreview.IsWindow() ) return E_FAIL;
   *phwnd = m_wndParent;
   return S_OK;
}

STDMETHODIMP CWebPreviewHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
   ATLTRACENOTIMPL(L"CWebPreviewHandler::ContextSensitiveHelp");
}

// Implementation

HRESULT CWebPreviewHandler::_WaitForIeBrowser(IWebBrowser2* pBrowser, DWORD dwTimeoutMS)
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


OBJECT_ENTRY_AUTO(CLSID_Preview, CWebPreviewHandler)
