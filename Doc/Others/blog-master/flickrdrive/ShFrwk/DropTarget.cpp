
#include "stdafx.h"

#include "DropTarget.h"
#include "ShellFolder.h"


CDropTarget::CDropTarget() : m_hwndTarget(NULL), m_bAcceptFmt(false)
{ 
}

HRESULT WINAPI CDropTarget::UpdateRegistry(BOOL bRegister) throw()
{
   // COM registration is done by CShellFolder instance.
   return S_OK;
}

HRESULT CDropTarget::Init(CShellFolder* pFolder, PCUITEMID_CHILD pidlItem, HWND hWnd)
{
   ATLTRACE(L"CDropTarget::Init\n");
   ATLASSERT(m_spFolder==NULL);
   m_hwndTarget = hWnd;
   m_spFolder = pFolder;
   m_pidlItem = pidlItem;
   return S_OK;
}

// IDropTarget

STDMETHODIMP CDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD dwKeyState, POINTL ptl, LPDWORD pdwEffect)
{
   ATLTRACE(L"CDropTarget::DragEnter\n");
   // Does the drag source provide acceptable clipboard format types?
   // We should ask the folder item about this.
   m_bAcceptFmt = _AcceptDropFormat(pDataObj);
   *pdwEffect = _QueryDrop(dwKeyState, *pdwEffect);
   ATLTRACE(L"CDropTarget::DragEnter - accept=%d effect=0x%X\n", m_bAcceptFmt, *pdwEffect);
   return S_OK;
}

STDMETHODIMP CDropTarget::DragOver(DWORD dwKeyState, POINTL ptl, LPDWORD pdwEffect)
{
   *pdwEffect = _QueryDrop(dwKeyState, *pdwEffect);
   return S_OK;
}

STDMETHODIMP CDropTarget::DragLeave(VOID)
{
   ATLTRACE(L"CDropTarget::DragLeave\n");
   m_bAcceptFmt = false;
   return S_OK;
}

STDMETHODIMP CDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD dwKeyState, POINTL ptl, LPDWORD pdwEffect)
{
   ATLTRACE(L"CDropTarget::Drop\n");

   // Determine drop effect...
   DWORD dwDropEffect = _QueryDrop(dwKeyState, *pdwEffect);
   *pdwEffect = DROPEFFECT_NONE;                            // Default to failed/cancelled      
   if( dwDropEffect == DROPEFFECT_NONE ) return S_OK;       // Did we just deny this drop?

   // Simulate the drop by executing a "paste" menu-command on the folder.
   // The folder can decide how to handle a paste operation; ie. by a CopyEngine operation.
   HRESULT Hr = E_FAIL;
   VFS_MENUCOMMAND Cmd = { m_hwndTarget, DFM_CMD_PASTE, "paste", dwDropEffect, pDataObj, NULL, NULL, NULL, NULL };
   if( m_pidlItem.IsNull() ) {
      Hr = m_spFolder->ExecuteMenuCommand(Cmd);
   }
   else {
      CRefPtr<CShellFolder> spChildFolder;
      HR( m_spFolder->BindToFolder(m_pidlItem.GetItem(0), spChildFolder) );
      Hr = spChildFolder->ExecuteMenuCommand(Cmd);
   }
   if( Hr == S_OK ) *pdwEffect = Cmd.dwDropEffect;

   DataObj_SetData(pDataObj, CFSTR_PERFORMEDDROPEFFECT, pdwEffect, sizeof(DWORD));
   DataObj_SetData(pDataObj, CFSTR_PASTESUCCEEDED, pdwEffect, sizeof(DWORD));

   ATLTRACE(L"CDropTarget::Drop - completed  hr=0x%X effect=0x%X\n", Hr, *pdwEffect);
   return Hr;
}

// IPersist

STDMETHODIMP CDropTarget::GetClassID(CLSID* pClassID)
{
   *pClassID = GetObjectCLSID();
   return S_OK;
}

// IPersistFile

STDMETHODIMP CDropTarget::IsDirty()
{
   ATLTRACENOTIMPL(L"CDropTarget::IsDirty");
}

