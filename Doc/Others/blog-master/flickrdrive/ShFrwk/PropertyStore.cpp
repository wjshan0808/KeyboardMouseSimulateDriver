
#include "stdafx.h"

#include "PropertyStore.h"
#include "ShellFolder.h"


//////////////////////////////////////////////////////////////////////
// CPropertyStoreFactory

HRESULT CPropertyStoreFactory::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl)
{
   m_spFolder = pFolder;
   m_pidl = pidl;
   return S_OK;
}

// IPropertyStoreFactory

STDMETHODIMP CPropertyStoreFactory::GetPropertyStore(GETPROPERTYSTOREFLAGS gpsFlags, IUnknown* punkFactory, REFIID riid, LPVOID* ppRetVal)
{
   ATLTRACE(L"CPropertyStoreFactory::GetPropertyStore  flags=0x%04X [GPS_%s] punk=%p riid=%s\n", gpsFlags, DbgGetGPSF(gpsFlags), punkFactory, DbgGetIID(riid));
   ATLASSERT(punkFactory==NULL);
   CSimpleValArray<PROPERTYKEY> aKeys;
   HR( _GetColumnKeyMap(gpsFlags, aKeys) );
   CComObject<CPropertyStore>* pStore = NULL;
   HR( CComObject<CPropertyStore>::CreateInstance(&pStore) );
   CComPtr<IUnknown> spKeepAlive = pStore->GetUnknown();
   HR( pStore->Init(m_spFolder, m_pidl.GetItem(0), aKeys.GetData(), (UINT) aKeys.GetSize(), gpsFlags) );
   return pStore->QueryInterface(riid, ppRetVal);
}

STDMETHODIMP CPropertyStoreFactory::GetPropertyStoreForKeys(const PROPERTYKEY* rgKeys, UINT cKeys, GETPROPERTYSTOREFLAGS gpsFlags, REFIID riid, LPVOID* ppRetVal)
{
   ATLTRACE(L"CPropertyStoreFactory::GetPropertyStoreForKeys  keys=%u flags=0x%04X [GPS_%s] riid=%s\n", cKeys, gpsFlags, DbgGetGPSF(gpsFlags), DbgGetIID(riid));
   CSimpleValArray<PROPERTYKEY> aKeys;
   HR( _FilterKeyMap(gpsFlags, rgKeys, cKeys, aKeys) );
   CComObject<CPropertyStore>* pStore = NULL;
   HR( CComObject<CPropertyStore>::CreateInstance(&pStore) );
   CComPtr<IUnknown> spKeepAlive = pStore->GetUnknown();
   HR( pStore->Init(m_spFolder, m_pidl.GetItem(0), aKeys.GetData(), (UINT) aKeys.GetSize(), gpsFlags) );
   return pStore->QueryInterface(riid, ppRetVal);
}

// Implementation

HRESULT CPropertyStoreFactory::_GetColumnKeyMap(GETPROPERTYSTOREFLAGS gpsFlags, CSimpleValArray<PROPERTYKEY>& aKeys) const
{
   VFS_COLUMNINFO Info = { 0 };
   UINT iColumn = 0;
   while( true ) {
      if( FAILED( m_spFolder->m_spFolderItem->GetColumnInfo(iColumn++, Info) ) ) break;
      if( IsBitSet(gpsFlags, GPS_READWRITE) && !IsBitSet(Info.dwAttributes, VFS_COLF_WRITEABLE) ) continue;
      if( IsBitSet(gpsFlags, GPS_FASTPROPERTIESONLY) && IsBitSet(Info.dwFlags, SHCOLSTATE_SLOW) ) continue;
      if( !aKeys.Add(Info.pkey) ) return E_OUTOFMEMORY;
   }
   return S_OK;
}

HRESULT CPropertyStoreFactory::_FilterKeyMap(GETPROPERTYSTOREFLAGS gpsFlags, const PROPERTYKEY* rgKeys, UINT cKeys, CSimpleValArray<PROPERTYKEY>& aKeys) const
{
   // No advanced filtering is needed?
   if( !IsBitSet(gpsFlags, GPS_HANDLERPROPERTIESONLY|GPS_READWRITE|GPS_FASTPROPERTIESONLY) ) {
      for( UINT i = 0; i < cKeys; i++ ) aKeys.Add(rgKeys[i]);
      return S_OK;
   }
   // Filter properties based on column/property information...
   VFS_COLUMNINFO Info = { 0 };
   for( UINT i = 0; i < cKeys; i++ ) {
      UINT iColumn = 0;
      while( true ) {
         if( FAILED( m_spFolder->m_spFolderItem->GetColumnInfo(iColumn++, Info) ) ) {
            if( !IsBitSet(gpsFlags, GPS_HANDLERPROPERTIESONLY) ) aKeys.Add(rgKeys[i]);
            break;
         }
         if( Info.pkey != rgKeys[i] ) continue;
         if( IsBitSet(gpsFlags, GPS_READWRITE) && !IsBitSet(Info.dwAttributes, VFS_COLF_WRITEABLE) ) break;
         if( IsBitSet(gpsFlags, GPS_FASTPROPERTIESONLY) && IsBitSet(Info.dwFlags, SHCOLSTATE_SLOW) ) break;
         if( !aKeys.Add(Info.pkey) ) return E_OUTOFMEMORY;
         break;
      }
   }
   return S_OK;
}


