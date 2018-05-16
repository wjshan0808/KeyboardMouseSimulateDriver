
#include "stdafx.h"

#include "PropSheetExt.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CPropSheetExt

CPropSheetExt::CPropSheetExt() : m_pPage(NULL)
{
}

void CPropSheetExt::FinalRelease()
{
   delete m_pPage;
}

HRESULT WINAPI CPropSheetExt::UpdateRegistry(BOOL bRegister) throw()
{
   // COM registration is done by CShellFolder instance.
   return S_OK;
}

// IShellPropSheetExt

STDMETHODIMP CPropSheetExt::AddPages(LPFNSVADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
   ATLTRACE(L"CPropSheetExt::AddPages\n");
   ATLASSERT(m_pPage==NULL);
   if( m_pPage != NULL ) return E_UNEXPECTED;
   if( m_aChildren.GetSize() == 0 ) return E_UNEXPECTED;
   // Let the first item create the property page.
   // We might have multiple items in the selection. It's the item's Property Page's
   // job to handle this situation, we will just pass it the entire selection.
   CNseItem* pItem = m_aChildren[0];
   ATLASSERT(pItem);
   if( pItem == NULL ) return E_FAIL;
   if( FAILED( pItem->GetPropertyPage(&m_pPage) ) ) return S_OK;
   HR( m_pPage->SetItemReferences(this, m_spItems, &m_aChildren) );
   // Add page, return HRESULT + page index
   return m_pPage->AddPages(pfnAddPage, lParam);
}

STDMETHODIMP CPropSheetExt::ReplacePage(EXPPS uPageID, LPFNSVADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam)
{
   ATLTRACENOTIMPL(L"CPropSheetExt::ReplacePage");
}

// IShellExtInit

STDMETHODIMP CPropSheetExt::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObject, HKEY hkeyProgID)
{
   ATLTRACE(L"CPropSheetExt::Initialize\n");
   ATLASSERT(m_spItems==NULL);
   // Argh, the Shell provides us with a list of absolute pidls. We at least
   // need to get to the C++ implementation of each NSE Item.
   // Go to work...
   m_spDataObject = pDataObject;
   HR( ::SHCreateShellItemArrayFromDataObject(m_spDataObject, IID_PPV_ARGS(&m_spItems)) );
   DWORD dwItemCount = 0;
   HR( m_spItems->GetCount(&dwItemCount) );
   if( dwItemCount == 0 ) return E_INVALIDARG;
   for( DWORD i = 0; i < dwItemCount; i++ ) {
      CComPtr<IShellItem> spItem;
      HR( m_spItems->GetItemAt(i, &spItem) );
      CComPtr<IShellItem> spParent;
      HR( spItem->GetParent(&spParent) );
      CComPtr<IShellFolder> spShellFolder;
      HR( spParent->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARGS(&spShellFolder)) );
      CRefPtr<CShellFolder> spFolder;
      if( FAILED( spShellFolder->CreateViewObject(NULL, CLSID_ShellFolder, (LPVOID*) &spFolder) ) ) {
         // Not our folder; aka it's the root item
         ATLASSERT(dwItemCount==1);
         CPidl pidlRoot;
         HR( pidlRoot.CreateFromObject(spItem) );
         CComObject<CShellFolder>* pRootFolder = NULL;
         HR( CComObject<CShellFolder>::CreateInstance(&pRootFolder) );
         CComQIPtr<IShellFolder> spRootFolder = pRootFolder;
         HR( pRootFolder->Initialize(pidlRoot) );
         m_aChildren.Add( pRootFolder->m_spFS->GenerateRoot(pRootFolder) );
         m_aFolders.Add(spRootFolder);
      }
      else {
         // ShellItem is a child of our folder; cache its NSE Item
         CCoTaskString strName;
         HR( spItem->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &strName) );
         CNseItem* pItem = NULL;
         HR( spFolder->m_spFolderItem->GetChild(strName, SHGDN_FORPARSING, &pItem) );
         m_aChildren.Add(pItem);
         m_aFolders.Add(spShellFolder);
      }
   }
   return S_OK;
}

OBJECT_ENTRY_AUTO(CLSID_PropertySheet, CPropSheetExt)


///////////////////////////////////////////////////////////////////////////////
// CNsePropertyPage

CNsePropertyPage::CNsePropertyPage() : m_uIDD(0), m_uStartPageIndex(0), m_pSheet(NULL), m_pShellItems(NULL)
{
}

CNsePropertyPage::~CNsePropertyPage()
{
   m_pSheet = NULL;
}

// Operations

HRESULT CNsePropertyPage::SetItemReferences(CPropSheetExt* pSheet, IShellItemArray* pShellItems, CNseItemArray* pNseItems)
{
   m_pSheet = pSheet;
   m_pNseItems = pNseItems;
   m_pShellItems = pShellItems;
   return S_OK;
}

