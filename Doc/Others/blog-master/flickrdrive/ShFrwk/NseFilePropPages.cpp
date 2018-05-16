
#include "stdafx.h"
#include "shfrwkres.h"

#include "NseFileSystem.h"
#include "ShellFolder.h"
#include "PropSheetExt.h"

#pragma warning( disable : 4510 4610 )


///////////////////////////////////////////////////////////////////////////////
// CPropertyChangeUInt32

class ATL_NO_VTABLE CPropertyChangeUInt32 : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public IPropertyChange
{
public:
   enum { PKA_BOR = 1, PKA_BAND, PKA_PLUS, PKA_MINUS, PKA_BNOTAND };

   PROPERTYKEY m_pkey;
   int m_iOperation;
   UINT m_uChangeValue;

   BEGIN_COM_MAP(CPropertyChangeUInt32)
      COM_INTERFACE_ENTRY(IPropertyChange)
      COM_INTERFACE_ENTRY(IObjectWithPropertyKey)
   END_COM_MAP()

   // Operations

   HRESULT Init(REFPROPERTYKEY pkey, int iOperation, ULONG uChange)
   {
      m_pkey = pkey;
      m_iOperation = iOperation;
      m_uChangeValue = uChange;
      return S_OK;
   }

   // IPropertyChange

   STDMETHOD(ApplyToPropVariant)(REFPROPVARIANT propvarIn, PROPVARIANT* ppropvarOut)
   {
      ULONG uValue = 0;
      HR( ::PropVariantToUInt32(propvarIn, &uValue) );
      switch( m_iOperation ) {
      case PKA_BOR:     uValue |= m_uChangeValue; break;
      case PKA_BAND:    uValue &= m_uChangeValue; break;
      case PKA_PLUS:    uValue += m_uChangeValue; break;
      case PKA_MINUS:   uValue -= m_uChangeValue; break;
      case PKA_BNOTAND: uValue &= ~m_uChangeValue; break;
      }
      return ::InitPropVariantFromUInt32(uValue, ppropvarOut);
   }

   // IObjectWithPropertyKey

   STDMETHOD(SetPropertyKey)(REFPROPERTYKEY pkey)
   {
      m_pkey = pkey;
      return S_OK;
   }

   STDMETHOD(GetPropertyKey)(PROPERTYKEY* pRetVal)
   {
      *pRetVal = m_pkey;
      return S_OK;
   }
};


//////////////////////////////////////////////////////////////////////
// CNseFileRootPropertyPage

CNseFileRootPropertyPage::CNseFileRootPropertyPage()
{
   m_bstrCaption.LoadString(IDS_NSE_VOLUME);
   if( m_bstrCaption.Length() == 0 ) m_bstrCaption.LoadString(IDS_PAGE_VOLUME);
   m_uIDD = IDD_PROPPAGE_VOLUME;
   m_uStartPageIndex = 1;
   m_dwPercent = 0;
}

// Message handlers

