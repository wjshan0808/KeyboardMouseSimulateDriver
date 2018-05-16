
#include "stdafx.h"

#include "TransferItem.h"
#include "ShellFolder.h"
#include "FileStream.h"


// {4F74D1CF-680C-4EA3-8020-4BDA6792DA3C}
const GUID GUID_UndocumentedStreamResource = { 0x4F74D1CF, 0x680C, 0x4EA3, 0x80, 0x20, 0x4B, 0xDA, 0x67, 0x92, 0xDA, 0x3C };


///////////////////////////////////////////////////////////////////////////////
// CTransferSource

HRESULT CTransferSource::Init(CShellFolder* pFolder)
{
   ATLTRACE(L"CTransferSource::Init\n");
   m_spFolder = pFolder;
   return S_OK;
}

void CTransferSource::FinalRelease()
{
   // Notify our folder about the completion; allowing it to clean up things
   if( m_spFolder != NULL ) m_spFolder->m_spFolderItem->TransferDone();
}

// ITransferSource

STDMETHODIMP CTransferSource::Advise(ITransferAdviseSink* psink, DWORD* pdwCookie)
{
   ATLTRACE(L"CTransferSource::Advise\n");
   *pdwCookie = m_aSinks.Add(psink);
   return S_OK;
}

STDMETHODIMP CTransferSource::Unadvise(DWORD dwCookie)
{
   ATLTRACE(L"CTransferSource::Unadvise\n");
   return m_aSinks.Remove(dwCookie) ? S_OK : E_FAIL;
}

STDMETHODIMP CTransferSource::SetProperties(IPropertyChangeArray* pPropArray)
{
   ATLTRACE(L"CTransferSource::SetProperties\n");
   m_aPropChanges.Add(pPropArray);
   return S_OK;
}

STDMETHODIMP CTransferSource::OpenItem(IShellItem* psiSource, DWORD dwFlags, REFIID riid, LPVOID* ppv)
{
   ATLTRACE(L"CTransferSource::OpenItem  riid=%s flags=0x%X\n", DbgGetIID(riid), dwFlags);
   CNseItemPtr spItem = m_spFolder->GenerateChildItemFromShellItem(psiSource);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   CComObject<CShellItemResources>* pItemResources = NULL;
   HR( CComObject<CShellItemResources>::CreateInstance(&pItemResources) );
   CComPtr<IUnknown> spKeepAlive = pItemResources->GetUnknown();
   HR( pItemResources->Init(m_spFolder, spItem->GetITEMID()) );
   return pItemResources->QueryInterface(riid, ppv);
}

STDMETHODIMP CTransferSource::MoveItem(IShellItem* psiSource, IShellItem* psiParentDst, LPCWSTR pszNameDst, DWORD dwFlags, IShellItem** ppsiNew)
{
   ATLTRACE(L"CTransferSource::MoveItem  flags=0x%X\n", dwFlags);
   // Tell caller that he should convert this to a "copy and delete" operation instead...
   CNseItemPtr spItem = m_spFolder->GenerateChildItemFromShellItem(psiSource);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   if( spItem->IsFolder() ) return E_NOINTERFACE;
   return AtlHresultFromWin32(ERROR_NOT_SAME_DEVICE);
}

STDMETHODIMP CTransferSource::RecycleItem(IShellItem* psiSource, IShellItem* psiParentDest, DWORD dwFlags, IShellItem** ppsiNewDest)
{
   ATLASSERT(false);  // TODO: Implement this maybe?
   ATLTRACENOTIMPL(L"CTransferSource::RecycleItem");
}

STDMETHODIMP CTransferSource::RemoveItem(IShellItem* psiSource, DWORD dwFlags)
{
   ATLTRACE(L"CTransferSource::RemoveItem  flags=0x%X\n", dwFlags);
   CNseItemPtr spItem = m_spFolder->GenerateChildItemFromShellItem(psiSource);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   BOOL bIsFolder = spItem->IsFolder();
   // Delete the file...
   HR( _UpdateTransferState(TS_PERFORMING) );
   HR( spItem->Delete() );
   HR( _SendShellNotify(SHCNE_DELETE, dwFlags, spItem) );
   return bIsFolder ? COPYENGINE_S_DONT_PROCESS_CHILDREN : S_OK;
}

