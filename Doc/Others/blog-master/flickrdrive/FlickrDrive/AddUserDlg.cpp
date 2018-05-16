
#include "StdAfx.h"

#include "AddUserDlg.h"

#include <commoncontrols.h>


///////////////////////////////////////////////////////////////////////////////
// CWelcomeUserDlg

LRESULT CWelcomeUserDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   CenterWindow();
   // Create font, logo, set dialog controls
   NONCLIENTMETRICS ncm = { sizeof(ncm) };
   ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
   ncm.lfMessageFont.lfWeight = FW_BOLD;
   ncm.lfMessageFont.lfHeight -= 2;;
   m_hFont = ::CreateFontIndirect(&ncm.lfMessageFont);
   SendDlgItemMessage(IDC_DESCRIPTION_TITLE, WM_SETFONT, (WPARAM) m_hFont);
   m_hLogoBmp = ::LoadBitmap(_pModule->GetResourceInstance(), MAKEINTRESOURCE(IDB_FLICKRDRIVE1));
   SHFILEINFO sfi = { 0 }; ::SHGetFileInfo(_T("C:\\"), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
   CComPtr<IImageList> spImageList;
   m_hDriveIcon = NULL;
   ::SHGetImageList(SHIL_JUMBO, IID_PPV_ARGS(&spImageList));
   if( spImageList != NULL ) spImageList->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &m_hDriveIcon);
   SetDlgItemText(IDC_USERNAME, _T(""));
   GotoDlgCtrl(GetDlgItem(IDC_USERNAME));
   return 0;
}

LRESULT CWelcomeUserDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ::DeleteObject(m_hFont);
   ::DeleteObject(m_hLogoBmp);
   ::DestroyIcon(m_hDriveIcon);
   bHandled = FALSE;
   return 0;
}

LRESULT CWelcomeUserDlg::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   LPDRAWITEMSTRUCT lpDIS = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
   if( wParam == IDC_LOGO )
   {
      // Fill white background
      COLORREF clrOld = ::SetBkColor(lpDIS->hDC, RGB(255,255,255));
      ::ExtTextOut(lpDIS->hDC, 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL);
      ::SetBkColor(lpDIS->hDC, clrOld);
      // Paint logo bitmap
      BITMAP BmpInfo = { 0 };
      ::GetObject(m_hLogoBmp, sizeof(BITMAP), &BmpInfo);
      RECT rcClient = { 0 };
      ::GetClientRect(lpDIS->hwndItem, &rcClient);
      HDC hDcCompat = ::CreateCompatibleDC(lpDIS->hDC);
      HBITMAP hBmpCompat = ::CreateCompatibleBitmap(hDcCompat, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
      HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hDcCompat, m_hLogoBmp);
      ::BitBlt(lpDIS->hDC, 10, 10, BmpInfo.bmWidth, BmpInfo.bmHeight, hDcCompat, 0, 0, SRCCOPY);
      ::SelectObject(hDcCompat, hOldBitmap);
      ::DeleteObject(hBmpCompat);
      ::DrawIconEx(lpDIS->hDC, lpDIS->rcItem.right - 120, -30, m_hDriveIcon, 96, 96, 0, NULL, DI_NORMAL);
   }
   if( wParam == IDC_COPYNOTICE )
   {
      CString sLabel;
      sLabel.LoadString(IDS_COPYNOTICE);
      // Fill background and print text
      RECT rcText = lpDIS->rcItem;
      COLORREF clrOld = ::SetBkColor(lpDIS->hDC, RGB(205,205,205));
      ::ExtTextOut(lpDIS->hDC, 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL);
      ::SetTextColor(lpDIS->hDC, RGB(30,30,30));
      ::InflateRect(&rcText, -4, -4);
      ::ExtTextOut(lpDIS->hDC, 4, 4, ETO_OPAQUE, &rcText, sLabel, sLabel.GetLength(), NULL);
      ::SetBkColor(lpDIS->hDC, clrOld);
   }
   return 0;
}

LRESULT CWelcomeUserDlg::OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   GetDlgItem(IDOK).EnableWindow(GetDlgItem(IDC_USERNAME).GetWindowTextLength() > 0);
   return 0;
}

