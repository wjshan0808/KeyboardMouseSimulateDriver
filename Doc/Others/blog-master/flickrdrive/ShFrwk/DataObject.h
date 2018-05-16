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
// CDataObject

class CDataObject : 
   public CComObjectRootEx<CComMultiThreadModelNoCS>, 
   public IDataObject
{
public:
   CRefPtr<CShellFolder> m_spFolder;                 // Reference to our folder
   HWND m_hwndOwner;                                 // Reference to owner window
   CPidl m_pidls;                                    // The list of PIDL items   
   CSimpleArray<FILEDESCRIPTOR> m_aFiles;            // The files

   BEGIN_COM_MAP(CDataObject)
      COM_INTERFACE_ENTRY(IDataObject)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, HWND hwndOwner, IDataObject* pMasterObject, UINT cidl, PCUITEMID_CHILD_ARRAY rgpidl);

   // IDataObject

   STDMETHOD(GetData)(FORMATETC* pformatetcIn, STGMEDIUM* pmedium);
   STDMETHOD(GetDataHere)(FORMATETC* pformatetc, STGMEDIUM* pmedium);
   STDMETHOD(QueryGetData)(FORMATETC* pformatetc);
   STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* pformatectIn, FORMATETC* pformatetcOut);
   STDMETHOD(SetData)(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);
   STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
   STDMETHOD(DAdvise)(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);
   STDMETHOD(DUnadvise)(DWORD dwConnection);
   STDMETHOD(EnumDAdvise)(IEnumSTATDATA** ppenumAdvise);

   // Implementation

   HRESULT _SetHGLOBAL(STGMEDIUM* pMedium, LPCVOID pData, SIZE_T cbData) const;
   HRESULT _CollectFiles(IShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, LPCWSTR pstrFolder, CPidl& pidls);

   // Static data

   static CLIPFORMAT s_cfFILEDESCRIPTOR;             // Clipboard format for FILEDESCRIPTOR
   static CLIPFORMAT s_cfFILECONTENTS;               // Clipboard format for FILECONTENTS
   static CLIPFORMAT s_cfPREFERREDDROPEFFECT;        // Clipboard format for PREFERREDDROPEFFECT
   static CLIPFORMAT s_cfPRIVATE;                    // Clipboard format for our private marker
};