STDMETHODIMP CTransferSource::RenameItem(IShellItem* psiSource, LPCWSTR pszNewName, DWORD dwFlags, IShellItem** ppsiNewDest)
{
   ATLTRACE(L"CTransferSource::RenameItem  flags=0x%X\n", dwFlags);
   CNseItemPtr spItem = m_spFolder->GenerateChildItemFromShellItem(psiSource);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   // Rename file...
   HR( _UpdateTransferState(TS_PERFORMING) );
   WCHAR wszOutputName[MAX_PATH] = { 0 };
   wcscpy_s(wszOutputName, lengthof(wszOutputName), pszNewName);
   HR( spItem->Rename(pszNewName, wszOutputName) );
   // Lookup the new item and send change notifications...
   CNseItemPtr spNewItem;
   HR( m_spFolder->m_spFolderItem->GetChild(wszOutputName, SHGDN_FORPARSING, &spNewItem) );
   HR( _SendShellNotify(SHCNE_RENAMEITEM, dwFlags, spItem, spNewItem) );
   CComQIPtr<IShellFolder> spShellFolder = m_spFolder;
   HR( ::SHCreateItemWithParent(NULL, spShellFolder, spNewItem->GetITEMID(), IID_PPV_ARGS(ppsiNewDest)) );
   return S_OK;
}

STDMETHODIMP CTransferSource::LinkItem(IShellItem* psiSource, IShellItem* psiParentDest, LPCWSTR pszNewName, DWORD dwLinkFlags, IShellItem** ppsiNewDest)
{
   ATLTRACENOTIMPL(L"CTransferSource::LinkItem");
}

STDMETHODIMP CTransferSource::ApplyPropertiesToItem(IShellItem* psiSource, IShellItem** ppsiNewDest)
{
   ATLTRACE(L"CTransferSource::ApplyPropertiesToItem\n");
   HR( _UpdateTransferState(TS_PERFORMING) );
   CNseItemPtr spItem = m_spFolder->GenerateChildItemFromShellItem(psiSource);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   for( UINT i = 0; i < m_aPropChanges.GetCount(); i++ ) {
      CComPtr<IPropertyChangeArray> spPCA = m_aPropChanges.GetAt(i);
      if( spPCA == NULL ) break;
      UINT nChanges = 0;
      HR( spPCA->GetCount(&nChanges) );
      for( UINT iIndex = 0; iIndex < nChanges; iIndex++ ) {
         CComPtr<IPropertyChange> spChange;
         HR( spPCA->GetAt(iIndex, IID_PPV_ARGS(&spChange)) );
         PROPERTYKEY pkey = { 0 };
         HR( spChange->GetPropertyKey(&pkey) );
         CComPropVariant vIn, vOut;
         spItem->GetProperty(pkey, vIn);
         spChange->ApplyToPropVariant(vIn, &vOut);
         HRESULT Hr = spItem->SetProperty(pkey, vOut);
         if( FAILED(Hr) ) return _ReportPropertyError(psiSource, pkey, Hr);
      }
   }
   return S_OK;
}

STDMETHODIMP CTransferSource::GetDefaultDestinationName(IShellItem* psiSource, IShellItem* psiParentDest, LPWSTR* ppszDestinationName)
{
   ATLTRACE(L"CTransferSource::GetDefaultDestinationName\n");
   // We need to return the name we eventually wish our file should have.
   // We'll start trying out if a filename is available, and if not, fall back to 
   // the regular item and parsing name.
   CNseItemPtr spItem = m_spFolder->GenerateChildItemFromShellItem(psiSource);
   if( spItem == NULL ) return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
   CComPropVariant v;
   if( SUCCEEDED( spItem->GetProperty(PKEY_FileName, v) ) ) return ::SHStrDup(v.pwszVal, ppszDestinationName);
   if( SUCCEEDED( spItem->GetProperty(PKEY_ItemName, v) ) ) return ::SHStrDup(v.pwszVal, ppszDestinationName);
   if( SUCCEEDED( spItem->GetProperty(PKEY_ParsingName, v) ) ) return ::SHStrDup(v.pwszVal, ppszDestinationName);
   return psiSource->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, ppszDestinationName);
}