LRESULT CWelcomeUserDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   GetDlgItemText(IDC_USERNAME, m_sUsername);
   m_bAddFamily = (IsDlgButtonChecked(IDC_ADD_FAMILY) == BST_CHECKED);
   m_bAddFriends = (IsDlgButtonChecked(IDC_ADD_FRIENDS) == BST_CHECKED);
   HRESULT Hr = _ShellModule.Rest.CheckAccount(m_sUsername);
   if( FAILED(Hr) ) return DoDisplayErrorMessage(m_hWnd, IDS_ERR_USERLOOKUP, Hr);
   EndDialog(IDOK);
   return 0;
}

LRESULT CWelcomeUserDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   EndDialog(IDCANCEL);
   return 0;
}


///////////////////////////////////////////////////////////////////////////////
// CAddUserDlg

LRESULT CAddUserDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   CenterWindow();
   // Create font, logo, set dialog controls
   NONCLIENTMETRICS ncm = { sizeof(ncm) };
   ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
   ncm.lfMessageFont.lfWeight = FW_BOLD;
   m_hFont = ::CreateFontIndirect(&ncm.lfMessageFont);
   SendDlgItemMessage(IDC_DESCRIPTION_LABEL, WM_SETFONT, (WPARAM) m_hFont);
   m_hLogoBmp = ::LoadBitmap(_pModule->GetResourceInstance(), MAKEINTRESOURCE(IDB_FLICKRDRIVE1));
   SHFILEINFO sfi = { 0 }; ::SHGetFileInfo(_T("C:\\"), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
   CComPtr<IImageList> spImageList;
   m_hDriveIcon = NULL;
   ::SHGetImageList(SHIL_JUMBO, IID_PPV_ARGS(&spImageList));
   if( spImageList != NULL ) spImageList->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &m_hDriveIcon);
   CString str;
   str.LoadString(IDS_ACCOUNT_GUEST);
   ATLASSERT(FLICKRFS_ACCOUNT_GUEST==0);
   SendDlgItemMessage(IDC_ACCOUNT, CB_ADDSTRING, (WPARAM) 0, (LPARAM) static_cast<LPCTSTR>(str));
   str.LoadString(IDS_ACCOUNT_FULLPERM);
   SendDlgItemMessage(IDC_ACCOUNT, CB_ADDSTRING, (WPARAM) 0, (LPARAM) static_cast<LPCTSTR>(str));
   SendDlgItemMessage(IDC_ACCOUNT, CB_SETCURSEL, 0, 0);
   SetDlgItemText(IDC_USERNAME, _T(""));
   GotoDlgCtrl(GetDlgItem(IDC_USERNAME));
   return 0;
}

LRESULT CAddUserDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ::DeleteObject(m_hFont);
   ::DeleteObject(m_hLogoBmp);
   bHandled = FALSE;
   return 0;
}

LRESULT CAddUserDlg::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   LPDRAWITEMSTRUCT lpDIS = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
   if( wParam == IDC_LOGO )
   {
      // Fill white background
      COLORREF clrOld = ::SetBkColor(lpDIS->hDC, RGB(255,255,255));
      ::ExtTextOut(lpDIS->hDC, 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL);
      ::SetBkColor(lpDIS->hDC, clrOld);
      // Paint logo bitmap
      BITMAP BmpInfo = { 0 };
      ::GetObject(m_hLogoBmp, sizeof(BITMAP), &BmpInfo);
      RECT rcClient = { 0 };
      ::GetClientRect(lpDIS->hwndItem, &rcClient);
      HDC hDcCompat = ::CreateCompatibleDC(lpDIS->hDC);
      HBITMAP hBmpCompat = ::CreateCompatibleBitmap(hDcCompat, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
      HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hDcCompat, m_hLogoBmp);
      ::BitBlt(lpDIS->hDC, 10, 10, BmpInfo.bmWidth, BmpInfo.bmHeight, hDcCompat, 0, 0, SRCCOPY);
      ::SelectObject(hDcCompat, hOldBitmap);
      ::DeleteObject(hBmpCompat);
      ::DrawIconEx(lpDIS->hDC, lpDIS->rcItem.right - 120, -30, m_hDriveIcon, 96, 96, 0, NULL, DI_NORMAL);
   }
   return 0;
}