LRESULT CNseFileRootPropertyPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ATLASSERT(m_pSheet);
   ATLASSERT(m_pNseItems);
   CNseItem* pNseItem = (*m_pNseItems)[0];
   CComPtr<IShellItem> spShellItem;
   CComPtr<IPropertyStore> spStore;
   if( SUCCEEDED( m_pShellItems->GetItemAt(0, &spShellItem) ) ) {
      spShellItem->BindToHandler(NULL, BHID_PropertyStore, IID_PPV_ARGS(&spStore));
   }
   // Translate labels using property-key presentation text...
   struct {
      REFPROPERTYKEY pkey;
      UINT uRes;
      LPCTSTR pszPrefix;
      LPCTSTR pszPostfix;
   } aLabels[] = {
      { PKEY_ItemNameDisplay,     IDC_NAME_LABEL,          L"&",               L":" },
      { PKEY_ItemTypeText,        IDC_TYPE_LABEL,          L"",                L":" },
      { PKEY_Volume_FileSystem,   IDC_FILESYSTEM_LABEL,    L"",                L":" },
      { PKEY_IsFlagged,           IDC_FLAGS_LABEL,         L"",                L":" },      
      { PKEY_FileAllocationSize,  IDC_USEDSPACE_LABEL,     L"",                L":" },
      { PKEY_FreeSpace,           IDC_FREESPACE_LABEL,     L"",                L":" },
      { PKEY_Capacity,            IDC_CAPACITY_LABEL,      L"",                L":" },
   };
   for( int i = 0; i < lengthof(aLabels); i++ ) {
      _SetLabelTextFromPKey(aLabels[i].uRes, aLabels[i].pkey, aLabels[i].pszPrefix, aLabels[i].pszPostfix);
   }
   // Gather information...
   CComPropVariant vName, vType, vFileSystem, vFlags, vFreeSpace, vUsedSpace, vCapacity;
   struct {
      UINT uRes;
      REFPROPERTYKEY pkeyFormat;
      CComPropVariant& vValue;
   } aValues[] = {
      { IDC_NAME,              PKEY_ItemNameDisplay,       vName },
      { IDC_TYPE,              PKEY_ItemTypeText,          vType },
      { IDC_FILESYSTEM,        PKEY_Volume_FileSystem,     vFileSystem },
      { IDC_FLAGS,             PKEY_FlagStatusText,        vFlags },
      { IDC_FREESPACE,         PKEY_FreeSpace,             vFreeSpace },
      { IDC_USEDSPACE,         PKEY_FileAllocationSize,    vUsedSpace },
      { IDC_USEDSPACE,         PKEY_Size,                  vUsedSpace },
      { IDC_CAPACITY,          PKEY_Capacity,              vCapacity },
   };
   for( int i = 0; i < lengthof(aValues); i++ ) {
      CComPropVariant v;
      pNseItem->GetProperty(aValues[i].pkeyFormat, v);
      if( v.vt != VT_EMPTY ) aValues[i].vValue = v;
   }
   for( int i = 0; spStore != NULL && i < lengthof(aValues); i++ ) {
      CComPropVariant v;
      spStore->GetValue(aValues[i].pkeyFormat, &v);
      if( v.vt != VT_EMPTY ) aValues[i].vValue = v;
   }
   if( vCapacity.vt == VT_NULL || vCapacity.vt == VT_EMPTY ) {
      // No Capacity property? Let's assume it's a file-class NSE and
      // the virtual disk has the entire drive at its feet.
      CCoTaskString strFile;
      spShellItem->GetDisplayName(SIGDN_FILESYSPATH, &strFile);
      WCHAR wszDrive[4] = { 0 };
      wcsncpy(wszDrive, strFile, 3);
      ULARGE_INTEGER ullFreeBytesAvailableToCaller = { 0 };
      ::GetDiskFreeSpaceEx(wszDrive, &ullFreeBytesAvailableToCaller, NULL, NULL);
      vCapacity = vFreeSpace = ullFreeBytesAvailableToCaller.QuadPart;
      vCapacity.uhVal.QuadPart += vUsedSpace.uhVal.QuadPart;
   }
   // Set information...
   for( int i = 0; i < lengthof(aValues); i++ ) {
      _SetValueTextFromPKey(aValues[i].uRes, aValues[i].pkeyFormat, aValues[i].vValue);
   }
   if( vCapacity.uhVal.QuadPart != 0 ) {
      m_dwPercent = (DWORD)( (double) vUsedSpace.uhVal.QuadPart / (double) vCapacity.uhVal.QuadPart * 100.0 );
      if( m_dwPercent > 100 ) m_dwPercent = 100;
   }
   bHandled = FALSE;
   return 0;
}

