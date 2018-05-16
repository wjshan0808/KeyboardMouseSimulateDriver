
#include "stdafx.h"

#include "CommandProvider.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CExplorerCommandProvider

CExplorerCommandProvider::CExplorerCommandProvider() : m_hMenu(NULL)
{ 
}

void CExplorerCommandProvider::FinalRelease()
{
   if( m_hMenu != NULL ) ::DestroyMenu(m_hMenu);
}

HRESULT CExplorerCommandProvider::Init(CShellFolder* pFolder, HWND hWnd, HMENU hMenu)
{
   ATLTRACE(L"CExplorerCommandProvider::Init\n");
   m_spFolder = pFolder;
   m_hMenu = hMenu;
   m_hWnd = hWnd;
   // Check that the menu is valid; we might have supplied an empty menu
   HMENU hMenuPopup = ::GetSubMenu(m_hMenu, _T("CommandMenu"));
   if( !::IsMenu(hMenuPopup) || ::GetMenuItemCount(hMenuPopup) == 0 ) return E_HANDLE;
   // Give item(s) a chance to enable/disable menuitems
   VFS_MENUSTATE State = { hMenuPopup, NULL };
   m_spFolder->m_spFolderItem->SetMenuState(State);
   return S_OK;
}

// IExplorerCommandProvider

STDMETHODIMP CExplorerCommandProvider::GetCommands(IUnknown* punkSite, REFIID riid, LPVOID* ppenumr)
{
   ATLTRACE(L"CExplorerCommandProvider::GetCommands\n");
   typedef CComEnum< IEnumExplorerCommand, &IID_IEnumExplorerCommand, IExplorerCommand*, _CopyInterface<IExplorerCommand> > CEnumCommands;
   HMENU hMenuPopup = ::GetSubMenu(m_hMenu, _T("CommandMenu"));
   CInterfaceArray<IExplorerCommand> aList;
   for( int i = 0; i < ::GetMenuItemCount(hMenuPopup); i++ ) {
      CComPtr<IExplorerCommand> spCommand;
      HR( _GetCommand(hMenuPopup, ::GetMenuItemID(hMenuPopup, i), IID_PPV_ARGS(&spCommand)) );
      aList.Add(spCommand);
   }
   CComObject<CEnumCommands>* pEnumResources;
   HR( CComObject<CEnumCommands>::CreateInstance(&pEnumResources) );
   HR( pEnumResources->Init(&aList.GetData()->p, &aList.GetData()->p + aList.GetCount(), NULL, AtlFlagCopy) );
   return pEnumResources->QueryInterface(riid, ppenumr);
}

STDMETHODIMP CExplorerCommandProvider::GetCommand(REFGUID rguidCommandId, REFIID riid, LPVOID* ppRetVal)
{
   ATLTRACE(L"CExplorerCommandProvider::GetCommand\n");
   HMENU hMenuPopup = ::GetSubMenu(m_hMenu, _T("CommandMenu"));
   return _GetCommand(hMenuPopup, CExplorerCommand::GetMenuID(rguidCommandId), riid, ppRetVal);
}

// Implementation

HRESULT CExplorerCommandProvider::_GetCommand(HMENU hMenu, UINT uMenuID, REFIID riid, LPVOID* ppRetVal)
{
   // NOTE: We grab the menu-information here, including its title. The string
   //       buffer is local and since we intend to store a copy for each Command item
   //       we'll have to transfer the text to a buffer there too.
   WCHAR wszTitle[100] = { 0 };
   MENUITEMINFO mii = { 0 };
   mii.cbSize = sizeof(mii);
   mii.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
   mii.dwTypeData = wszTitle;
   mii.cch = lengthof(wszTitle) - 1;
   if( !::GetMenuItemInfo(hMenu, uMenuID, FALSE, &mii) ) return E_FAIL;
   CComObject<CExplorerCommand>* pCommand = NULL;
   HR( CComObject<CExplorerCommand>::CreateInstance(&pCommand) );
   CComPtr<IUnknown> spKeepAlive = pCommand->GetUnknown();
   HR( pCommand->Init(this, mii) );
   return pCommand->QueryInterface(riid, ppRetVal);
}


///////////////////////////////////////////////////////////////////////////////
// CExplorerCommand

HRESULT CExplorerCommand::Init(CExplorerCommandProvider* pProvider, const MENUITEMINFO& mii)
{
   ATLTRACE(L"CExplorerCommand::Init\n");
   m_spProvider = pProvider;
   // Copy menu-information and switch string buffer to one we own
   m_mii = mii;
   wcscpy_s(m_wszTitle, lengthof(m_wszTitle), mii.dwTypeData);
   m_mii.dwTypeData = m_wszTitle;
   return S_OK;
}

// IExplorerCommand

STDMETHODIMP CExplorerCommand::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName)
{
   return ::SHStrDupW(m_mii.dwTypeData, ppszName);
}

