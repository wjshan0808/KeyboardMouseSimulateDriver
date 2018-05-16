
#include "stdafx.h"

#include "ContextMenu.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CContextMenu

HRESULT WINAPI CContextMenu::UpdateRegistry(BOOL bRegister) throw()
{
   // COM registration is done by CShellFolder instance
   return S_OK;
}

// IContextMenu

STDMETHODIMP CContextMenu::QueryContextMenu(HMENU hMenu, UINT iIndexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
   ATLTRACE(L"CContextMenu::QueryContextMenu\n");
   ATLASSERT(m_spMaster);
   if( m_spMaster == NULL ) return E_UNEXPECTED;
   return m_spMaster->QueryContextMenu(hMenu, iIndexMenu, idCmdFirst, idCmdLast, uFlags);
}

STDMETHODIMP CContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pcmi)
{
   ATLTRACE(L"CContextMenu::InvokeCommand\n");
   ATLASSERT(m_spMaster);
   if( m_spMaster == NULL ) return E_UNEXPECTED;
   return m_spMaster->InvokeCommand(pcmi);
}

STDMETHODIMP CContextMenu::GetCommandString(UINT_PTR idCmd, UINT uType, LPUINT pReserved, LPSTR pszName, UINT cchMax)
{
   ATLTRACE(L"CContextMenu::GetCommandString\n");
   ATLASSERT(m_spMaster);
   if( m_spMaster == NULL ) return E_UNEXPECTED;
   return m_spMaster->GetCommandString(idCmd, uType, pReserved, pszName, cchMax);
}

// IContextMenu2

STDMETHODIMP CContextMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ATLASSERT(m_spMaster);
   if( m_spMaster == NULL ) return E_UNEXPECTED;
   return m_spMaster->HandleMenuMsg(uMsg, wParam, lParam);
}

// IContextMenu3

STDMETHODIMP CContextMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
   ATLASSERT(m_spMaster);
   if( m_spMaster == NULL ) return E_UNEXPECTED;
   return m_spMaster->HandleMenuMsg2(uMsg, wParam, lParam, plResult);
}

// IShellExtInit

STDMETHODIMP CContextMenu::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObject, HKEY hkeyProgID)
{
   ATLTRACE(L"CContextMenu::Initialize\n");
   ATLASSERT(m_spMaster==NULL);
   // Create and initialize the root folder
   CPidl pidl;
   HR( pidl.CreateFromObject(pDataObject) );
   CComObject<CShellFolder>* pFolder = NULL;
   HR( CComObject<CShellFolder>::CreateInstance(&pFolder) );
   CComPtr<IUnknown> spKeepAlive = pFolder->GetUnknown();
   HR( pFolder->Initialize(pidl) );
   // Just ask the folder to create the menu, then propagate all
   // calls to this instance (see above).
   m_spFolder = pFolder;
   HR( m_spFolder->CreateMenu(NULL, _T("ExplorerMenu"), &m_spMaster) );
   return S_OK;
}


OBJECT_ENTRY_AUTO(CLSID_ContextMenu, CContextMenu)
