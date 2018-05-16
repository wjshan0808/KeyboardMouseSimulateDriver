
#include "stdafx.h"

#include "SendTo.h"
#include "ShellFolder.h"


CSendTo::CSendTo() : m_bAcceptFmt(false)
{ 
}

HRESULT WINAPI CSendTo::UpdateRegistry(BOOL bRegister) throw()
{
   // COM registration is done by CShellFolder instance.
   return S_OK;
}

HRESULT CSendTo::Init(CShellFolder* pFolder)
{
   ATLTRACE(L"CSendTo::Init\n");
   ATLASSERT(m_spFolder==NULL);
   m_spFolder = pFolder;
   return S_OK;
}

// IDropTarget

STDMETHODIMP CSendTo::DragEnter(LPDATAOBJECT pDataObj, DWORD dwKeyState, POINTL ptl, LPDWORD pdwEffect)
{
   ATLTRACE(L"CSendTo::DragEnter\n");
   // Does the drag source provide acceptable clipboard format types?
   // We should ask the folder item about this.
   m_bAcceptFmt = _AcceptDropFormat(pDataObj);
   *pdwEffect = _QueryDrop(dwKeyState, *pdwEffect);
   ATLTRACE(L"CSendTo::DragEnter - accept=%d effect=0x%X\n", m_bAcceptFmt, *pdwEffect);
   return S_OK;
}

STDMETHODIMP CSendTo::DragOver(DWORD dwKeyState, POINTL ptl, LPDWORD pdwEffect)
{
   *pdwEffect = _QueryDrop(dwKeyState, *pdwEffect);
   return S_OK;
}

STDMETHODIMP CSendTo::DragLeave(VOID)
{
   ATLTRACE(L"CSendTo::DragLeave\n");
   m_bAcceptFmt = false;
   return S_OK;
}

STDMETHODIMP CSendTo::Drop(LPDATAOBJECT pDataObj, DWORD dwKeyState, POINTL ptl, LPDWORD pdwEffect)
{
   ATLTRACE(L"CSendTo::Drop\n");

   // Determine drop effect...
   DWORD dwDropEffect = _QueryDrop(dwKeyState, *pdwEffect);
   *pdwEffect = DROPEFFECT_NONE;                            // Default to failed/cancelled      
   if( dwDropEffect == DROPEFFECT_NONE ) return S_OK;       // Did we just deny this drop?

   // Simulate the send-to drop by executing a "paste" menu-command on the folder.
   // The folder can decide how to handle a paste operation; ie. by a CopyEngine operation.
   HRESULT Hr = E_FAIL;
   VFS_MENUCOMMAND Cmd = { NULL, DFM_CMD_PASTE, "paste", dwDropEffect, pDataObj, NULL, NULL, NULL, NULL };
   Hr = m_spFolder->ExecuteMenuCommand(Cmd);
   if( Hr == S_OK ) *pdwEffect = dwDropEffect;

   DataObj_SetData(pDataObj, CFSTR_PERFORMEDDROPEFFECT, pdwEffect, sizeof(DWORD));
   DataObj_SetData(pDataObj, CFSTR_PASTESUCCEEDED, pdwEffect, sizeof(DWORD));

   ATLTRACE(L"CSendTo::Drop - completed  hr=0x%X effect=0x%X\n", Hr, *pdwEffect);
   return Hr;
}

// IPersist

STDMETHODIMP CSendTo::GetClassID(CLSID* pClassID)
{
   *pClassID = GetObjectCLSID();
   return S_OK;
}

// IPersistFile

STDMETHODIMP CSendTo::IsDirty()
{
   ATLTRACENOTIMPL(L"CSendTo::IsDirty");
}