LRESULT CNseFileRootPropertyPage::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   LPDRAWITEMSTRUCT lpDIS = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
   if( wParam == IDC_USEDSPACE_BLOCK )
   {
      COLORREF clrOld = ::SetBkColor(lpDIS->hDC, RGB(0,0,255));
      ::ExtTextOut(lpDIS->hDC, 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL);
      ::SetBkColor(lpDIS->hDC, clrOld);   
   }
   if( wParam == IDC_FREESPACE_BLOCK )
   {
      COLORREF clrOld = ::SetBkColor(lpDIS->hDC, RGB(255,0,255));
      ::ExtTextOut(lpDIS->hDC, 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL);
      ::SetBkColor(lpDIS->hDC, clrOld);   
   }
   if( wParam == IDC_PIE )
   {
      HDC hDC = lpDIS->hDC;
      const RECT& rc = lpDIS->rcItem;
      int dwWidth = (rc.right - rc.left) / 2;
      int dwHeight = (rc.bottom - rc.top) / 2;
      int nX = (int) dwWidth;
      int nY = (int) dwHeight;
      DWORD dwRadius = dwWidth < dwHeight ? dwWidth : dwHeight;
      float fStartAngle = 0.0f;
      float fSweepAngle = (float) m_dwPercent * 3.6f;     
      HBRUSH hbrushBlue = ::CreateSolidBrush(RGB(0,0,255)); 
      HBRUSH hbrushRed = ::CreateSolidBrush(RGB(255,0,255)); 
      HBRUSH hbrushOld = (HBRUSH) ::SelectObject(hDC, hbrushBlue); 
      ::BeginPath(hDC);
      ::MoveToEx(hDC, nX, nY, (LPPOINT) NULL); 
      ::AngleArc(hDC, nX, nY, dwRadius, fStartAngle, fSweepAngle); 
      ::LineTo(hDC, nX, nY); 
      ::EndPath(hDC); 
      ::StrokeAndFillPath(hDC); 
      ::SelectObject(hDC, hbrushRed); 
      ::BeginPath(hDC);
      ::MoveToEx(hDC, nX, nY, (LPPOINT) NULL); 
      ::AngleArc(hDC, nX, nY, dwRadius, fSweepAngle, 360.0f - fSweepAngle); 
      ::EndPath(hDC); 
      ::StrokeAndFillPath(hDC); 
      ::SelectObject(hDC, hbrushOld);
      ::DeleteObject(hbrushBlue);
      ::DeleteObject(hbrushRed);
   }
   bHandled = FALSE;
   return 0;
}


//////////////////////////////////////////////////////////////////////
// CNseFileItemPropertyPage

typedef struct tagCOUNTERDATA
{
   HWND hWnd;
   CPidl pidls[200];
} COUNTERDATA;

CNseFileItemPropertyPage::CNseFileItemPropertyPage()
{
   m_bstrCaption.LoadString(IDS_PAGE_ITEM);
   m_uIDD = IDD_PROPPAGE_ITEM;
   m_uStartPageIndex = 1;
}

// Message handlers

