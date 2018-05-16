
#include "stdafx.h"

#include "DataObject.h"
#include "ShellFolder.h"
#include "FileStream.h"

#pragma warning( disable : 4510 4610 )


///////////////////////////////////////////////////////////////////////////////
// CDataObject statics

CLIPFORMAT CDataObject::s_cfPRIVATE = (CLIPFORMAT)::RegisterClipboardFormatW(CComBSTR(CLSID_ShellFolder));
CLIPFORMAT CDataObject::s_cfFILECONTENTS = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_FILECONTENTS);
CLIPFORMAT CDataObject::s_cfFILEDESCRIPTOR = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
CLIPFORMAT CDataObject::s_cfPREFERREDDROPEFFECT = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);


///////////////////////////////////////////////////////////////////////////////
// CDataObject

HRESULT CDataObject::Init(CShellFolder* pFolder, HWND hwndOwner, IDataObject* pMasterObject, UINT cidl, PCUITEMID_CHILD_ARRAY rgpidl)
{
   ATLTRACE(L"CDataObject::Init\n");

   if( cidl == 0 ) return E_INVALIDARG;

   // This is not a full implementation of an IDataObject, but rather an
   // implementation that works with the SHCreateDataObject method. It
   // expects that a master data-object takes care of much of the maintenance.

   m_spFolder = pFolder;
   m_hwndOwner = hwndOwner;
   m_pidls.Copy(rgpidl, cidl);

   return S_OK;
}

// IDataObject

STDMETHODIMP CDataObject::GetData(FORMATETC* pFormatetc, STGMEDIUM* pMedium)
{
   ATLTRACE(L"CDataObject::GetData  cf=%s\n", DbgGetCF(pFormatetc->cfFormat));

   if( pFormatetc->cfFormat == s_cfFILEDESCRIPTOR && _ShellModule.GetConfigBool(VFS_HAVE_VIRTUAL_FILES) ) 
   {
      // The FILEDESCRIPTOR format will be queried by the Shell before the FILECONTENTS.
      // It returns descriptions of the files the IDataObject contains.
      if( !IsBitSet(pFormatetc->tymed, TYMED_HGLOBAL) ) return DV_E_TYMED;

      if( m_aFiles.GetSize() == 0 ) {
         WCHAR wszFolder[MAX_PATH] = { 0 };
         HR( _CollectFiles(m_spFolder, m_spFolder->m_pidlFolder, wszFolder, m_pidls) );
         ATLTRACE(L"CDataObject::GetData - %d files in data-object\n", m_aFiles.GetSize());
      }

      FILEGROUPDESCRIPTOR fgd = { m_aFiles.GetSize(), 0 };
      DWORD dwFgdSize = sizeof(fgd) - sizeof(FILEDESCRIPTOR);
      DWORD dwTotalSize = dwFgdSize + (sizeof(FILEDESCRIPTOR) * m_aFiles.GetSize());
      CAutoVectorPtr<BYTE> buffer( new BYTE[dwTotalSize] );
      if( buffer == NULL ) return E_OUTOFMEMORY;
      ::CopyMemory(buffer, &fgd, dwFgdSize);
      ::CopyMemory(buffer + dwFgdSize, m_aFiles.GetData(), sizeof(FILEDESCRIPTOR) * m_aFiles.GetSize());
      HR( _SetHGLOBAL(pMedium, buffer, dwTotalSize) );
      return S_OK;
   }

   if( pFormatetc->cfFormat == s_cfFILECONTENTS && _ShellModule.GetConfigBool(VFS_HAVE_VIRTUAL_FILES) ) 
   {
      // The FILECONTENTS gets called for each file.
      // It will return the IStream for the file.
      if( pFormatetc->ptd != NULL ) return DV_E_DVTARGETDEVICE;
      if( !IsBitSet(pFormatetc->tymed, TYMED_ISTREAM) ) return DV_E_TYMED;
      if( !IsBitSet(pFormatetc->dwAspect, DVASPECT_CONTENT) ) return DV_E_DVASPECT;

      LONG iIndex = pFormatetc->lindex;
      if( iIndex < 0 || iIndex >= (LONG) m_aFiles.GetSize() ) return DV_E_LINDEX;

      // Return the IStream for the file...
      const FILEDESCRIPTOR& fd = m_aFiles[iIndex];
      CPidl pidlParsed;
      HR( m_spFolder->ParseDisplayName(NULL, NULL, const_cast<LPWSTR>(fd.cFileName), NULL, &pidlParsed, NULL) );
      HR( m_spFolder->BindToObject(pidlParsed, NULL, IID_PPV_ARGS(&pMedium->pstm)) );
      pMedium->tymed = TYMED_ISTREAM;
      pMedium->pUnkForRelease = NULL;
      return S_OK;
   }

   if( pFormatetc->cfFormat == s_cfPREFERREDDROPEFFECT && _ShellModule.GetConfigBool(VFS_CAN_SLOW_COPY) )
   {
      DWORD dwDropEffect = DROPEFFECT_COPY;
      return _SetHGLOBAL(pMedium, &dwDropEffect, sizeof(dwDropEffect));
   }

   if( pFormatetc->cfFormat == s_cfPRIVATE )
   {
      DWORD dwValue = 1;
      return _SetHGLOBAL(pMedium, &dwValue, sizeof(dwValue));
   }

   return E_NOTIMPL;
}

STDMETHODIMP CDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDataObject::GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pMedium)
{
   ATLTRACE(L"CDataObject::GetDataHere\n");
   return GetData(pFormatetc, pMedium);
}

