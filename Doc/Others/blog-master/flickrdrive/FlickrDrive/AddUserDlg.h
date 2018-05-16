
#pragma once

#include "resource.h"

#include "rest.h"


///////////////////////////////////////////////////////////////////////////////
// CWelcomeUserDlg

class CWelcomeUserDlg : public CDialogImpl<CWelcomeUserDlg>
{
public:
   enum { IDD = IDD_WELCOMEUSER };

   HFONT m_hFont;
   HBITMAP m_hLogoBmp;
   HICON m_hDriveIcon;
   CString m_sUsername;
   BOOL m_bAddFriends;
   BOOL m_bAddFamily;

   BEGIN_MSG_MAP(CWelcomeUserDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
      COMMAND_ID_HANDLER(IDOK, OnOK)
      COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
      COMMAND_CODE_HANDLER(EN_CHANGE, OnEditChange)
   END_MSG_MAP()

   // Message handlers
   
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};


///////////////////////////////////////////////////////////////////////////////
// CAddUserDlg

class CAddUserDlg : public CDialogImpl<CAddUserDlg>
{
public:
   enum { IDD = IDD_ADDUSER };

   HFONT m_hFont;
   HBITMAP m_hLogoBmp;
   HICON m_hDriveIcon;
   TFlickrAccountType m_AccountType;
   CString m_sUsername;

   BEGIN_MSG_MAP(CAddUserDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
      COMMAND_ID_HANDLER(IDOK, OnOK)
      COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
      COMMAND_CODE_HANDLER(EN_CHANGE, OnEditChange)
   END_MSG_MAP()

   // Message handlers
   
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};


///////////////////////////////////////////////////////////////////////////////
// Definitions

UINT DoWebAuthentication(HWND hWnd, LPCTSTR pstrURL);
UINT DoWebAuthenticationCompleted(HWND hWnd, LPCTSTR pstrURL);