LRESULT CNseFileItemPropertyPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ATLASSERT(m_pSheet);
   ATLASSERT(m_pNseItems);
   CNseItem* pNseItem = (*m_pNseItems)[0];
   CComPtr<IShellItem> spShellItem;
   CComPtr<IPropertyStore> spStore;
   CComPtr<IExtractIcon> spExtractIcon;
   if( SUCCEEDED( m_pShellItems->GetItemAt(0, &spShellItem) ) ) {
      spShellItem->BindToHandler(NULL, BHID_PropertyStore, IID_PPV_ARGS(&spStore));
      spShellItem->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARGS(&spExtractIcon));
   }
   DWORD dwItemCount = 0;
   m_pShellItems->GetCount(&dwItemCount);
   // Translate labels using property-key presentation text...
   struct {
      REFPROPERTYKEY pkey;
      UINT uRes;
      LPCTSTR pszPrefix;
      LPCTSTR pszPostfix;
   } aLabels[] = {
      { PKEY_Comment,                IDC_COMMENT_LABEL,         L"&",               L":" },
      { PKEY_ItemTypeText,           IDC_TYPE_LABEL,            L"",                L":" },
      { PKEY_Size,                   IDC_FILESIZE_LABEL,        L"",                L":" },
      { PKEY_TotalFileSize,          IDC_FILESIZEONDISK_LABEL,  L"",                L":" },      
      { PKEY_ItemFolderPathDisplay,  IDC_LOCATION_LABEL,        L"",                L":" },
      { PKEY_DateModified,           IDC_MODIFIED_LABEL,        L"",                L":" },
      { PKEY_FileAttributes,         IDC_ATTRIBUTES_LABEL,      L"",                L":" },
   };
   for( int i = 0; i < lengthof(aLabels); i++ ) {
      _SetLabelTextFromPKey(aLabels[i].uRes, aLabels[i].pkey, aLabels[i].pszPrefix, aLabels[i].pszPostfix);
   }
   // Other texts...
   _SetLabelTextFromRes(IDC_READONLY, IDS_READONLY);
   _SetLabelTextFromRes(IDC_HIDDEN, IDS_HIDDEN);
   // Gather information...
   CComPropVariant vFileName, vFileType, vComment, vFileSize, vTotalCount, vLocation, vModifiedDate, vFileAttribs;
   struct {
      UINT uRes;
      REFPROPERTYKEY pkeyFormat;
      CComPropVariant& vValue;
   } aValues[] = {
      { IDC_FILENAME,          PKEY_FileName,                vFileName },
      { IDC_COMMENT,           PKEY_Comment,                 vComment },
      { IDC_TYPE,              PKEY_ItemTypeText,            vFileType },
      { IDC_LOCATION,          PKEY_ItemFolderPathDisplay,   vLocation },
      { IDC_FILESIZE,          PKEY_Size,                    vFileSize },
      { IDC_FILESIZE,          PKEY_TotalFileSize,           vFileSize },
      { IDC_FILESIZEONDISK,    PKEY_Size,                    vTotalCount },
      { IDC_FILESIZEONDISK,    PKEY_TotalFileSize,           vTotalCount },
      { IDC_FILESIZEONDISK,    PKEY_FileCount,               vTotalCount },
      { IDC_MODIFIED,          PKEY_DateCreated,             vModifiedDate },
      { IDC_MODIFIED,          PKEY_DateModified,            vModifiedDate },
      { IDC_FLAGS,             PKEY_FileAttributes,          vFileAttribs },
   };
   for( int i = 0; spStore != NULL && i < lengthof(aValues); i++ ) {
      CComPropVariant v;
      pNseItem->GetProperty(aValues[i].pkeyFormat, v);
      if( v.vt != VT_EMPTY ) aValues[i].vValue = v;
   }
   for( int i = 0; spStore != NULL && i < lengthof(aValues); i++ ) {
      if( i > 0 && aValues[i].uRes == aValues[i - 1].uRes ) continue;
      _SetValueTextFromPKey(aValues[i].uRes, aValues[i].pkeyFormat, aValues[i].vValue);
   }
   // Enable edit controls when properties are writable
   CComQIPtr<IPropertyStoreCapabilities> spStoreCap = spStore;
   for( int i = 0; spStoreCap != NULL && i < lengthof(aValues); i++ ) {
      if( spStoreCap->IsPropertyWritable(aValues[i].pkeyFormat) == S_OK ) {
         SendDlgItemMessage(aValues[i].uRes, EM_SETREADONLY, FALSE, 0);
         SendDlgItemMessage(aValues[i].uRes, EM_SETMODIFY, FALSE, 0);
      }
   }
   // Multiple items selected?
   if( dwItemCount > 1 ) {
      _SetLabelTextFromRes(IDC_FILENAME, IDS_MULTIPLEVALUES);
      ::EnableWindow(GetDlgItem(IDC_FILENAME), FALSE);
   }
   // Is it a folder or a multi-select?
   // Display "Total file size" instead and count files in subfolders.
   if( pNseItem->IsFolder() || dwItemCount > 1 ) {
      _SetLabelTextFromPKey(IDC_FILESIZE_LABEL, PKEY_TotalFileSize, L"", L":");
      _SetLabelTextFromPKey(IDC_FILESIZEONDISK_LABEL, PKEY_FileCount, L"", L":");
      // Create thread that lookup the total file count
      COUNTERDATA* pData = new COUNTERDATA;
      pData->hWnd = m_hWnd;
      for( DWORD i = 0; i < dwItemCount && i < lengthof(pData->pidls); i++ ) {
         CComPtr<IShellItem> spShellItem;
         m_pShellItems->GetItemAt(i, &spShellItem);
         pData->pidls[i].CreateFromObject(spShellItem);
      }
      ::SHCreateThread(CountingThread, pData, CTF_COINIT | CTF_PROCESS_REF, NULL);
   }
   // Get the file-icon...
   if( spExtractIcon != NULL ) {
      UINT uFlags = 0;
      int iIconIndex = -1;
      SHFILEINFO sfi = { 0 };
      WCHAR wszIconFile[MAX_PATH] = { 0 };
      HICON hIconLarge = NULL, hIconSmall = NULL;
      // Get values for a possible SHGetFileInfo() call
      DWORD dwFileAttribs = 0;
      WCHAR wszFileName[MAX_PATH] = { 0 };
      ::PropVariantToUInt32(vFileAttribs, &dwFileAttribs);
      ::PropVariantToString(vFileName, wszFileName, lengthof(wszFileName));
      // Extract and set icon...
      HRESULT Hr = spExtractIcon->GetIconLocation(0, wszIconFile, lengthof(wszIconFile), &iIconIndex, &uFlags);
      if( Hr == S_OK ) Hr = spExtractIcon->Extract(wszIconFile, iIconIndex, &hIconLarge, &hIconSmall, MAKELPARAM(32, 32));
      if( Hr == S_FALSE ) ::SHGetFileInfo(wszFileName, dwFileAttribs, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_LARGEICON), hIconLarge = sfi.hIcon;
      if( hIconSmall != NULL ) ::DestroyIcon(hIconSmall);
      if( hIconLarge != NULL ) SendDlgItemMessage(IDC_APPICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIconLarge);
   }
   // File attributes...
   if( vFileAttribs.vt == VT_UI4 && dwItemCount == 1 ) {
      if( IsBitSet(vFileAttribs.uiVal, FILE_ATTRIBUTE_READONLY) ) CheckDlgButton(IDC_READONLY, BST_CHECKED);
      if( IsBitSet(vFileAttribs.uiVal, FILE_ATTRIBUTE_HIDDEN) ) CheckDlgButton(IDC_HIDDEN, BST_CHECKED);
      ::EnableWindow(GetDlgItem(IDC_READONLY), TRUE);
      ::EnableWindow(GetDlgItem(IDC_HIDDEN), TRUE);
   }
   // Bump style of ReadOnly & Hidden controls when it's a Folder or multi-select
   if( pNseItem->IsFolder() || dwItemCount > 1 ) {
      SendDlgItemMessage(IDC_READONLY, BM_SETSTYLE, BS_AUTO3STATE, 1L);
      SendDlgItemMessage(IDC_HIDDEN, BM_SETSTYLE, BS_AUTO3STATE, 1L);
      CheckDlgButton(IDC_READONLY, BST_INDETERMINATE);
      CheckDlgButton(IDC_HIDDEN, BST_INDETERMINATE);
      ::EnableWindow(GetDlgItem(IDC_READONLY), TRUE);
      ::EnableWindow(GetDlgItem(IDC_HIDDEN), TRUE);
   }
   m_bAttribsClicked = false;
   bHandled = FALSE;
   return 0;
}