LRESULT CAddUserDlg::OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   GetDlgItem(IDOK).EnableWindow(GetDlgItem(IDC_USERNAME).GetWindowTextLength() > 0);
   return 0;
}

LRESULT CAddUserDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   m_AccountType = (TFlickrAccountType) SendDlgItemMessage(IDC_ACCOUNT, CB_GETCURSEL);
   GetDlgItemText(IDC_USERNAME, m_sUsername);
   HRESULT Hr = _ShellModule.Rest.CheckAccount(m_sUsername);
   if( FAILED(Hr) ) return DoDisplayErrorMessage(m_hWnd, IDS_ERR_USERLOOKUP, Hr);
   EndDialog(IDOK);
   return 0;
}

LRESULT CAddUserDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   EndDialog(IDCANCEL);
   return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Authentication dialogs

/**
 * Ask user to web authenticate.
 * Inform user that he must authenticate access for FlickrDrive on the Yahoo Flickr
 * website. Open an Internet Browser to allow the user to complete the authentication.
 */
UINT DoWebAuthentication(HWND hWnd, LPCTSTR pstrURL)
{
   TASKDIALOGCONFIG tdc = { 0 };
   tdc.cbSize = sizeof(TASKDIALOGCONFIG);
   tdc.hInstance = _pModule->GetResourceInstance();
   tdc.hwndParent = hWnd;
   tdc.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
   tdc.pszWindowTitle = MAKEINTRESOURCE(IDS_CAPTION_MESSAGE);
   tdc.pszMainIcon = MAKEINTRESOURCE(IDI_FDRIVE1);
   tdc.pszMainInstruction = MAKEINTRESOURCE(IDS_AUTH_TITLE);
   tdc.pszContent = MAKEINTRESOURCE(IDS_AUTH_DESCRIPTION);
   TASKDIALOG_BUTTON aButtons[] = {
      { 200, MAKEINTRESOURCE(IDS_AUTH_OK) },
      { 201, MAKEINTRESOURCE(IDS_AUTH_CANCEL) },
   };
   tdc.cButtons = lengthof(aButtons);
   tdc.pButtons = aButtons;
   int iButton = 0;
   ::TaskDialogIndirect(&tdc, &iButton, NULL, NULL);
   if( iButton == 200 ) {
      ::ShellExecute(hWnd, _T("open"), pstrURL, NULL, NULL, SW_SHOW);
      ::Sleep(3000);
   }
   return iButton == 200 ? IDOK : IDCANCEL;
}

static HRESULT CALLBACK TaskDialogCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
   switch( msg ) {
   case TDN_HYPERLINK_CLICKED:
      ::ShellExecute(hWnd, _T("open"), reinterpret_cast<LPCTSTR>(lpRefData), NULL, NULL, SW_SHOW);
      break;
   }
   return 0;
}

/**
 * Prompt to complete web authentication.
 * Since we cannot reliably wait for the Internet Browser and the user completing the 
 * web authentication process, we'll show a prompt and allow the user to dismiss it
 * when he completes the authentication.
 */
UINT DoWebAuthenticationCompleted(HWND hWnd, LPCTSTR pstrURL)
{
   TASKDIALOGCONFIG tdc = { 0 };
   tdc.cbSize = sizeof(TASKDIALOGCONFIG);
   tdc.hInstance = _pModule->GetResourceInstance();
   tdc.hwndParent = hWnd;
   tdc.pfCallback = TaskDialogCallback;
   tdc.lpCallbackData = (LONG_PTR) pstrURL;
   tdc.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW | TDF_ENABLE_HYPERLINKS;
   tdc.pszWindowTitle = MAKEINTRESOURCE(IDS_CAPTION_MESSAGE);
   tdc.pszMainIcon = MAKEINTRESOURCE(IDI_FDRIVE1);
   tdc.pszMainInstruction = MAKEINTRESOURCE(IDS_AUTHCOMPL_TITLE);
   tdc.pszContent = MAKEINTRESOURCE(IDS_AUTHCOMPL_DESCRIPTION);
   tdc.pszFooter = MAKEINTRESOURCE(IDS_AUTHCOMPL_FOOTER);
   TASKDIALOG_BUTTON aButtons[] = {
      { 200, MAKEINTRESOURCE(IDS_AUTHCOMPL_DONE) },
      { 201, MAKEINTRESOURCE(IDS_AUTHCOMPL_CANCEL) },
   };
   tdc.cButtons = lengthof(aButtons);
   tdc.pButtons = aButtons;
   int iButton = 0;
   ::TaskDialogIndirect(&tdc, &iButton, NULL, NULL);
   return iButton == 200 ? IDOK : IDCANCEL;
}