STDMETHODIMP CDropTarget::Load(LPCOLESTR pwstrFileName, DWORD dwMode)
{
   ATLTRACE(L"CDropTarget::Load\n");
   CPidl pidl;
   HR( pidl.CreateFromPath(pwstrFileName) );
   CComObject<CShellFolder>* pFolder = NULL;
   HR( CComObject<CShellFolder>::CreateInstance(&pFolder) );
   CComPtr<IUnknown> spKeepAlive = pFolder->GetUnknown();
   HR( pFolder->Initialize(pidl) );
   return Init(pFolder, NULL, NULL);
}

STDMETHODIMP CDropTarget::Save(LPCOLESTR, BOOL fRemember)
{
   ATLTRACENOTIMPL(L"CDropTarget::Save");
}

STDMETHODIMP CDropTarget::SaveCompleted(LPCOLESTR pszFileName)
{
   ATLTRACENOTIMPL(L"CDropTarget::SaveCompleted");
}

STDMETHODIMP CDropTarget::GetCurFile(LPOLESTR* ppszFileName)
{
   ATLTRACE(L"CDropTarget::GetCurFile\n");
   if( m_spFolder == NULL ) return E_UNEXPECTED;
   WCHAR wszPath[MAX_PATH] = { 0 };
   HR( ::SHGetPathFromIDList(m_spFolder->m_pidlRoot, wszPath) );
   return ::SHStrDup(wszPath, ppszFileName);
}   

// IShellExtInit

STDMETHODIMP CDropTarget::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObject, HKEY hkeyProgID)
{
   ATLTRACE(L"CDropTarget::Initialize\n");
   ATLASSERT(m_spFolder==NULL);
   // Create and initialize the root folder
   CPidl pidl;
   HR( pidl.CreateFromObject(pDataObject) );
   CComObject<CShellFolder>* pFolder = NULL;
   HR( CComObject<CShellFolder>::CreateInstance(&pFolder) );
   CComPtr<IUnknown> spKeepAlive = pFolder->GetUnknown();
   HR( pFolder->Initialize(pidl) );
   return Init(pFolder, NULL, NULL);
}

// Implementation

bool CDropTarget::_AcceptDropFormat(IDataObject* pDataObj) const
{
   bool bResult = false;
   if( pDataObj == NULL ) return false;
   // Check if the selected item (or just the folder) will accept any drop operation
   if( m_pidlItem.IsNull() ) {
      bResult = (m_spFolder->m_spFolderItem->IsDropDataAvailable(pDataObj) == S_OK);
   }
   else {
      CNseItemPtr spItem = m_spFolder->GenerateChildItem(m_pidlItem.GetItem(0));
      if( spItem != NULL ) bResult = (spItem->IsDropDataAvailable(pDataObj) == S_OK);
   }
   return bResult;
}

DWORD CDropTarget::_QueryDrop(DWORD dwKeyState, DWORD dwEffect) const
{
   if( !m_bAcceptFmt ) return DROPEFFECT_NONE;
   DWORD dwMask = _GetDropEffectFromKeyState(dwKeyState);
   if( IsBitSet(dwEffect, dwMask) ) return dwEffect & dwMask;
   // Map common alternatives
   if( IsBitSet(dwEffect, DROPEFFECT_COPY) ) return DROPEFFECT_COPY;
   if( IsBitSet(dwEffect, DROPEFFECT_MOVE) ) return DROPEFFECT_MOVE;
   return DROPEFFECT_COPY;
}

DWORD CDropTarget::_GetDropEffectFromKeyState(DWORD dwKeyState) const
{
   // The DROPEFFECT_COPY operation is the default.
   // We don't support DROPEFFECT_LINK operations.
   DWORD dwDropEffect = DROPEFFECT_COPY;
   if( IsBitSet(dwKeyState, MK_CONTROL) ) dwDropEffect = DROPEFFECT_COPY;
   if( IsBitSet(dwKeyState, MK_SHIFT) ) dwDropEffect = DROPEFFECT_MOVE;
   return dwDropEffect;
}


OBJECT_ENTRY_AUTO(CLSID_DropTarget, CDropTarget)