LRESULT CNseFileItemPropertyPage::OnButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   m_bAttribsClicked = true;
   return 0;
}

LRESULT CNseFileItemPropertyPage::OnFileCountUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   // This message is sent by the CountingThread; it updates the UI values of the
   // "Total File Size" and "File Count" controls.
#pragma warning( disable : 4239 4244 )
   _SetValueTextFromPKey(IDC_FILESIZE, PKEY_TotalFileSize, CComPropVariant(lParam));
   _SetValueTextFromPKey(IDC_FILESIZEONDISK, PKEY_FileCount, CComPropVariant(wParam));
#pragma warning( default : 4239 4244 )
   return 0;
}

LRESULT CNseFileItemPropertyPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
   // Apply changes made by user.
   // NOTE: In this version we'll ignore all errors!
   _ApplyProperties();
   return PSNRET_NOERROR;
}

// Implementation

HRESULT CNseFileItemPropertyPage::_ApplyProperties()
{
   // Use Windows FileOperation component to apply changes...
   CComPtr<IFileOperation> spFO;
   HR( ::SHCreateFileOperation(m_hWnd, 0, &spFO) );
   if( ::IsWindowEnabled(GetDlgItem(IDC_FILENAME)) && SendDlgItemMessage(IDC_FILENAME, EM_GETMODIFY) != 0 ) 
   {
      CComBSTR bstrText;
      GetDlgItemText(IDC_FILENAME, bstrText.m_str);
      CComPtr<IShellItem> spShellItem;
      HR( m_pShellItems->GetItemAt(0, &spShellItem) );
      HR( spFO->RenameItem(spShellItem, bstrText, NULL) );
   }
   if( ::IsWindowEnabled(GetDlgItem(IDC_READONLY)) && m_bAttribsClicked ) 
   {
      CComPtr<IPropertyChangeArray> spPCA;
      HR( ::PSCreatePropertyChangeArray(NULL, NULL, NULL, 0, IID_PPV_ARGS(&spPCA)) );
      if( IsDlgButtonChecked(IDC_READONLY) != BST_INDETERMINATE ) {
         CComObject<CPropertyChangeUInt32>* pChange = NULL;
         HR( CComObject<CPropertyChangeUInt32>::CreateInstance(&pChange) );
         CComQIPtr<IPropertyChange> spChange = pChange->GetUnknown();
         HR( pChange->Init(PKEY_FileAttributes, IsDlgButtonChecked(IDC_READONLY) ? CPropertyChangeUInt32::PKA_BOR : CPropertyChangeUInt32::PKA_BNOTAND, FILE_ATTRIBUTE_READONLY) );
         HR( spPCA->Append(spChange) );
      }
      if( IsDlgButtonChecked(IDC_HIDDEN) != BST_INDETERMINATE ) {
         CComObject<CPropertyChangeUInt32>* pChange = NULL;
         HR( CComObject<CPropertyChangeUInt32>::CreateInstance(&pChange) );
         CComQIPtr<IPropertyChange> spChange = pChange->GetUnknown();
         HR( pChange->Init(PKEY_FileAttributes, IsDlgButtonChecked(IDC_HIDDEN) ? CPropertyChangeUInt32::PKA_BOR : CPropertyChangeUInt32::PKA_BNOTAND, FILE_ATTRIBUTE_HIDDEN) );
         HR( spPCA->Append(spChange) );
      }
      HR( spFO->SetProperties(spPCA) );
      HR( spFO->ApplyPropertiesToItems(m_pSheet->m_spDataObject) );
   }
   HR( spFO->PerformOperations() );
   return S_OK;
}

