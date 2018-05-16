
#include "stdafx.h"

#include "EnumIDList.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// EnumObjects Thread

typedef struct tagENUMDATA
{
   CRefPtr<CShellFolder> spFolder;
   CPidl pidlMonitor;
   HWND hWnd;
   SHCONTF grfFlags;
} ENUMDATA;


VOID CALLBACK AddEnumObjectCB(HANDLE hAsync, PCITEMID_CHILD pidlItem)
{
   const ENUMDATA* pData = reinterpret_cast<ENUMDATA*>(hAsync);
   ShellFolderView_AddObject(pData->hWnd, ::ILCloneChild(pidlItem));
}

DWORD WINAPI EnumObjectsThread(LPVOID pData)
{
   ENUMDATA data = * (ENUMDATA*) pData;
   delete (ENUMDATA*) pData;

   CNseItemArray aChildren;
   HRESULT Hr = data.spFolder->m_spFolderItem->EnumChildren(data.hWnd, data.grfFlags, &data, AddEnumObjectCB);
   if( FAILED(Hr) ) return 0;

   return 0;
};


///////////////////////////////////////////////////////////////////////////////
// CEnumIDList

CEnumIDList::CEnumIDList() : m_hWnd(NULL), m_grfFlags(0), m_uPos(0), m_uCount(0)
{
}

HRESULT CEnumIDList::Init(CShellFolder* pFolder, HWND hwnd, SHCONTF grfFlags)
{
   ATLTRACE(L"CEnumIDList::Init  hwnd=%X flags=0x%X [SHCONT_%s]\n", hwnd, grfFlags, DbgGetSHCONTF(grfFlags));

   m_spFolder = pFolder;
   m_hWnd = hwnd;
   m_grfFlags = grfFlags;

   HRESULT Hr = S_OK;
   if( IsBitSet(m_grfFlags, SHCONTF_ENABLE_ASYNC) ) 
   {
      m_grfFlags &= ~SHCONTF_INIT_ON_FIRST_NEXT;
      // The CNseItem::EnumChildren() call return E_PENDING if it is called
      // with async enabled and wants to run on a new thread.
      Hr = _PopulateList();
      if( Hr == E_PENDING ) {
         ENUMDATA* pData = new ENUMDATA;
         pData->spFolder = m_spFolder;
         pData->hWnd = m_hWnd;
         pData->grfFlags = m_grfFlags;
         pData->pidlMonitor = m_spFolder->m_pidlMonitor;
         if( !::SHCreateThread(EnumObjectsThread, pData, CTF_COINIT | CTF_PROCESS_REF | CTF_INSIST, NULL) ) return E_FAIL;
         Hr = S_OK;
      }
   }
   else if( IsBitSet(m_grfFlags, SHCONTF_INIT_ON_FIRST_NEXT) ) 
   {
      // Delayed initialization of enum...
   }
   else
   {
      Hr =_PopulateList();
   }
   return Hr;
}

// IEnumIDList

STDMETHODIMP CEnumIDList::Next(ULONG celt, PITEMID_CHILD* rgelt, ULONG* pceltFetched)
{
   ATLTRACE(L"CEnumIDList::Next\n");
   *rgelt = NULL; 
   if( pceltFetched != NULL ) *pceltFetched = 0;
   if( pceltFetched == NULL && celt != 1 ) return E_POINTER;
   // Did user request delayed initialization?
   if( IsBitSet(m_grfFlags, SHCONTF_INIT_ON_FIRST_NEXT) ) {
      m_grfFlags &= ~SHCONTF_INIT_ON_FIRST_NEXT;
      HR( _PopulateList() );
   }
   // Grab the requested PIDLs...
   ULONG nCount = 0;
   while( m_uPos < m_uCount && nCount < celt ) {
      rgelt[nCount++] = ::ILCloneFirst(m_pCur);
      m_pCur = ::ILGetNext(m_pCur);
      m_uPos++;
   }
   if( pceltFetched != NULL ) *pceltFetched = nCount;
   return celt == nCount ? S_OK : S_FALSE;
}

STDMETHODIMP CEnumIDList::Reset()
{
   ATLTRACE(L"CEnumIDList::Reset\n");
   m_uPos = 0;
   m_pCur = m_pidl;
   return S_OK; 
}

STDMETHODIMP CEnumIDList::Skip(ULONG celt)
{
   ATLTRACE(L"CEnumIDList::Skip\n");
   ATLASSERT(!IsBitSet(m_grfFlags, SHCONTF_INIT_ON_FIRST_NEXT));
   ULONG nCount = 0;
   while( m_uPos < m_uCount && nCount < celt ) {
      m_pCur = ::ILGetNext(m_pCur);
      m_uPos++; 
      nCount++;
   }
   return nCount == celt ? S_OK : S_FALSE;
}

STDMETHODIMP CEnumIDList::Clone(IEnumIDList** ppEnum)
{
   ATLTRACENOTIMPL(L"CEnumIDList::Clone");
}

// Implementation

HRESULT CEnumIDList::_PopulateList()
{
   ATLASSERT(m_pidl.IsNull());
   HRESULT Hr = m_spFolder->m_spFolderItem->EnumChildren(m_hWnd, m_grfFlags, m_aChildren);
   ATLASSERT(Hr==S_OK || Hr==E_PENDING);
   if( Hr != S_OK && Hr != E_PENDING ) return Hr;
   for( int i = 0; i < m_aChildren.GetSize(); i++ ) m_pidl.Append( m_aChildren[i]->GetITEMID() );
   m_uCount = m_pidl.GetItemCount();
   m_pCur = m_pidl;
   ATLTRACE(L"CEnumIDList::_PopulateList - %ld items\n", m_uCount);
   return Hr;
}