STDMETHODIMP CDataObject::QueryGetData(FORMATETC* pFormatetc)
{
   ATLTRACE(L"CDataObject::QueryGetData  cf='%s' tymed=0x%X\n", DbgGetCF(pFormatetc->cfFormat), pFormatetc->tymed);
   static struct {
      CLIPFORMAT cf;
      DWORD tymed;
      VFS_CONFIG require;
   } aSupportedFormats[] = {
      { s_cfPRIVATE,              TYMED_HGLOBAL,   VFS_NONE },
      { s_cfPREFERREDDROPEFFECT,  TYMED_HGLOBAL,   VFS_CAN_SLOW_COPY },
      { s_cfFILEDESCRIPTOR,       TYMED_HGLOBAL,   VFS_HAVE_VIRTUAL_FILES },
      { s_cfFILECONTENTS,         TYMED_ISTREAM,   VFS_HAVE_VIRTUAL_FILES },
   };
   for( int i = 0; i < lengthof(aSupportedFormats); i++ ) {
      if( pFormatetc->cfFormat != aSupportedFormats[i].cf ) continue;
      if( !IsBitSet(pFormatetc->tymed, aSupportedFormats[i].tymed) ) return DV_E_TYMED;
      if( aSupportedFormats[i].require != VFS_NONE && !_ShellModule.GetConfigBool(aSupportedFormats[i].require) ) continue;
      return S_OK;
   }
   return DV_E_FORMATETC;
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(FORMATETC* pFormatectIn, FORMATETC* pFormatetcOut)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDataObject::DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDataObject::DUnadvise(DWORD dwConnection)
{
   return E_NOTIMPL;
}

STDMETHODIMP CDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
   return E_NOTIMPL;
}

// Implementation

HRESULT CDataObject::_SetHGLOBAL(STGMEDIUM* pMedium, LPCVOID pData, SIZE_T cbData) const
{
   ATLASSERT(pMedium);
   ATLASSERT(pData);
   HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, cbData);
   if( hMem == NULL ) return E_OUTOFMEMORY;
   LPVOID pMediumData = ::GlobalLock(hMem);
   if( pMediumData == NULL ) return E_OUTOFMEMORY;
   ::CopyMemory(pMediumData, pData, cbData);
   ::GlobalUnlock(hMem); 
   pMedium->hGlobal = hMem;
   pMedium->tymed = TYMED_HGLOBAL;
   pMedium->pUnkForRelease = NULL;
   return S_OK;
}

HRESULT CDataObject::_CollectFiles(IShellFolder* pShellFolder, PCIDLIST_RELATIVE pidlFolder, LPCWSTR pstrFolder, CPidl& pidls)
{
   ATLASSERT(pShellFolder);
   UINT nCount = pidls.GetItemCount();
   for( UINT i = 0; i < nCount; i++ ) 
   {
      CPidl pidlItem = pidls.GetItem(i);

      WIN32_FIND_DATA wfd = { 0 };
      HRESULT Hr = ::SHGetDataFromIDList(pShellFolder, pidlItem.GetItem(0), SHGDFIL_FINDDATA, &wfd, sizeof(wfd));
      if( FAILED(Hr) ) return Hr;

      FILEDESCRIPTOR fd = { 0 };
      fd.dwFlags = (DWORD)(FD_FILESIZE | FD_ATTRIBUTES | FD_UNICODE);
      if( _ShellModule.GetConfigBool(VFS_CAN_PROGRESSUI) ) fd.dwFlags |= FD_PROGRESSUI;
      wcscpy_s(fd.cFileName, lengthof(fd.cFileName), pstrFolder);
      wcscat_s(fd.cFileName, lengthof(fd.cFileName), wfd.cFileName);
      fd.nFileSizeLow = wfd.nFileSizeLow;
      if( IsFileTimeValid(wfd.ftCreationTime) ) fd.ftCreationTime = wfd.ftCreationTime, fd.dwFlags |= FD_CREATETIME;
      if( IsFileTimeValid(wfd.ftLastWriteTime) ) fd.ftLastWriteTime = wfd.ftLastWriteTime, fd.dwFlags |= FD_WRITESTIME;
      if( IsFileTimeValid(wfd.ftLastAccessTime) ) fd.ftLastAccessTime = wfd.ftLastAccessTime, fd.dwFlags |= FD_ACCESSTIME;
      fd.dwFileAttributes = wfd.dwFileAttributes;
      if( !m_aFiles.Add(fd) ) return E_OUTOFMEMORY;

      // Recurse into sub-folders...
      if( IsBitSet(wfd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
      {
         CComPtr<IShellFolder> spSubFolder;
         HR( pShellFolder->BindToObject(pidlItem, NULL, IID_PPV_ARGS(&spSubFolder)) );
         CComPtr<IEnumIDList> spEnum;
         HR( spSubFolder->EnumObjects(m_hwndOwner, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_STORAGE, &spEnum) );
         ULONG uFetched = 0;
         CPidl pidlSubItems;
         PITEMID_CHILD pidlChild = NULL;
         while( spEnum->Next(1, &pidlChild, &uFetched), uFetched > 0 ) {
            pidlSubItems.Append(pidlChild);
            ::CoTaskMemFree(pidlChild);
            uFetched = 0;
         }
         WCHAR wszSubFolder[MAX_PATH] = { 0 };
         ::wnsprintf(wszSubFolder, lengthof(wszSubFolder) - 1, L"%s%s\\", pstrFolder, wfd.cFileName);
         CPidl pidlSubFolder = pidlFolder + pidlItem;
         HR( _CollectFiles(spSubFolder, pidlSubFolder, wszSubFolder, pidlSubItems) );
      }
   }
   return S_OK;
}