// Static members

DWORD WINAPI CNseFileItemPropertyPage::CountingThread(LPVOID pData)
{
   const COUNTERDATA* pInfo = reinterpret_cast<COUNTERDATA*>(pData);
   COUNTERDATA Info = *pInfo;
   delete pInfo;

   // Get all the Shell Items from the selection
   CInterfaceArray<IShellItem2> aQueue;
   for( int i = 0; i < lengthof(Info.pidls); i++ ) {
      const CPidl& pidl = Info.pidls[i];
      if( pidl.IsNull() ) break;
      CComPtr<IShellItem2> spShellItem;
      HR( ::SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&spShellItem)) );
      aQueue.Add(spShellItem);
   }

   // Collect files and folders.
   // TODO: Better memory management, please.
   DWORD dwLastTick = 0;
   DWORD dwTotalCount = 0;
   ULONGLONG ullTotalSize = 0;
   while( aQueue.GetCount() > 0 && ::IsWindow(Info.hWnd) ) {
      CComPtr<IShellItem2> spShellItem = aQueue.GetAt(0);
      aQueue.RemoveAt(0);
      // Count files and keep track of total filesize
      ULONGLONG ullSize = 0;
      ULONG dwAttribs = FILE_ATTRIBUTE_DIRECTORY;
      spShellItem->GetUInt64(PKEY_Size, &ullSize);
      spShellItem->GetUInt32(PKEY_FileAttributes, &dwAttribs);
      if( ullSize > 0 || !IsBitSet(dwAttribs, FILE_ATTRIBUTE_DIRECTORY) ) {
         ++dwTotalCount;
         ullTotalSize += ullSize;
      }
      // Enum children...
      CComPtr<IEnumShellItems> spEnumItems;
      spShellItem->BindToHandler(NULL, BHID_EnumItems, IID_PPV_ARGS(&spEnumItems));
      if( spEnumItems != NULL ) {
         ULONG uFetched = 0;
         CComPtr<IShellItem> spNextItem;
         while( spEnumItems->Next(1, &spNextItem, &uFetched), uFetched > 0 ) {
            CComQIPtr<IShellItem2> spShellItem2 = spNextItem;
            if( spShellItem2 != NULL ) aQueue.Add(spShellItem2);               
            spNextItem.Release();
            uFetched = 0;
         }
      }      
      // Post partial result?
      const DWORD PARTIAL_TIMEOUT_MS = 300;
      if( ::GetTickCount() - dwLastTick > PARTIAL_TIMEOUT_MS ) {
         dwLastTick = ::GetTickCount();
         if( ::IsWindow(Info.hWnd) ) ::PostMessage(Info.hWnd, WM_FILECOUNT_UPDATE, dwTotalCount, (LPARAM) ullTotalSize);
      }
   }

   // Post final result
   if( ::IsWindow(Info.hWnd) ) ::PostMessage(Info.hWnd, WM_FILECOUNT_UPDATE, dwTotalCount, (LPARAM) ullTotalSize);

   return 0;
}