STDMETHODIMP CTransferSource::EnterFolder(IShellItem* psiChildFolderDest)
{
   ATLTRACE(L"CTransferSource::EnterFolder\n");
   return S_OK;
}

STDMETHODIMP CTransferSource::LeaveFolder(IShellItem* psiChildFolderDest)
{
   ATLTRACE(L"CTransferSource::LeaveFolder\n");
   return S_OK;
}

// Implementation

HRESULT CTransferSource::_UpdateTransferState(TRANSFER_ADVISE_STATE State)
{
   for( int i = 0; i < m_aSinks.GetSize(); i++ ) {
      CComQIPtr<ITransferAdviseSink> spSink = m_aSinks.GetAt(i);
      if( spSink != NULL ) spSink->UpdateTransferState(State);
   }
   return S_OK;
}

HRESULT CTransferSource::_UpdateTransferProgress(ULONGLONG ullPos, ULONGLONG ullSize)
{
   for( int i = 0; i < m_aSinks.GetSize(); i++ ) {
      CComQIPtr<ITransferAdviseSink> spSink = m_aSinks.GetAt(i);
      if( spSink != NULL ) spSink->UpdateProgress(ullPos, ullSize, 0, 0, 0, 0);
   }
   return S_OK;
}

HRESULT CTransferSource::_ReportPropertyError(IShellItem* pShellItem, REFPROPERTYKEY pkey, HRESULT Hr)
{
   for( int i = 0; i < m_aSinks.GetSize(); i++ ) {
      CComQIPtr<ITransferAdviseSink> spSink = m_aSinks.GetAt(i);
      if( spSink != NULL ) spSink->PropertyFailure(pShellItem, &pkey, Hr);
   }
   return S_OK;
}