STDMETHODIMP CSendTo::Load(LPCOLESTR pwstrFileName, DWORD dwMode)
{
   ATLTRACE(L"CSendTo::Load\n");
   CComObject<CShellFolder>* pFolder = NULL;
   HR( CComObject<CShellFolder>::CreateInstance(&pFolder) );
   CComPtr<IUnknown> spKeepAlive = pFolder->GetUnknown();
   // Need to catch the root folder of our NSE folder...
   KNOWNFOLDERID FolderId = CLSID_NULL;
   switch( _ShellModule.GetConfigInt(VFS_INT_LOCATION) ) {
   case VFS_LOCATION_DESKTOP:    FolderId = FOLDERID_Desktop; break;
   case VFS_LOCATION_MYCOMPUTER: FolderId = FOLDERID_ComputerFolder; break;
   case VFS_LOCATION_USERFILES:  FolderId = FOLDERID_UsersFiles; break;
   case VFS_LOCATION_NETHOOD:    FolderId = FOLDERID_NetworkFolder; break;
   case VFS_LOCATION_PRINTERS:   FolderId = FOLDERID_PrintersFolder; break;
   default:                      return E_UNEXPECTED;
   }
   CPidl pidlFolderPath;
   CCoTaskString strFolderPath;
   HR( pidlFolderPath.CreateFromKnownFolder(FolderId) );
   HR( pidlFolderPath.GetName(SIGDN_DESKTOPABSOLUTEPARSING, &strFolderPath) );
   WCHAR wszRoot[MAX_PATH] = { 0 };
   ::wnsprintf(wszRoot, lengthof(wszRoot) - 1, L"%s%s::%s", static_cast<LPCWSTR>(strFolderPath), strFolderPath.IsEmpty() ? L"" : L"\\", static_cast<LPCWSTR>(CComBSTR(CLSID_ShellFolder)));
   CPidl pidlRoot;
   HR( pidlRoot.CreateFromDisplayName(wszRoot) );
   HR( pFolder->Initialize(pidlRoot) );
   return Init(pFolder);
}

STDMETHODIMP CSendTo::Save(LPCOLESTR, BOOL fRemember)
{
   ATLTRACENOTIMPL(L"CSendTo::Save");
}

STDMETHODIMP CSendTo::SaveCompleted(LPCOLESTR pszFileName)
{
   ATLTRACENOTIMPL(L"CSendTo::SaveCompleted");
}

STDMETHODIMP CSendTo::GetCurFile(LPOLESTR* ppszFileName)
{
   ATLTRACE(L"CSendTo::GetCurFile\n");
   if( m_spFolder == NULL ) return E_UNEXPECTED;
   WCHAR wszPath[MAX_PATH] = { 0 };
   HR( ::SHGetPathFromIDList(m_spFolder->m_pidlRoot, wszPath) );
   return ::SHStrDup(wszPath, ppszFileName);
}   

// Implementation

bool CSendTo::_AcceptDropFormat(IDataObject* pDataObj) const
{
   if( pDataObj == NULL ) return false;
   return m_spFolder->m_spFolderItem->IsDropDataAvailable(pDataObj) == S_OK;
}

DWORD CSendTo::_QueryDrop(DWORD dwKeyState, DWORD dwEffect) const
{
   if( !m_bAcceptFmt ) return DROPEFFECT_NONE;
   DWORD dwMask = _GetDropEffectFromKeyState(dwKeyState);
   if( IsBitSet(dwEffect, dwMask) ) return dwEffect & dwMask;
   // Map common alternatives
   if( IsBitSet(dwEffect, DROPEFFECT_COPY) ) return DROPEFFECT_COPY;
   if( IsBitSet(dwEffect, DROPEFFECT_MOVE) ) return DROPEFFECT_MOVE;
   return DROPEFFECT_COPY;
}

DWORD CSendTo::_GetDropEffectFromKeyState(DWORD dwKeyState) const
{
   // The DROPEFFECT_COPY operation is the default.
   // We don't support DROPEFFECT_LINK operations.
   DWORD dwDropEffect = DROPEFFECT_COPY;
   if( IsBitSet(dwKeyState, MK_CONTROL) ) dwDropEffect = DROPEFFECT_COPY;
   if( IsBitSet(dwKeyState, MK_SHIFT) ) dwDropEffect = DROPEFFECT_MOVE;
   return dwDropEffect;
}


OBJECT_ENTRY_AUTO(CLSID_SendTo, CSendTo)