STDMETHODIMP CExplorerCommand::GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon)
{
   WCHAR wszFilename[MAX_PATH] = { 0 };
   ::GetModuleFileName(_pModule->GetResourceInstance(), wszFilename, MAX_PATH);
   WCHAR wszIconPath[MAX_PATH + 20] = { 0 };
   ::wnsprintf(wszIconPath, lengthof(wszIconPath) - 1, _T("%s,-%d"), wszFilename, m_mii.wID);
   return ::SHStrDupW(wszIconPath, ppszIcon);
}

STDMETHODIMP CExplorerCommand::GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip)
{
   CComBSTR bstrMenuText;
   bstrMenuText.LoadString(m_mii.wID);
   if( bstrMenuText.Length() == 0 ) return E_FAIL;
   LPCWSTR pSep = wcschr(bstrMenuText, '\n');
   if( pSep == NULL ) return E_FAIL;
   return ::SHStrDupW(pSep + 1, ppszInfotip);
}

STDMETHODIMP CExplorerCommand::GetCanonicalName(GUID* pguidCommandName)
{
   *pguidCommandName = GetCanonicalName(m_mii.wID);
   return S_OK;
}

STDMETHODIMP CExplorerCommand::GetState(IShellItemArray* psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState)
{
   *pCmdState = 0;
   if( IsBitSet(m_mii.fState, MFS_CHECKED) ) *pCmdState |= ECS_CHECKED;
   if( IsBitSet(m_mii.fState, MFS_DISABLED) ) *pCmdState |= ECS_DISABLED;
   return S_OK;
}

STDMETHODIMP CExplorerCommand::Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc)
{
   ATLTRACE(L"CExplorerCommand::Invoke\n");
   // Execute menu command.
   // If nobody cared for the command, we will try it directly on the folder.
   // This allows commands to work on a specific folder-level even when no items are selected.
   VFS_MENUCOMMAND Cmd = { m_spProvider->m_hWnd, m_mii.wID, VFS_MNUCMD_NOVERB, DROPEFFECT_NONE, NULL, psiItemArray, NULL, m_spUnkSite, NULL };
   HRESULT Hr = m_spProvider->m_spFolder->ExecuteMenuCommand(Cmd);
   if( Hr == S_FALSE ) Hr = m_spProvider->m_spFolder->m_spFolderItem->ExecuteMenuCommand(Cmd);
   return Hr;
}

STDMETHODIMP CExplorerCommand::GetFlags(EXPCMDFLAGS* pFlags)
{
   *pFlags = 0;
   if( m_mii.hSubMenu != NULL ) *pFlags |= ECF_HASSUBCOMMANDS;
   if( m_mii.fType == MFT_SEPARATOR ) *pFlags |= ECF_ISSEPARATOR;
   return S_OK;
}

STDMETHODIMP CExplorerCommand::EnumSubCommands(IEnumExplorerCommand** ppEnum)
{
   typedef CComEnum< IEnumExplorerCommand, &IID_IEnumExplorerCommand, IExplorerCommand*, _CopyInterface<IExplorerCommand> > CEnumCommands;
   HMENU hMenuPopup = ::GetSubMenu(m_spProvider->m_hMenu, _T("CommandMenu"));
   hMenuPopup = ::GetSubMenu(hMenuPopup, ::GetMenuPosFromID(hMenuPopup, m_mii.wID));
   CInterfaceArray<IExplorerCommand> aList;
   for( int i = 0; hMenuPopup != NULL && i < ::GetMenuItemCount(hMenuPopup); i++ ) {
      ATLASSERT(::GetSubMenu(hMenuPopup, i)==NULL);  // No sub-sub menus!
      CComPtr<IExplorerCommand> spCommand;
      HR( m_spProvider->_GetCommand(hMenuPopup, ::GetMenuItemID(hMenuPopup, i), IID_PPV_ARGS(&spCommand)) );
      aList.Add(spCommand);
   }
   CComObject<CEnumCommands>* pEnumResources;
   HR( CComObject<CEnumCommands>::CreateInstance(&pEnumResources) );
   HR( pEnumResources->Init(&aList.GetData()->p, &aList.GetData()->p + aList.GetCount(), NULL, AtlFlagCopy) );
   return pEnumResources->QueryInterface(ppEnum);
}

// Static members

GUID CExplorerCommand::GetCanonicalName(UINT wID)
{
   // Derive menu-item GUIDs from the ShellFolder CLSID; violating
   // a few GUID uniqueness rules here perhaps.
   GUID guid = CLSID_ShellFolder;
   guid.Data2 = 0xEFFE;
   guid.Data3 = (USHORT) wID;
   return guid;
}

UINT CExplorerCommand::GetMenuID(GUID guid)
{
   return guid.Data2 == 0xEFFE ? guid.Data3 : 0;
}