HRESULT CTransferSource::_SendShellNotify(LONG wEventId, DWORD dwFlags, CNseItem* pItem1, CNseItem* pItem2 /*= NULL*/)
{
   if( _ShellModule.GetConfigBool(VFS_IGNORE_TRANSFER_NOTIFY) ) return S_FALSE;
   if( IsBitSet(dwFlags, TSF_SUSPEND_SHELLEVENTS) ) return S_FALSE;
   if( pItem1->IsFolder() ) 
   {
      switch( wEventId ) {
      case SHCNE_CREATE: wEventId = SHCNE_MKDIR; break;
      case SHCNE_DELETE: wEventId = SHCNE_RMDIR; break;
      case SHCNE_UPDATEITEM: wEventId = SHCNE_UPDATEDIR; break;
      case SHCNE_RENAMEITEM: wEventId = SHCNE_RENAMEFOLDER; break;
      }
   }
   if( pItem2 == NULL ) {
      CPidl pidl(m_spFolder->m_pidlRoot, m_spFolder->m_pidlFolder, pItem1->GetITEMID());
      ::SHChangeNotify(wEventId, SHCNF_IDLIST | SHCNF_FLUSH, pidl);
   }
   else {
      CPidl pidl1(m_spFolder->m_pidlRoot, m_spFolder->m_pidlFolder, pItem1->GetITEMID());
      CPidl pidl2(m_spFolder->m_pidlRoot, m_spFolder->m_pidlFolder, pItem2->GetITEMID());
      ::SHChangeNotify(wEventId, SHCNF_IDLIST | SHCNF_FLUSH, pidl1, pidl2);
   }
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CTransferDestination

HRESULT CTransferDestination::Init(CShellFolder* pFolder)
{
   ATLTRACE(L"CTransferDestination::Init\n");
   m_spFolder = pFolder;
   return S_OK;
}

// ITransferDestination

STDMETHODIMP CTransferDestination::Advise(ITransferAdviseSink* psink, DWORD* pdwCookie)
{
   ATLTRACE(L"CTransferDestination::Advise\n");
   *pdwCookie = m_aSinks.Add(psink);
   return S_OK;
}

STDMETHODIMP CTransferDestination::Unadvise(DWORD dwCookie)
{
   ATLTRACE(L"CTransferDestination::Unadvise\n");
   return m_aSinks.Remove(dwCookie) ? S_OK : E_FAIL;
}

STDMETHODIMP CTransferDestination::CreateItem(LPCWSTR pszName, DWORD dwAttributes, ULONGLONG ullSize, DWORD dwFlags, REFIID riidItem, LPVOID* ppvItem, REFIID riidResources, LPVOID* ppvResources)
{
   ATLTRACE(L"CTransferDestination::CreateItem  name=%s item=%s res=%s\n", pszName, DbgGetIID(riidItem), DbgGetIID(riidResources));
   if( riidItem == IID_IShellItem || riidItem == IID_IShellItem2 )
   {
      bool bIsFolder = IsBitSet(dwAttributes, FILE_ATTRIBUTE_DIRECTORY);
      // Create a new PIDL item from a WIN32_FIND_DATA structure
      WIN32_FIND_DATA wfd = { 0 };
      wcscpy_s(wfd.cFileName, lengthof(wfd.cFileName), pszName);
      wfd.dwFileAttributes = dwAttributes;
      wfd.nFileSizeLow = (DWORD) ullSize;
      // Interrogate the old NSE item.
      // NOTE: The following code may trigger an ASSERT; it is to be expected
      //       since we're not usually testing for a non-existing item.
      CNseItemPtr spPrevItem;
      m_spFolder->m_spFolderItem->GetChild(wfd.cFileName, SHGDN_FORPARSING, &spPrevItem);
      if( spPrevItem != NULL ) {
         const WIN32_FIND_DATA wfd2 = spPrevItem->GetFindData();
         if( IsBitSet(dwFlags, TSF_COPY_CREATION_TIME) ) wfd.ftCreationTime = wfd2.ftCreationTime;
         if( IsBitSet(dwFlags, TSF_COPY_WRITE_TIME) ) wfd.ftLastWriteTime = wfd2.ftLastWriteTime;
         if( !bIsFolder ) {
            if( !IsBitSet(dwFlags, TSF_OVERWRITE_EXIST) ) return AtlHresultFromWin32(ERROR_FILE_EXISTS);
            HR( spPrevItem->Delete() );
            HR( _SendShellNotify(SHCNE_DELETE, dwFlags, spPrevItem) );
         }
      }
      // Genereate the new NSE item
      CNseItemPtr spItem = m_spFolder->m_spFolderItem->GenerateChild(m_spFolder, m_spFolder->m_pidlFolder, wfd);
      if( spItem == NULL ) return AtlHresultFromWin32(ERROR_CANNOT_MAKE);
      // If it's a folder, then create the folder now...
      if( bIsFolder && spPrevItem == NULL ) {
         HR( spItem->CreateFolder() );
         spItem.Free();
         HR( m_spFolder->m_spFolderItem->GetChild(wfd.cFileName, SHGDN_FORPARSING, &spItem) );
         HR( _SendShellNotify(SHCNE_CREATE, dwFlags, spItem) );
      }
      CComQIPtr<IShellFolder> spShellFolder = m_spFolder;
      HR( ::SHCreateItemWithParent(NULL, spShellFolder, spItem->GetITEMID(), riidItem, ppvItem) );
      // Initialize the IShellItemResources instance
      CComObject<CShellItemResources>* pItemResources = NULL;
      HR( CComObject<CShellItemResources>::CreateInstance(&pItemResources) );
      CComPtr<IUnknown> spKeepAlive = pItemResources->GetUnknown();
      HR( pItemResources->Init(m_spFolder, wfd) );
      HR( pItemResources->QueryInterface(riidResources, ppvResources) );
      return S_OK;
   }
   ATLTRACE(L"CTransferDestination::CreateItem - failed\n");
   return E_NOINTERFACE;
}

// IServiceProvider

STDMETHODIMP CTransferDestination::QueryService(REFGUID guidService, REFIID riid, LPVOID* ppv)
{
   ATLTRACE(L"CTransferDestination::QueryService  service=%s\n", DbgGetIID(guidService));
   ATLTRACE(L"CTransferDestination::QueryService - faled\n");
   return E_NOINTERFACE;
}

// Implementation

HRESULT CTransferDestination::_SendShellNotify(LONG wEventId, DWORD dwFlags, CNseItem* pItem1, CNseItem* pItem2 /*= NULL*/)
{
   if( _ShellModule.GetConfigBool(VFS_IGNORE_TRANSFER_NOTIFY) ) return S_FALSE;
   if( IsBitSet(dwFlags, TSF_SUSPEND_SHELLEVENTS) ) return S_FALSE;
   if( pItem1->IsFolder() ) 
   {
      switch( wEventId ) {
      case SHCNE_CREATE: wEventId = SHCNE_MKDIR; break;
      case SHCNE_DELETE: wEventId = SHCNE_RMDIR; break;
      case SHCNE_UPDATEITEM: wEventId = SHCNE_UPDATEDIR; break;
      case SHCNE_RENAMEITEM: wEventId = SHCNE_RENAMEFOLDER; break;
      }
   }
   if( pItem2 == NULL ) {
      CPidl pidl(m_spFolder->m_pidlRoot, m_spFolder->m_pidlFolder, pItem1->GetITEMID());
      ::SHChangeNotify(wEventId, SHCNF_IDLIST | SHCNF_FLUSH, pidl);
   }
   else {
      CPidl pidl1(m_spFolder->m_pidlRoot, m_spFolder->m_pidlFolder, pItem1->GetITEMID());
      CPidl pidl2(m_spFolder->m_pidlRoot, m_spFolder->m_pidlFolder, pItem2->GetITEMID());
      ::SHChangeNotify(wEventId, SHCNF_IDLIST | SHCNF_FLUSH, pidl1, pidl2);
   }
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CTransferMediumItem

HRESULT CTransferMediumItem::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl)
{
   ATLTRACE(L"CTransferMediumItem::Init\n");
   m_spFolder = pFolder;
   m_pidl = pidl;
   return S_OK;
}

// ITransferMediumItem

STDMETHODIMP CTransferMediumItem::GetItemIDList(PIDLIST_ABSOLUTE* ppidl)
{
   ATLTRACE(L"CTransferMediumItem::GetItemIDList\n");
   CPidl pidlFull(m_spFolder->m_pidlRoot, m_spFolder->m_pidlFolder, m_pidl.GetItem(0));
   *ppidl = pidlFull.Detach();
   return S_OK;
}

STDMETHODIMP CTransferMediumItem::GetItem(IShellItem** ppsi)
{
   ATLTRACE(L"CTransferMediumItem::GetItem\n");
   CComQIPtr<IShellFolder> spShellFolder = m_spFolder->GetUnknown();
   HR( ::SHCreateItemWithParent(NULL, spShellFolder, m_pidl.GetItem(0), IID_PPV_ARGS(ppsi)) );
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CShellItemResources

HRESULT CShellItemResources::Init(CShellFolder* pFolder, const WIN32_FIND_DATA wfd)
{
   m_spFolder = pFolder;

   m_spItem = m_spFolder->m_spFolderItem->GenerateChild(m_spFolder, m_spFolder->m_pidlFolder, wfd);
   return m_spItem != NULL ? S_OK : E_FAIL;
}

HRESULT CShellItemResources::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidlChild)
{
   m_spFolder = pFolder;

   m_spItem = m_spFolder->GenerateChildItem(::ILCloneChild(pidlChild), TRUE);
   return m_spItem != NULL ? S_OK : E_FAIL;
}

// IShellItemResources

STDMETHODIMP CShellItemResources::GetAttributes(DWORD* pdwAttributes)
{
   ATLTRACE(L"CShellItemResources::GetAttributes\n");
   *pdwAttributes = m_spItem->GetFindData().dwFileAttributes;
   return S_OK;
}

STDMETHODIMP CShellItemResources::GetSize(ULONGLONG* pullSize)
{
   ATLTRACE(L"CShellItemResources::GetSize\n");
   *pullSize = m_spItem->GetFindData().nFileSizeLow;
   return S_OK;
}

STDMETHODIMP CShellItemResources::GetTimes(FILETIME* pftCreation, FILETIME* pftWrite, FILETIME* pftAccess)
{
   ATLTRACE(L"CShellItemResources::GetTimes\n");
   const WIN32_FIND_DATA wfd = m_spItem->GetFindData();
   if( pftCreation != NULL ) *pftCreation = wfd.ftCreationTime;
   if( pftAccess != NULL ) *pftAccess = wfd.ftLastAccessTime;
   if( pftWrite != NULL ) *pftWrite = wfd.ftLastAccessTime;
   return S_OK;
}

STDMETHODIMP CShellItemResources::SetTimes(const FILETIME* pftCreation, const FILETIME* pftWrite, const FILETIME* pftAccess)
{
   ATLTRACE(L"CShellItemResources::SetTimes\n");
   if( pftCreation != NULL ) m_spItem->SetProperty(PKEY_DateCreated, *pftCreation);
   if( pftAccess != NULL ) m_spItem->SetProperty(PKEY_DateAccessed, *pftAccess);
   if( pftWrite != NULL ) m_spItem->SetProperty(PKEY_DateModified, *pftWrite);
   return S_OK;
}

STDMETHODIMP CShellItemResources::GetResourceDescription(const SHELL_ITEM_RESOURCE* pcsir, LPWSTR* ppszDescription)
{
   ATLTRACE(L"CShellItemResources::GetResourceDescription\n");
   if( pcsir->guidType == GUID_UndocumentedStreamResource ) {
      return ::SHStrDupW(L"Stream", ppszDescription);
   }
   ATLTRACE(L"CShellItemResources::GetResourceDescription - failed\n");
   return E_FAIL;
}

STDMETHODIMP CShellItemResources::EnumResources(IEnumResources** ppenumr)
{
   ATLTRACE(L"CShellItemResources::EnumResources\n");
   CSimpleArray<SHELL_ITEM_RESOURCE> aList;
   SHELL_ITEM_RESOURCE FileStream = { GUID_UndocumentedStreamResource, L"" };
   aList.Add(FileStream);
   typedef CComEnum< IEnumResources, &IID_IEnumResources, SHELL_ITEM_RESOURCE, _Copy<SHELL_ITEM_RESOURCE> > CEnumResources;
   CComObject<CEnumResources>* pEnumResources;
   HR( CComObject<CEnumResources>::CreateInstance(&pEnumResources) );
   HR( pEnumResources->Init(aList.GetData(), aList.GetData() + aList.GetSize(), NULL, AtlFlagCopy) );
   return pEnumResources->QueryInterface(IID_PPV_ARGS(ppenumr));
}

STDMETHODIMP CShellItemResources::SupportsResource(const SHELL_ITEM_RESOURCE* pcsir)
{
   ATLTRACE(L"CShellItemResources::SupportsResource  guid=%s [%s]\n", DbgGetIID(pcsir->guidType), pcsir->szName);
   if( pcsir->guidType == GUID_UndocumentedStreamResource ) return S_OK;
   ATLTRACE(L"CShellItemResources::SupportsResource - failed\n");
   return E_NOINTERFACE;
}

STDMETHODIMP CShellItemResources::OpenResource(const SHELL_ITEM_RESOURCE* pcsir, REFIID riid, LPVOID* ppv)
{
   ATLTRACE(L"CShellItemResources::OpenResource  guid=%s [%s] riid=%s\n", DbgGetIID(pcsir->guidType), pcsir->szName, DbgGetIID(riid));
   if( pcsir->guidType == GUID_UndocumentedStreamResource )
   {
      CComObject<CFileStream>* pStream = NULL;
      HR( CComObject<CFileStream>::CreateInstance(&pStream) );
      CComPtr<IUnknown> spKeepAlive = pStream->GetUnknown();
      HR( pStream->Init(m_spFolder, m_spItem->GetITEMID()) );
      return pStream->QueryInterface(riid, ppv);
   }
   return E_NOINTERFACE;
}

STDMETHODIMP CShellItemResources::CreateResource(const SHELL_ITEM_RESOURCE* pcsir, REFIID riid, LPVOID* ppv)
{
   ATLTRACE(L"CShellItemResources::CreateResource  guid=%s [%s] riid=%s\n", DbgGetIID(pcsir->guidType), pcsir->szName, DbgGetIID(riid));
   if( pcsir->guidType == GUID_UndocumentedStreamResource )
   {
      CComObject<CFileStream>* pStream = NULL;
      HR( CComObject<CFileStream>::CreateInstance(&pStream) );
      CComPtr<IUnknown> spKeepAlive = pStream->GetUnknown();
      HR( pStream->Init(m_spFolder, m_spItem->GetITEMID()) );
      return pStream->QueryInterface(riid, ppv);
   }
   return E_NOINTERFACE;
}

STDMETHODIMP CShellItemResources::MarkForDelete(VOID)
{
   ATLTRACENOTIMPL(L"CShellItemResources::MarkForDelete");
}

