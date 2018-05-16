
#include "stdafx.h"

#include "CategoryProvider.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CCategoryProvider

HRESULT CCategoryProvider::Init(CShellFolder* pFolder)
{
   m_spFolder = pFolder;
   return S_OK;
}

// ICategoryProvider

STDMETHODIMP CCategoryProvider::CanCategorizeOnSCID(const SHCOLUMNID* pscid)
{
   CComPtr<IPropertyDescription> spDescription;
   HR( ::PSGetPropertyDescription(*pscid, IID_PPV_ARGS(&spDescription)) );
   PROPDESC_TYPE_FLAGS Flags = PDTF_DEFAULT;
   HR( spDescription->GetTypeFlags(PDTF_CANGROUPBY, &Flags) );
   return Flags != PDTF_DEFAULT ? S_OK : S_FALSE;
}

STDMETHODIMP CCategoryProvider::GetDefaultCategory(GUID *pguid, SHCOLUMNID* pscid)
{
   ATLTRACENOTIMPL(L"CCategoryProvider::GetDefaultCategory\n");
}

STDMETHODIMP CCategoryProvider::GetCategoryForSCID(const SHCOLUMNID* pscid, GUID* pguid)
{
   ATLTRACENOTIMPL(L"CCategoryProvider::GetCategoryForSCID\n");
}

STDMETHODIMP CCategoryProvider::EnumCategories(IEnumGUID** penum)
{
   ATLTRACENOTIMPL(L"CCategoryProvider::EnumCategories\n");
}

STDMETHODIMP CCategoryProvider::GetCategoryName(const GUID* pguid, LPWSTR pszName, UINT cch)
{
   ATLTRACENOTIMPL(L"CCategoryProvider::GetCategoryName\n");
}

STDMETHODIMP CCategoryProvider::CreateCategory(const GUID* pguid, REFIID riid, LPVOID* ppv)
{
   ATLTRACE(L"CCategoryProvider::CreateCategory  %s\n", DbgGetIID(riid));
   ATLTRACE(L"CCategoryProvider::CreateCategory - failed\n");
   return E_NOINTERFACE;
}