///////////////////////////////////////////////////////////////////////////////
// Error Message dialog

/**
 * Display error message text.
 * Format a HRESULT as an error message prompt.
 */
HRESULT DoDisplayErrorMessage(HWND hWnd, UINT nTitle, HRESULT Hr)
{
   HRESULT HrOrig = Hr;
   if( HRESULT_FACILITY(Hr) == FACILITY_WINDOWS ) Hr = HRESULT_CODE(Hr);
   if( HRESULT_FACILITY(Hr) == FACILITY_WIN32 ) Hr = HRESULT_CODE(Hr);
   // Format error message; try various sources
   LPTSTR pstr = NULL;
   if( pstr == NULL ) ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, Hr, 0, (LPTSTR) &pstr, 0, NULL);
   static LPCTSTR s_ppstrModules[] = { _T("winhttp"), _T("mswsock"), _T("netmsg"), _T("crypt32") };
   for( int i = 0; pstr == NULL && i < lengthof(s_ppstrModules); i++ ) {
      ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, ::GetModuleHandle(s_ppstrModules[i]), Hr, 0, (LPTSTR) &pstr, 0, NULL);
   }
   bool bReleaseStr = (pstr != NULL);
   CString sTemp(MAKEINTRESOURCE(IDS_ERR_GENERIC));
   CComBSTR bstrErr;
   CComPtr<IErrorInfo> spErrInfo;
   ::GetErrorInfo(0, &spErrInfo);
   if( spErrInfo != NULL ) spErrInfo->GetDescription(&bstrErr);
   if( pstr == NULL ) pstr = bstrErr.m_str;
   if( pstr == NULL ) pstr = const_cast<LPTSTR>(static_cast<LPCTSTR>(sTemp));
   while( pstr[0] != '\0' && _tcschr(_T("\r\n "), pstr[_tcslen(pstr) - 1]) != NULL ) pstr[_tcslen(pstr) - 1] = '\0';
   // Display message as a TaskDialog prompt
   CString sMessage, sFooter, sExtra;
   if( Hr >= WINHTTP_ERROR_BASE && Hr <= WINHTTP_ERROR_LAST ) sExtra.LoadString(IDS_ERR_WINHTTP);
   if( Hr == ERROR_BAD_USER_PROFILE ) sExtra.LoadString(IDS_ERR_NSID);
   sMessage.Format(IDS_ERRMSG_DESCRIPTION, pstr, sExtra);
   sFooter.Format(IDS_ERRMSG_HRESULT, HrOrig, bstrErr);
   if( bReleaseStr ) ::LocalFree(pstr);
   TASKDIALOGCONFIG tdc = { 0 };
   tdc.cbSize = sizeof(TASKDIALOGCONFIG);
   tdc.hInstance = _pModule->GetResourceInstance();
   tdc.hwndParent = hWnd;
   tdc.pszMainIcon = TD_ERROR_ICON;
   tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
   tdc.dwCommonButtons = TDCBF_OK_BUTTON;
   tdc.pszWindowTitle = MAKEINTRESOURCE(IDS_CAPTION_ERROR);
   tdc.pszMainInstruction = MAKEINTRESOURCE(nTitle);
   tdc.pszContent = sMessage;
   tdc.pszFooter = sFooter;
   ::TaskDialogIndirect(&tdc, NULL, NULL, NULL);
   return HrOrig;
}

