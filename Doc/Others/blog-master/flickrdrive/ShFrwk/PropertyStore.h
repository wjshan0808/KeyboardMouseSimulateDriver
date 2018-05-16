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
// CPropertyStore

class ATL_NO_VTABLE CPropertyStore :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IPropertyStore,
   public IPropertyStoreCapabilities
{
public:
   CComPtr<IPropertyStoreCache> m_spCache;
   CRefPtr<CShellFolder> m_spFolder;
   GETPROPERTYSTOREFLAGS m_Flags;
   CPidl m_pidl;

   BEGIN_COM_MAP(CPropertyStore)
      COM_INTERFACE_ENTRY(IPropertyStore)
      COM_INTERFACE_ENTRY(IPropertyStoreCapabilities)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl, const PROPERTYKEY* rgKeys, UINT cKeys, GETPROPERTYSTOREFLAGS gpsFlags);

   // IPropertyStore

   STDMETHOD(GetCount)(DWORD* cProps);
   STDMETHOD(GetAt)(DWORD iProp, PROPERTYKEY* pkey);
   STDMETHOD(GetValue)(REFPROPERTYKEY key, PROPVARIANT* ppropvar);
   STDMETHOD(SetValue)(REFPROPERTYKEY key, REFPROPVARIANT propvar);
   STDMETHOD(Commit)();

   // IPropertyStoreCapabilities

   STDMETHOD(IsPropertyWritable)(REFPROPERTYKEY key);
};


///////////////////////////////////////////////////////////////////////////////
// CPropertyStoreFactory

class ATL_NO_VTABLE CPropertyStoreFactory : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public IPropertyStoreFactory
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CPidl m_pidl;

   BEGIN_COM_MAP(CPropertyStoreFactory)
     COM_INTERFACE_ENTRY(IPropertyStoreFactory)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl);

   // IPropertyStoreFactory

   STDMETHOD(GetPropertyStore)(GETPROPERTYSTOREFLAGS flags, IUnknown* punkFactory, REFIID riid, LPVOID* ppRetVal);
   STDMETHOD(GetPropertyStoreForKeys)(const PROPERTYKEY* rgKeys, UINT cKeys, GETPROPERTYSTOREFLAGS gpsFlags, REFIID riid, LPVOID *ppRetVal);

   // Implementation

   HRESULT _GetColumnKeyMap(GETPROPERTYSTOREFLAGS gpsFlags, CSimpleValArray<PROPERTYKEY>& aKeys) const;
   HRESULT _FilterKeyMap(GETPROPERTYSTOREFLAGS gpsFlags, const PROPERTYKEY* rgKeys, UINT cKeys, CSimpleValArray<PROPERTYKEY>& aKeys) const;
};