//////////////////////////////////////////////////////////////////////
// CPropertyStore

HRESULT CPropertyStore::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl, const PROPERTYKEY* rgKeys, UINT cKeys, GETPROPERTYSTOREFLAGS gpsFlags)
{
   ATLTRACE(L"CPropertyStore::Init  keys=%u flags=0x%04X [GPS_%s]\n", cKeys, gpsFlags, DbgGetGPSF(gpsFlags));
   ATLASSERT(m_spCache==NULL);
   ATLASSERT(m_spFolder==NULL);

   // Some flags are not supported!
   if( IsBitSet(gpsFlags, GPS_TEMPORARY) ) {
      ATLTRACE(L"CPropertyStore::Init - failed\n");
      return E_INVALIDARG;
   }

   m_spFolder = pFolder;

   m_pidl = pidl;
   m_Flags = gpsFlags;  

   // Create a memory Property Store to cache values
   HR( ::PSCreateMemoryPropertyStore(IID_PPV_ARGS(&m_spCache)) );

   if( IsBitSet(m_Flags, GPS_DELAYCREATION) ) {
      CComPropVariant varEmpty;
      for( UINT i = 0; i < cKeys; i++ ) {
         HR( m_spCache->SetValueAndState(rgKeys[i], &varEmpty, PSC_NOTINSOURCE) );
      }
   }
   else {
      CAutoVectorPtr<CComPropVariant> vars( new CComPropVariant[cKeys] );
      HR( m_spFolder->GetItemProperties(pidl, cKeys, rgKeys, vars) );
      for( UINT i = 0; i < cKeys; i++ ) {
         HR( m_spCache->SetValueAndState(rgKeys[i], &vars[i], PSC_NORMAL) );
      }
   }

   return S_OK;
}

// IPropertyStore

STDMETHODIMP CPropertyStore::GetCount(DWORD* cProps)
{
   ATLTRACE(L"CPropertyStore::GetCount\n");
   return m_spCache->GetCount(cProps);
}

STDMETHODIMP CPropertyStore::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
   ATLTRACE(L"CPropertyStore::GetAt  index=%lu\n", iProp);
   return m_spCache->GetAt(iProp, pkey);
}

STDMETHODIMP CPropertyStore::GetValue(REFPROPERTYKEY key, PROPVARIANT* ppropvar)
{
   ATLTRACE(L"CPropertyStore::GetValue  key=%s\n", DbgGetPKEY(key));
   PSC_STATE State = PSC_NORMAL;
   HRESULT Hr = m_spCache->GetValueAndState(key, ppropvar, &State);
   if( State == PSC_NOTINSOURCE ) {
      m_spFolder->GetItemProperty(m_pidl.GetItem(0), key, ppropvar);
      Hr = m_spCache->SetValueAndState(key, ppropvar, PSC_NORMAL);
   }
#ifdef _DEBUG
   if( FAILED(Hr) || ppropvar->vt == VT_EMPTY ) ATLTRACE(L"CPropertyStore::GetValue - failed\n");
#endif // _DEBUG
   return S_OK;
}

STDMETHODIMP CPropertyStore::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
   ATLTRACE(L"CPropertyStore::SetValue  key=%s\n", DbgGetPKEY(key));
   HR( m_spCache->SetValueAndState(key, &propvar, PSC_DIRTY) );
   return S_OK;
}

STDMETHODIMP CPropertyStore::Commit()
{
   ATLTRACE(L"CPropertyStore::Commit\n");
   DWORD cProps = 0;
   HR( GetCount(&cProps) );
   for( DWORD i = 0; i < cProps; i++ ) {
      PROPERTYKEY pkey = { 0 };
      PSC_STATE State = PSC_NORMAL;
      HR( m_spCache->GetAt(i, &pkey) );
      HR( m_spCache->GetState(pkey, &State) );
      if( State != PSC_DIRTY ) continue;
      CComPropVariant var;
      HR( m_spCache->GetValue(pkey, &var) );
      HR( m_spFolder->SetItemProperty(m_pidl.GetItem(0), pkey, &var) );
   }
   return S_OK;
}

// IPropertyStoreCapabilities

STDMETHODIMP CPropertyStore::IsPropertyWritable(REFPROPERTYKEY pkey)
{
   ATLTRACE(L"CPropertyStore::IsPropertyWritable  key=%s\n", DbgGetPKEY(pkey));
   UINT iColumn = 0;
   while( true ) {
      VFS_COLUMNINFO Info = { 0 };
      if( FAILED( m_spFolder->m_spFolderItem->GetColumnInfo(iColumn++, Info) ) ) break;
      if( pkey != Info.pkey ) continue;
      if( !IsBitSet(Info.dwAttributes, VFS_COLF_WRITEABLE) ) return S_FALSE;
      VFS_PROPSTATE dwState = VFS_PROPSTATE_NORMAL;
      m_spFolder->GetItemPropertyState(m_pidl.GetItem(0), Info.pkey, dwState);
      return IsBitSet(dwState, VFS_PROPSTATE_READONLY) ? S_FALSE : S_OK;
   }
   return S_FALSE;
}