HRESULT CNsePropertyPage::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
   ATLTRACE(L"CNsePropertyPage::AddPages\n");
   ATLASSERT(m_bstrCaption.Length()>0);      // Needs to be initialized in subclass constructor
   ATLASSERT(m_uIDD!=0);                     // Needs to be initialized in subclass constructor

   PROPSHEETPAGE psp = { 0 };
   psp.dwSize        = sizeof(psp);
   psp.dwFlags       = PSP_USETITLE | PSP_USECALLBACK;
   psp.hInstance     = _pModule->GetResourceInstance();
   psp.pszTemplate   = MAKEINTRESOURCE(m_uIDD);
   psp.hIcon         = 0;
   psp.pszTitle      = m_bstrCaption;
   psp.pfnDlgProc    = PageDlgProc;
   psp.pcRefParent   = NULL;
   psp.pfnCallback   = PropSheetPageProc;
   psp.lParam        = reinterpret_cast<LPARAM>(this);
   HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(&psp);            
   if( hPage == NULL ) return E_OUTOFMEMORY;
   if( pfnAddPage(hPage, lParam) == FALSE ) {
      ::DestroyPropertySheetPage(hPage);
      return E_FAIL;
   }

   return MAKE_HRESULT(SEVERITY_SUCCESS, 0, m_uStartPageIndex);
}

void CNsePropertyPage::SetModified(BOOL bChanged /*= TRUE*/)
{
   ATLASSERT(::IsWindow(m_hWnd));
   ATLASSERT(GetParent()!=NULL);
   ::SendMessage(GetParent(), bChanged ? PSM_CHANGED : PSM_UNCHANGED, (WPARAM) m_hWnd, 0L);
}

// Implementation

BOOL CNsePropertyPage::_SetLabelTextFromRes(UINT uResID, UINT uLabelID)
{
   CComBSTR bstr;
   bstr.LoadString(uLabelID);
   return SetDlgItemText((int)uResID, bstr) > 0;
}

BOOL CNsePropertyPage::_SetValueTextFromPKey(UINT uResID, REFPROPERTYKEY pkey, CComPropVariant& vValue)
{
   WCHAR wszText[300] = { 0 };
   ::PSFormatForDisplay(pkey, vValue, PDFF_DEFAULT, wszText, lengthof(wszText));
   return SetDlgItemText((int)uResID, wszText) > 0;
}

BOOL CNsePropertyPage::_SetLabelTextFromPKey(UINT uResID, REFPROPERTYKEY pkey, LPCTSTR pstrPrefix, LPCTSTR pstrPostfix)
{
   CComPtr<IPropertyDescription> spInfo;
   ::PSGetPropertyDescription(pkey, IID_PPV_ARGS(&spInfo));
   if( spInfo == NULL ) return FALSE;
   CCoTaskString strName;
   spInfo->GetDisplayName(&strName);
   if( strName.IsEmpty() ) return FALSE;
   WCHAR wszLabel[200] = { 0 };
   ::wnsprintf(wszLabel, lengthof(wszLabel) - 1, L"%s%s%s", pstrPrefix, static_cast<LPCWSTR>(strName), pstrPostfix);
   return SetDlgItemText((int)uResID, wszLabel) > 0;
}

// Callbacks

UINT CALLBACK CNsePropertyPage::PropSheetPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
   ATLTRACE(L"CNsePropertyPage::PropSheetPageProc  msg=%ld\n", uMsg);
   CNsePropertyPage* pT = reinterpret_cast<CNsePropertyPage*>(ppsp->lParam);
   ATLASSERT(pT);
   ATLASSERT(pT->m_pSheet);
   if( pT == NULL ) return 0;
   if( pT->m_pSheet == NULL ) return 0;
   switch( uMsg ) {
   case PSPCB_CREATE:   return 1;
   case PSPCB_ADDREF:   pT->m_pSheet->AddRef(); break;
   case PSPCB_RELEASE:  pT->m_pSheet->Release(); break;
   }
   return 0;
}

INT_PTR CALLBACK CNsePropertyPage::PageDlgProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{    
   CNsePropertyPage* pT = reinterpret_cast<CNsePropertyPage*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
   LRESULT lRes = 0;
   if( uMessage == WM_INITDIALOG ) {
      pT = reinterpret_cast<CNsePropertyPage*>(reinterpret_cast<LPPROPSHEETPAGE>(lParam)->lParam);
      ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) pT);
      // Set the window handle
      pT->m_hWnd = hWnd;
      lRes = 1;
   }
   if( pT == NULL ) {
      // The first message might be WM_SETFONT not WM_INITDIALOG as expected.
      // Don't invoke default WindowProc since it will recurse!
      return FALSE;
   }
   MSG msg = { hWnd, uMessage, wParam, lParam, 0, { 0, 0 } };
   const MSG* pOldMsg = pT->m_pCurrentMsg;
   pT->m_pCurrentMsg = &msg;
   // Pass to the message map to process
   BOOL bRet = pT->ProcessWindowMessage(hWnd, uMessage, wParam, lParam, lRes, 0);
   // Restore saved value for the current message
   ATLASSERT(pT->m_pCurrentMsg==&msg);
   pT->m_pCurrentMsg = pOldMsg;
   // Set result if message was handled
   if( bRet ) {
      switch( uMessage ) {
      case WM_INITDIALOG:
      case WM_VKEYTOITEM:
      case WM_CHARTOITEM:
      case WM_COMPAREITEM:
      case WM_QUERYDRAGICON:
      case WM_CTLCOLORDLG:
      case WM_CTLCOLORBTN:
      case WM_CTLCOLOREDIT:
      case WM_CTLCOLORSTATIC:
      case WM_CTLCOLORMSGBOX:
      case WM_CTLCOLORLISTBOX:
      case WM_CTLCOLORSCROLLBAR:
         return (int) lRes;
      }
      ::SetWindowLongPtr(hWnd, DWLP_MSGRESULT, lRes);
      return TRUE;
   }
   if( uMessage == WM_NCDESTROY ) {
      // Clear out window handle
      ::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0L);
      pT->m_hWnd = NULL;
   }
   return FALSE;
}

