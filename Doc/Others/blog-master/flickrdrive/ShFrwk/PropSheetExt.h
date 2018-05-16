/////////////////////////////////////////////////////////////////////////////
// Shell Extension classes
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2009 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#pragma once

class CShellFolder;
class CNsePropertyPage;


///////////////////////////////////////////////////////////////////////////////
// CPropSheetExt

class ATL_NO_VTABLE CPropSheetExt : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public CComCoClass<CPropSheetExt, &CLSID_PropertySheet>,
   public IShellPropSheetExt,
   public IShellExtInit
{
public:
   CNsePropertyPage* m_pPage;                    // Reference to the embedded page
   CComPtr<IDataObject> m_spDataObject;          // Reference to selection
   CInterfaceArray<IShellFolder> m_aFolders;     // Keeps the IShellFolder parents alive
   CComPtr<IShellItemArray> m_spItems;           // Keeps the IShellItem items alive
   CNseItemArray m_aChildren;                    // List of NSE Items to summerize

   BEGIN_COM_MAP(CPropSheetExt)
      COM_INTERFACE_ENTRY(IShellExtInit)
      COM_INTERFACE_ENTRY(IShellPropSheetExt)
   END_COM_MAP()

   CPropSheetExt();
   void FinalRelease();

   static HRESULT WINAPI UpdateRegistry(BOOL bRegister) throw();

   // IShellPropSheetExt

   STDMETHOD(AddPages)(LPFNSVADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);       
   STDMETHOD(ReplacePage)(EXPPS uPageID, LPFNSVADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam);

   // IShellExtInit

   STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObject, HKEY hkeyProgID);
};


///////////////////////////////////////////////////////////////////////////////
// CNsePropertyPage

class CNsePropertyPage : public CWindow, public CMessageMap
{
public:
   CPropSheetExt* m_pSheet;                      // Reference to PropSheet Extension
   CNseItemArray* m_pNseItems;                   // List of NSE Items shown on this page
   IShellItemArray* m_pShellItems;               // List of Shell Items shown on this page
   UINT m_uIDD;                                  // ID of dialog; must initialize in constructor
   CComBSTR m_bstrCaption;                       // Title (tab) of page; must initialize in constructor
   USHORT m_uStartPageIndex;                     // Start Page index (0=default, 1=me)
   const MSG* m_pCurrentMsg;                     // Reference to current window message

   CNsePropertyPage();
   virtual ~CNsePropertyPage();

   // Operations

   HRESULT AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
   HRESULT SetItemReferences(CPropSheetExt* pSheet, IShellItemArray* pShellItems, CNseItemArray* paItems);
   void SetModified(BOOL bChanged = TRUE);

   // Implementation

   BOOL _SetLabelTextFromRes(UINT uResID, UINT uLabelID);
   BOOL _SetValueTextFromPKey(UINT uResID, REFPROPERTYKEY pkey, CComPropVariant& vValue);
   BOOL _SetLabelTextFromPKey(UINT uResID, REFPROPERTYKEY pkey, LPCTSTR pstrPrefix, LPCTSTR pstrPostfix);

   // Callbacks

   virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
      LRESULT& lResult, DWORD dwMsgMapID) = 0;

   static UINT CALLBACK PropSheetPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
   static INT_PTR CALLBACK PageDlgProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
};


///////////////////////////////////////////////////////////////////////////////
// CNseFileRootPropertyPage

class CNseFileRootPropertyPage : public CNsePropertyPage
{
public:
   DWORD m_dwPercent;                            // Pie percent value displayed

   CNseFileRootPropertyPage();

   BEGIN_MSG_MAP(CNseFileRootPropertyPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
   END_MSG_MAP()

   // Message handlers
   
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


///////////////////////////////////////////////////////////////////////////////
// CNseFileItemPropertyPage

class CNseFileItemPropertyPage : public CNsePropertyPage
{
public:
   enum { WM_FILECOUNT_UPDATE = WM_APP + 120 };

   bool m_bAttribsClicked;                       // User changed file-attributes?

   CNseFileItemPropertyPage();

   BEGIN_MSG_MAP(CNseFileItemPropertyPage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_FILECOUNT_UPDATE, OnFileCountUpdate)
      COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
      NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
   END_MSG_MAP()

   // Message handlers

   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnFileCountUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   // Implementation

   HRESULT _ApplyProperties();

   // Static members

   static DWORD WINAPI CountingThread(LPVOID pData);
};

