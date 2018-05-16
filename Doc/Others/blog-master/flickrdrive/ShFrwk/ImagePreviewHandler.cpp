
#include "stdafx.h"

#include "ImagePreviewHandler.h"
#include "ShellFolder.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")


///////////////////////////////////////////////////////////////////////////////
// CImagePreviewHandler

CImagePreviewHandler::CImagePreviewHandler() : m_hBitmap(NULL)
{
   ATLTRACE(L"CImagePreviewHandler::CImagePreviewHandler\n");
}

HRESULT WINAPI CImagePreviewHandler::UpdateRegistry(BOOL bRegister) throw()
{
   // COM registration is done by CShellFolder instance.
   return S_OK;
}

void CImagePreviewHandler::FinalRelease()
{
   ATLTRACE(L"CImagePreviewHandler::FinalRelease\n");
   Unload();
}

// IPreviewHandler

STDMETHODIMP CImagePreviewHandler::SetWindow(HWND hwnd, const RECT* prc)
{
   ATLTRACE(L"CImagePreviewHandler::SetWindow\n");
   m_wndParent = hwnd;
   m_rcParent = *prc;
   if( m_wndPreview.IsWindow() ) {
      m_wndPreview.SetParent(hwnd);
      m_wndPreview.MoveWindow(prc);
   }
   return S_OK;
}

STDMETHODIMP CImagePreviewHandler::SetRect(const RECT* prc)
{
   ATLTRACE(L"CImagePreviewHandler::SetRect\n");
   m_rcParent = *prc;
   if( m_wndPreview.IsWindow() ) m_wndPreview.MoveWindow(prc);
   return S_OK;
}

STDMETHODIMP CImagePreviewHandler::DoPreview()
{
   ATLTRACE(L"CImagePreviewHandler::DoPreview\n");
   if( m_hBitmap != NULL ) return E_UNEXPECTED;
   if( m_wndPreview.IsWindow() ) return E_UNEXPECTED;
   if( !m_wndParent.IsWindow() ) return E_UNEXPECTED;
   // Create a HBITMAP from stream data
   using namespace Gdiplus;
   HRESULT Hr = E_FAIL;
   ULONG_PTR GdipToken = 0;
   GdiplusStartupInput GdipInput;
   GdiplusStartupOutput GdipOutput;
   if( GdiplusStartup(&GdipToken, &GdipInput, &GdipOutput) == Ok ) {
      Bitmap* pBitmap = Bitmap::FromStream(m_spStream, false);
      if( pBitmap != NULL ) {
         Color color(32, 32, 32);
         if( pBitmap->GetHBITMAP(color, &m_hBitmap) == Ok ) Hr = S_OK;
         delete pBitmap;
      }
      GdiplusShutdown(GdipToken);
   }
   if( FAILED(Hr) ) return Hr;
   // Create STATIC control
   m_wndPreview.Create(WC_STATIC, m_wndParent, m_rcParent, _T(""), WS_CHILD | WS_DISABLED | SS_BITMAP | SS_REALSIZEIMAGE | SS_CENTERIMAGE);
   if( !m_wndPreview.IsWindow() ) return E_FAIL;
   m_wndPreview.SendMessage(STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) m_hBitmap);
   m_wndPreview.ShowWindow(SW_SHOWNOACTIVATE);
   return S_OK;
}

STDMETHODIMP CImagePreviewHandler::Unload()
{
   ATLTRACE(L"CImagePreviewHandler::Unload\n");
   if( m_wndPreview.IsWindow() ) m_wndPreview.DestroyWindow();
   if( m_hBitmap != NULL ) ::DeleteObject(m_hBitmap); m_hBitmap = NULL;
   m_spStream.Release();
   return S_OK;
}

STDMETHODIMP CImagePreviewHandler::SetFocus()
{
   ATLTRACE(L"CImagePreviewHandler::SetFocus\n");
   if( !m_wndPreview.IsWindow() ) return S_FALSE;
   m_wndPreview.SetFocus();
   return S_OK;
}

STDMETHODIMP CImagePreviewHandler::QueryFocus(HWND* phwnd)
{
   ATLTRACE(L"CImagePreviewHandler::QueryFocus\n");
   *phwnd = ::GetFocus();
   return *phwnd != NULL ? S_OK : AtlHresultFromLastError();
}

STDMETHODIMP CImagePreviewHandler::TranslateAccelerator(MSG* pMsg)
{
   ATLTRACE(L"CImagePreviewHandler::TranslateAccelerator\n");
   CComQIPtr<IPreviewHandlerFrame> spFrame = m_spUnkSite;
   if( spFrame != NULL ) return spFrame->TranslateAccelerator(pMsg);
   return S_FALSE;
}

// IInitializeWithStream

STDMETHODIMP CImagePreviewHandler::Initialize(IStream* pStream, DWORD grfMode)
{
   ATLTRACE(L"CImagePreviewHandler::Initialize\n");
   m_spStream = pStream;
   return S_OK;
}

// IOleWindow

STDMETHODIMP CImagePreviewHandler::GetWindow(HWND* phwnd)
{
   ATLTRACE(L"CImagePreviewHandler::GetWindow\n");
   *phwnd = m_wndParent;
   return S_OK;
}

STDMETHODIMP CImagePreviewHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
   ATLTRACENOTIMPL(L"CImagePreviewHandler::ContextSensitiveHelp");
}


OBJECT_ENTRY_AUTO(CLSID_Preview, CImagePreviewHandler)
