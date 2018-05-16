
#include "stdafx.h"

#include "QueryInfo.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CQueryInfo

HRESULT CQueryInfo::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl)
{
   m_spFolder = pFolder;
   m_pidlItem = pidl;
   return S_OK;
}

// IQueryInfo

STDMETHODIMP CQueryInfo::GetInfoTip(DWORD dwFlags, LPWSTR* ppwszTip)
{
   ATLTRACE(L"CQueryInfo::GetInfoTip  flags=0x%X\n", dwFlags);
   *ppwszTip = NULL;
   // Get the NSE item of the virtual file
   CNseItemPtr spItem = m_spFolder->GenerateChildItem(m_pidlItem.GetItem(0));
   if( spItem == NULL ) return E_UNEXPECTED;
   return spItem->GetInfoTip(dwFlags, ppwszTip);
}

STDMETHODIMP CQueryInfo::GetInfoFlags(DWORD* pdwFlags)
{
   ATLTRACE(L"CQueryInfo::GetInfoFlags\n");
   *pdwFlags = 0;
   return E_FAIL;
}

