
#include "stdafx.h"

#include "ImageThumbProvider.h"
#include "ShellFolder.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")


///////////////////////////////////////////////////////////////////////////////
// CImageThumbnailProvider

HRESULT CImageThumbnailProvider::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidlItem)
{
   m_spFolder = pFolder;
   m_pidlItem = pidlItem;
   return S_OK;
}

// IThumbnailProvider

STDMETHODIMP CImageThumbnailProvider::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
   ATLTRACE(L"CImageThumbnailProvider::GetThumbnail  cx=%u\n", cx);
   // The GDI+ will use IStream::Seek() intensively, and since our IStream implementation
   // may not support that, we'll write the stream to a new memory stream first.
   CNseItemPtr spItem = m_spFolder->GenerateChildItem(m_pidlItem.GetItem(0));
   if( spItem == NULL ) return E_FAIL;
   CAutoPtr<CNseFileStream> spSrcStream;
   VFS_STREAM_REASON Reason = { VFS_STREAMTYPE_THUMBNAIL, GENERIC_READ, cx };
   HR( spItem->GetStream(Reason, &spSrcStream.m_p) );
   HR( spSrcStream->Init() );
   CComPtr<IStream> spDstStream;
   HR( ::CreateStreamOnHGlobal(NULL, TRUE, &spDstStream) );
   const ULONG MAX_BUFFER = 8000;
   CAutoVectorPtr<BYTE> buffer( new BYTE[MAX_BUFFER] );
   if( buffer == NULL ) return E_OUTOFMEMORY;
   while( true ) {
      ULONG ulBytesRead = 0;
      ULONG ulBytesWritten = 0;
      HR( spSrcStream->Read(buffer, MAX_BUFFER, ulBytesRead) );
      if( ulBytesRead == 0 ) break;
      HR( spDstStream->Write(buffer, ulBytesRead, &ulBytesWritten) );
   }
   LARGE_INTEGER ullZero = { 0 };
   HR( spDstStream->Seek(ullZero, STREAM_SEEK_SET, NULL) );
   // Now let the GDI+ library create a thumbnail for us...
   using namespace Gdiplus;
   HRESULT Hr = E_FAIL;
   ULONG_PTR GdipToken = 0;
   GdiplusStartupInput GdipInput;
   GdiplusStartupOutput GdipOutput;
   if( GdiplusStartup(&GdipToken, &GdipInput, &GdipOutput) == Ok ) {
      Bitmap* pBitmap = Bitmap::FromStream(spDstStream, false);
      if( pBitmap != NULL ) {
         Color color(32, 32, 32);
         if( pBitmap->GetHBITMAP(color, phbmp) == Ok ) {
            *pdwAlpha = WTSAT_RGB;
            Hr = S_OK;
         }
         delete pBitmap;
      }
      GdiplusShutdown(GdipToken);
   }
   return Hr;
}


///////////////////////////////////////////////////////////////////////////////
// Helper functions

HRESULT SHCreateImageThumbProvider(CShellFolder* pFolder, PCUITEMID_CHILD pidlItem, REFIID riid, LPVOID* ppRetVal)
{
   ATLASSERT(pFolder);
   ATLASSERT(pidlItem);
   if( riid != IID_IThumbnailProvider ) return E_NOTIMPL;
   CComObject<CImageThumbnailProvider>* pThumb = NULL;
   HR( CComObject<CImageThumbnailProvider>::CreateInstance(&pThumb) );
   CComPtr<IUnknown> spKeepAlive = pThumb->GetUnknown();
   HR( pThumb->Init(pFolder, pidlItem) );
   return pThumb->QueryInterface(riid, ppRetVal);
}

