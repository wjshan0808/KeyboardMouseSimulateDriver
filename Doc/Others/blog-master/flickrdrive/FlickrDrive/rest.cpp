
#include "StdAfx.h"

#include "rest.h"

#include "AddUserDlg.h"


///////////////////////////////////////////////////////////////////////////////
// Flickr REST API implementation

CFlickrAPI::CFlickrAPI() : m_pGuestAccount(NULL)
{
}

CFlickrAPI::~CFlickrAPI()
{
   if( m_pGuestAccount != NULL ) delete m_pGuestAccount;
   for( int i = 0; i < m_aPhotos.GetSize(); i++ ) delete m_aPhotos[i];
   for( int i = 0; i < m_aAccounts.GetSize(); i++ ) delete m_aAccounts[i];
   for( int i = 0; i < m_aTempPhotos.GetSize(); i++ ) delete m_aTempPhotos[i];
   for( int i = 0; i < m_aDeletedPhotos.GetSize(); i++ ) delete m_aDeletedPhotos[i];
}

HRESULT CFlickrAPI::Init()
{
   // Poor-mans semaphore
   static volatile bool s_bInited = false;
   if( s_bInited ) return S_OK;
   s_bInited = true;

   m_lock.Init();               // Create thread lock
   HR( _InitAPI() );            // Clear stuff
   HR( _InitCrypto() );         // Get the Windows Crypto access
   HR( _ReadAccountList() );    // Read account cache if its available already
   HR( _InitDummyAccount() );   // Create the dummy user we use when no owner-account was found

   return S_OK;
}

HRESULT CFlickrAPI::InitHttpService()
{
   if( m_hInternet != NULL ) return S_OK;
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( m_hInternet != NULL ) return S_OK;
   ATLTRACE(L"CFlickrAPI::InitHttpService\n");
   HRESULT Hr = S_OK;
   WINHTTP_CURRENT_USER_IE_PROXY_CONFIG Proxy = { 0 };
   BOOL bRes = ::WinHttpGetIEProxyConfigForCurrentUser(&Proxy);
   if( !bRes || Proxy.fAutoDetect || Proxy.lpszAutoConfigUrl != NULL ) {
      HINTERNET hProxy = ::WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
      if( hProxy != NULL ) {
         WINHTTP_AUTOPROXY_OPTIONS AutoProxy = { 0 };
         if( Proxy.fAutoDetect ) AutoProxy.dwFlags |= WINHTTP_AUTOPROXY_AUTO_DETECT, AutoProxy.dwAutoDetectFlags |= WINHTTP_AUTO_DETECT_TYPE_DHCP;
         if( Proxy.lpszAutoConfigUrl != NULL ) AutoProxy.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL, AutoProxy.lpszAutoConfigUrl = Proxy.lpszAutoConfigUrl;
         if( !bRes ) AutoProxy.dwFlags |= WINHTTP_AUTOPROXY_AUTO_DETECT, AutoProxy.dwAutoDetectFlags |= WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
         AutoProxy.fAutoLogonIfChallenged = TRUE;
         WINHTTP_PROXY_INFO ProxyInfo = { 0 };
         if( ::WinHttpGetProxyForUrl(hProxy, L"http://www.flickr.com", &AutoProxy, &ProxyInfo) ) {
            if( ProxyInfo.lpszProxy != NULL ) Proxy.lpszProxy = ProxyInfo.lpszProxy;
            if( ProxyInfo.lpszProxyBypass != NULL ) ::GlobalFree(ProxyInfo.lpszProxyBypass);
         }
         ::WinHttpCloseHandle(hProxy);
      }
   }
   m_hInternet.m_h = ::WinHttpOpen(BROWSER_USERAGENT, Proxy.lpszProxy == NULL ? WINHTTP_ACCESS_TYPE_DEFAULT_PROXY : WINHTTP_ACCESS_TYPE_NAMED_PROXY, Proxy.lpszProxy, Proxy.lpszProxyBypass, 0);
   if( m_hInternet == NULL ) Hr = AtlHresultFromLastError();
   if( Proxy.lpszProxy != NULL ) ::GlobalFree(Proxy.lpszProxy);
   if( Proxy.lpszProxyBypass != NULL ) ::GlobalFree(Proxy.lpszProxyBypass);
   if( Proxy.lpszAutoConfigUrl != NULL ) ::GlobalFree(Proxy.lpszAutoConfigUrl);
   return Hr;
}

HRESULT CFlickrAPI::GetAccountList(CFlickrAccountList& aList)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   HR( _ReadAccountList() );
   for( int i = 0; i < m_aAccounts.GetSize(); i++ ) {
      aList.Add(m_aAccounts[i]);
   }
   return S_OK;
}

HRESULT CFlickrAPI::GetAccountFromUsername(LPCWSTR pstrName, TFlickrAccount** ppAccount)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pstrName == NULL || pstrName[0] == '\0' ) return E_FAIL;
   HR( _ReadAccountList() );
   for( int i = 0; i < m_aAccounts.GetSize(); i++ ) {
      if( m_aAccounts[i]->sUserName.CompareNoCase(pstrName) == 0 ) return *ppAccount = m_aAccounts[i], S_OK;
   }
   return E_FAIL;
}

HRESULT CFlickrAPI::GetAccountFromNSID(LPCWSTR pstrNSID, TFlickrAccount** ppAccount)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pstrNSID == NULL || pstrNSID[0] == '\0' ) return E_FAIL;
   HR( _ReadAccountList() );
   for( int i = 0; i < m_aAccounts.GetSize(); i++ ) {
      if( m_aAccounts[i]->sNSID.CompareNoCase(pstrNSID) == 0 ) return *ppAccount = m_aAccounts[i], S_OK;
   }
   return E_FAIL;
}

HRESULT CFlickrAPI::GetAccountFromPrimary(TFlickrAccount** ppAccount)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   HR( _ReadAccountList() );
   for( int i = 0; i < m_aAccounts.GetSize(); i++ ) {
      if( m_aAccounts[i]->bIsPrimary ) return *ppAccount = m_aAccounts[i], S_OK;
   }
   for( int i = 0; i < m_aAccounts.GetSize(); i++ ) {
      if( m_aAccounts[i]->Access == FLICKRFS_ACCOUNT_OWNED ) return *ppAccount = m_aAccounts[i], S_OK;
   }
   return E_FAIL;
}

HRESULT CFlickrAPI::WelcomeAccount(HWND hWnd)
{
   ATLASSERT(::IsWindow(hWnd));
   // Call this method only when we haven't set up an account yet
   HR( _ReadAccountList() );
   ATLASSERT(m_aAccounts.GetSize()==0);
   // Let user create the first account...
   CWelcomeUserDlg dlg;
   if( dlg.DoModal(hWnd) != IDOK ) return E_ABORT;
   CString sAuthToken;
   HRESULT Hr = S_OK;
   if( SUCCEEDED(Hr) ) Hr = _DoAccountWebAuthentication(hWnd, dlg.m_sUsername, FLICKRFS_ACCOUNT_OWNED, sAuthToken);
   if( SUCCEEDED(Hr) ) Hr = _DoDefaultAccountSetup(dlg.m_sUsername, FLICKRFS_ACCOUNT_OWNED, sAuthToken, TRUE, dlg.m_bAddFriends, dlg.m_bAddFamily);
   if( FAILED(Hr) ) DoDisplayErrorMessage(hWnd, IDS_ERR_ACCOUNT, Hr);
   // Refresh account list in any case; might have partial success
   HR( _RefreshAccountCache() );
   return Hr;
}

HRESULT CFlickrAPI::AddAccount(HWND hWnd)
{
   // Make sure we have read the accounts so we can check for duplicates
   HR( _ReadAccountList() );
   // Let user attach a new account...
   CAddUserDlg dlg;
   if( hWnd == NULL ) hWnd = ::GetActiveWindow();
   if( dlg.DoModal(hWnd) != IDOK ) return E_ABORT;
   CString sAuthToken;
   HRESULT Hr = S_OK;
   if( SUCCEEDED(Hr) ) Hr = _DoAccountWebAuthentication(hWnd, dlg.m_sUsername, dlg.m_AccountType, sAuthToken);
   if( SUCCEEDED(Hr) ) Hr = _DoDefaultAccountSetup(dlg.m_sUsername, dlg.m_AccountType, sAuthToken, FALSE, FALSE, FALSE);
   if( FAILED(Hr) ) DoDisplayErrorMessage(hWnd, IDS_ERR_ACCOUNT, Hr);
   // Refresh account list in any case; might have partial success
   HR( _RefreshAccountCache() );
   return Hr;
}

HRESULT CFlickrAPI::AddAccount(LPCTSTR pstrUsername)
{
   HR( CheckAccount(pstrUsername) );
   HR( _DoDefaultAccountSetup(pstrUsername, FLICKRFS_ACCOUNT_GUEST, _T(""), FALSE, FALSE, FALSE) );
   HR( _RefreshAccountCache() );
   return S_OK;
}

HRESULT CFlickrAPI::CheckAccount(LPCTSTR pstrUsername)
{
   HR( _ReadAccountList() );

   // Does the Public Folder exists? Otherwise we should to create it...
   CRegKey regAccounts;
   if( regAccounts.Create(HKEY_CURRENT_USER, REGPATH_FLICKRFS REGPATH_FLICKRFS_ACCOUNTS) != ERROR_SUCCESS ) return E_ACCESSDENIED;
   CRegKey regPublicFolder;
   if( regPublicFolder.Open(regAccounts, FLICKRFS_ACCOUNTSTR_PUBLIC) != ERROR_SUCCESS )
   {
      CComBSTR bstrTitle;
      bstrTitle.LoadString(IDS_FOLDER_PUBLIC);
      if( regPublicFolder.Create(regAccounts, FLICKRFS_ACCOUNTSTR_PUBLIC) != ERROR_SUCCESS ) return E_ACCESSDENIED;
      regPublicFolder.SetStringValue(_T("User Name"), bstrTitle);
      regPublicFolder.SetStringValue(_T("Full Name"), bstrTitle);
      regPublicFolder.SetDWORDValue(_T("Type"), FLICKRFS_ACCOUNT_PUBLIC);

      CRegKey keyCollections;
      keyCollections.Create(regPublicFolder, REGPATH_FLICKRFS_COLLECTIONS);
      _CreateAccountFolder(keyCollections, FLICKRFS_FOLDERSTR_INTERESTING, IDS_FOLDER_INTERESTINGNESS, FLICKRFS_IMAGESET_INTERESTING);
      _CreateAccountFolder(keyCollections, FLICKRFS_FOLDERSTR_HOTTAGS, IDS_FOLDER_HOTTAGS, FLICKRFS_IMAGESET_HOTTAGS);
   }

   // Username is actually a valid (browsable) Flickr user?
   FLICKR_APIARG ApiPeopleFindByUsername[] =
   {
      { "method",       _T("flickr.people.findByUsername") },
      { "api_key",      FLICKR_API_KEY },
      { "username",     pstrUsername },
   };
   if( FAILED( _SubmitFlickrRestAction(ApiPeopleFindByUsername, lengthof(ApiPeopleFindByUsername)) ) ) {
      return AtlHresultFromWin32(ERROR_BAD_USER_PROFILE);
   }

   // User was already added?
   TFlickrAccount* pAccount = NULL;
   GetAccountFromUsername(pstrUsername, &pAccount);
   if( pAccount != NULL ) return AtlHresultFromWin32(ERROR_ALREADY_REGISTERED);

   return S_OK;
}

HRESULT CFlickrAPI::RemoveAccount(TFlickrAccount* pAccount)
{
   ATLASSERT(pAccount);
   ATLASSERT(pAccount->sNSID.GetLength()>0);
   if( pAccount->sNSID.IsEmpty() ) return E_ACCESSDENIED;
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   TCHAR szRegPath[250] = { 0 };
   ::wnsprintf(szRegPath, lengthof(szRegPath) - 1, _T("%s%s\\%s"), REGPATH_FLICKRFS, REGPATH_FLICKRFS_ACCOUNTS, pAccount->sNSID);
   ::RegDeleteTree(HKEY_CURRENT_USER, szRegPath);
   // Update cache
   m_aAccounts.Remove(pAccount);
   return S_OK;
}

HRESULT CFlickrAPI::ReadUploadStatus()
{
   // Already populated?
   if( m_VolumeInfo.ullUploadMaxBytes > 0 ) return S_OK;
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( m_VolumeInfo.ullUploadMaxBytes > 0 ) return S_OK;
   // Query login account bandwidth info...
   TFlickrAccount* pAccount = NULL;
   HR( GetAccountFromPrimary(&pAccount) );
   CComPtr<IXMLDOMDocument> spDoc;
   FLICKR_APIARG ApiPeopleGetUploadStatus[] =
   {
      { "method",       _T("flickr.people.getUploadStatus") },
      { "api_key",      FLICKR_API_KEY },
      { "auth_token",   pAccount->sAuthToken }
   };
   HR( _SubmitFlickrRestAction(ApiPeopleGetUploadStatus, lengthof(ApiPeopleGetUploadStatus), &spDoc) );
   CString sMaxBytes, sUsedBytes, sRemainingBytes;
   _XmlGetResultNode(spDoc, L"rsp/user/bandwidth/@maxbytes", sMaxBytes);
   _XmlGetResultNode(spDoc, L"rsp/user/bandwidth/@usedbytes", sUsedBytes);
   _XmlGetResultNode(spDoc, L"rsp/user/bandwidth/@remainingbytes", sRemainingBytes);
   m_VolumeInfo.ullUploadMaxBytes = _ttoi64(sMaxBytes);
   m_VolumeInfo.ullUploadUsedBytes = _ttoi64(sUsedBytes);
   m_VolumeInfo.ullUploadRemainingBytes = _ttoi64(sRemainingBytes);
   return S_OK;
}

HRESULT CFlickrAPI::RefreshAccountList(TFlickrAccount* pAccount)
{
   if( pAccount == NULL ) return E_INVALIDARG;
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   pAccount->aImagesets.RemoveAll();
   return S_OK;
}

HRESULT CFlickrAPI::AddImageset(TFlickrAccount* pAccount, LPCWSTR pstrName, TFlickrImagesetType Type)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   HR( _ReadAccountList() );
   // Already exists?
   TFlickrImageset* pImageset = NULL;
   FindImageset(pAccount, pstrName, &pImageset);
   if( pImageset != NULL ) return AtlHresultFromWin32(ERROR_ALREADY_EXISTS);
   // Create new imageset based on type requested...
   if( Type == FLICKRFS_IMAGESET_PHOTOSET )
   {
      if( pAccount->Access != FLICKRFS_ACCOUNT_OWNED ) return E_ACCESSDENIED;
      // Find the Primary Photo ID of the Photoset. We just pick the first
      // photo currently not in a photoset.
      FindImageset(pAccount, FLICKRFS_FOLDERSTR_NOTINSET, &pImageset);
      if( pImageset == NULL ) return E_UNEXPECTED;
      _ReadImagesetPhotoList(pImageset);
      if( pImageset->aPhotoIds.GetSize() == 0 ) return E_UNEXPECTED;
      // Create photoset...
      FLICKR_APIARG ApiPhotosetsCreate[] =
      {
         { "method",           _T("flickr.photosets.create") },
         { "api_key",          FLICKR_API_KEY },
         { "title",            pstrName },
         { "primary_photo_id", pImageset->aPhotoIds[0] },
         { "auth_token",       pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosetsCreate, lengthof(ApiPhotosetsCreate)) );
      // Refresh cache
      pAccount->aImagesets.RemoveAll();
      return S_OK;
   }
   if( Type == FLICKRFS_IMAGESET_TAGSEARCH )
   {
      TCHAR szRegPath[250] = { 0 };
      ::wnsprintf(szRegPath, lengthof(szRegPath) - 1, _T("%s%s\\%s\\%s\\%08X"), REGPATH_FLICKRFS, REGPATH_FLICKRFS_ACCOUNTS, pAccount->sNSID, REGPATH_FLICKRFS_COLLECTIONS, ::GetTickCount());
      CRegKey regImageset;
      if( regImageset.Create(HKEY_CURRENT_USER, szRegPath) != ERROR_SUCCESS ) return E_ACCESSDENIED;
      regImageset.SetStringValue(_T("Name"), pstrName);
      regImageset.SetDWORDValue(_T("Type"), FLICKRFS_IMAGESET_TAGSEARCH);
      // Refresh cache
      pAccount->aImagesets.RemoveAll();
      return S_OK;
   }
   return E_INVALIDARG;
}

HRESULT CFlickrAPI::FindImageset(TFlickrAccount* pAccount, LPCWSTR pstrName, TFlickrImageset** ppImageset)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pAccount == NULL ) return E_INVALIDARG;
   HR( _ReadImagesetList(pAccount) );
   for( int i = 0; i < pAccount->aImagesets.GetSize(); i++ ) {
      TFlickrImageset* pImageset = pAccount->aImagesets[i];
      if( pImageset->sImagesetID == pstrName ) return *ppImageset = pImageset, S_OK;
   }
   for( int i = 0; i < pAccount->aImagesets.GetSize(); i++ ) {
      TFlickrImageset* pImageset = pAccount->aImagesets[i];
      if( pImageset->sTitle.CompareNoCase(pstrName) == 0 ) return *ppImageset = pImageset, S_OK;
      if( pImageset->sFilename.CompareNoCase(pstrName) == 0 ) return *ppImageset = pImageset, S_OK;
   }
   return E_FAIL;
}

HRESULT CFlickrAPI::GetImagesetList(TFlickrAccount* pAccount, CFlickrImagesetList& aList)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   HR( _ReadImagesetList(pAccount) );
   for( int i = 0; i < pAccount->aImagesets.GetSize(); i++ ) {
      aList.Add(pAccount->aImagesets[i]);
   }
   return S_OK;
}

HRESULT CFlickrAPI::DeleteImageset(TFlickrImageset* pImageset)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET )
   {
      FLICKR_APIARG ApiPhotosetsDelete[] =
      {
         { "method",       _T("flickr.photosets.delete") },
         { "api_key",      FLICKR_API_KEY },
         { "photoset_id",  pImageset->sImagesetID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosetsDelete, lengthof(ApiPhotosetsDelete)) );
   }
   else
   {
      TCHAR szRegPath[250] = { 0 };
      ::wnsprintf(szRegPath, lengthof(szRegPath) - 1, _T("%s%s\\%s\\%s\\%s"), REGPATH_FLICKRFS, REGPATH_FLICKRFS_ACCOUNTS, pImageset->pAccount->sNSID, REGPATH_FLICKRFS_COLLECTIONS, pImageset->sImagesetID);
      DWORD dwErr = ::RegDeleteTree(HKEY_CURRENT_USER, szRegPath);
      if( dwErr != ERROR_SUCCESS ) return AtlHresultFromWin32(dwErr);
   }
   // Update cache
   pImageset->pAccount->aImagesets.Remove(pImageset);
   m_aTempPhotos.RemoveAll();
   return S_OK;
}

HRESULT CFlickrAPI::DeletePhoto(TFlickrPhoto* pPhoto)
{
   ATLASSERT(pPhoto);
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pPhoto == NULL ) return E_INVALIDARG;
   if( pPhoto->pAccount == NULL ) return E_ACCESSDENIED;
   FLICKR_APIARG ApiPhotosDelete[] =
   {
      { "method",       _T("flickr.photos.delete") },
      { "api_key",      FLICKR_API_KEY },
      { "photo_id",     pPhoto->sPhotoID },
      { "auth_token",   pPhoto->pAccount->sAuthToken }
   };
   HR( _SubmitFlickrRestAction(ApiPhotosDelete, lengthof(ApiPhotosDelete)) );
   // Update cache
   HR( _RefreshAccountPhotosCache(pPhoto->pAccount) );
   m_aDeletedPhotos.Add(pPhoto);
   m_aTempPhotos.Remove(pPhoto);
   m_aPhotos.Remove(pPhoto);
   return S_OK;
}

HRESULT CFlickrAPI::SpawnTempPhoto(const WIN32_FIND_DATA wfd)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   for( int i = 0; i < m_aTempPhotos.GetSize(); i++ ) {
      if( m_aTempPhotos[i]->sPhotoID == wfd.cFileName ) return S_OK;
   }
   TFlickrPhoto* pPhoto = new TFlickrPhoto;
   pPhoto->sPhotoID = wfd.cFileName;
   pPhoto->bIsFamily = FALSE;
   pPhoto->bIsFriend = FALSE;
   pPhoto->bIsPublic = FALSE;
   pPhoto->bIsOwned = TRUE;
   pPhoto->bIsSpawned = (wfd.cAlternateFileName[1] == VFS_HACK_SAVEAS_JUNCTION);
   pPhoto->ftDateTaken = wfd.ftCreationTime;
   pPhoto->ftDatePosted = wfd.ftLastWriteTime;
   pPhoto->sExtension = ::PathFindExtension(wfd.cFileName);
   pPhoto->sFilename = CString(wfd.cFileName, (int) _tcslen(wfd.cFileName) - pPhoto->sExtension.GetLength());
   pPhoto->sTitle = pPhoto->sFilename;
   pPhoto->pAccount = m_pGuestAccount;
   GetAccountFromPrimary(&pPhoto->pAccount);
   return m_aTempPhotos.Add(pPhoto) ? S_OK : E_OUTOFMEMORY;
}

HRESULT CFlickrAPI::PostProcessPhotoUpload(LPCTSTR pstrFilename, TFlickrImageset* pImageset, LPSTR pstrXmlResponse)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   HR( _ReadAccountList() );
   // Extract the PhotoID from the HTTP upload response...
   CComPtr<IXMLDOMDocument> spDoc;
   HR( _RestExtractResult(pstrXmlResponse, &spDoc) );
   CString sPhotoID;
   HR( _XmlGetResultNode(spDoc, L"rsp/photoid", sPhotoID) );
   // Wrap the new photo (possibly construct a temporary if Flickr didn't have time to sync)
   TFlickrPhoto* pPhoto = NULL;
   if( FAILED( GetPhoto(sPhotoID, &pPhoto) ) ) {
      pPhoto = new TFlickrPhoto();
      pPhoto->sPhotoID = sPhotoID;
      m_aTempPhotos.Add(pPhoto);
   }
   return PostProcessPhotoUpload(pPhoto, pImageset);
}

HRESULT CFlickrAPI::PostProcessPhotoUpload(TFlickrPhoto *pPhoto, TFlickrImageset* pImageset)
{
   ATLASSERT(pPhoto);
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   ATLTRACE(L"CFlickrAPI::PostProcessPhotoUpload  imageset='%s'\n", pImageset != NULL ? pImageset->sTitle : L"none");
   // Put new photo in requested imageset
   if( pImageset != NULL ) AddPhotoToImageset(pImageset, pPhoto);
   else _RefreshEntirePhotoListCache();
   // Clean up temporarily spawned photo instances
   for( int i = 0; i < m_aTempPhotos.GetSize(); i++ ) delete m_aTempPhotos[i];
   m_aTempPhotos.RemoveAll();
   return S_OK;
}

HRESULT CFlickrAPI::AddPhotoToImageset(TFlickrImagesetType Type, TFlickrPhoto* pPhoto)
{
   ATLASSERT(pPhoto);
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pPhoto == NULL ) return E_INVALIDARG;
   if( Type == FLICKRFS_IMAGESET_FAVOURITES )
   {
      TFlickrAccount* pAccount = NULL;
      HR( GetAccountFromPrimary(&pAccount) );
      FLICKR_APIARG ApiFavoritesAdd[] =
      {
         { "method",       _T("flickr.favorites.add") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "auth_token",   pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiFavoritesAdd, lengthof(ApiFavoritesAdd)) );
      TFlickrImageset* pImageset = NULL;
      if( SUCCEEDED( FindImageset(pAccount, FLICKRFS_FOLDERSTR_FAVOURITES, &pImageset) ) ) {
         pImageset->aPhotoIds.RemoveAll();
      }
      return S_OK;
   }
   return E_INVALIDARG;
}

HRESULT CFlickrAPI::AddPhotoToImageset(TFlickrImageset* pImageset, TFlickrPhoto* pPhoto)
{
   ATLASSERT(pPhoto);
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pPhoto == NULL ) return E_INVALIDARG;
   if( pImageset == NULL ) return E_INVALIDARG;
   if( pImageset->Type == FLICKRFS_IMAGESET_FAVOURITES )
   {
      FLICKR_APIARG ApiFavoritesAdd[] =
      {
         { "method",       _T("flickr.favorites.add") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiFavoritesAdd, lengthof(ApiFavoritesAdd)) );
      pImageset->aPhotoIds.RemoveAll();
      return S_OK;
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET )
   {
      FLICKR_APIARG ApiPhotosetsAddPhoto[] =
      {
         { "method",       _T("flickr.photosets.addPhoto") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "photoset_id",  pImageset->sImagesetID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosetsAddPhoto, lengthof(ApiPhotosetsAddPhoto)) );
      pImageset->aPhotoIds.RemoveAll();
      return S_OK;
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_GROUP )
   {
      FLICKR_APIARG ApiGroupsPoolsAdd[] =
      {
         { "method",       _T("flickr.groups.pools.add") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "group_id",     pImageset->sImagesetID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiGroupsPoolsAdd, lengthof(ApiGroupsPoolsAdd)) );
      pImageset->aPhotoIds.RemoveAll();
      return S_OK;
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH ) 
   {
      HR( AddPhotoProperty(pPhoto, PKEY_Keywords, CComPropVariant(pImageset->sKeywords)) );
      pImageset->aPhotoIds.RemoveAll();
      return S_OK;
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_NOTINSET ) 
   {
      pImageset->aPhotoIds.RemoveAll();
      return S_OK;
   }
   return E_INVALIDARG;
}

HRESULT CFlickrAPI::RemovePhotoFromImageset(TFlickrImageset* pImageset, TFlickrPhoto* pPhoto)
{
   ATLASSERT(pPhoto);
   ATLASSERT(pImageset);
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pPhoto == NULL ) return E_INVALIDARG;
   if( pImageset == NULL ) return E_INVALIDARG;
   if( pImageset->Type == FLICKRFS_IMAGESET_FAVOURITES )
   {
      FLICKR_APIARG ApiFavoritesRemove[] =
      {
         { "method",       _T("flickr.favorites.remove") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiFavoritesRemove, lengthof(ApiFavoritesRemove)) );
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET )
   {
      FLICKR_APIARG ApiPhotosetsRemovePhoto[] =
      {
         { "method",       _T("flickr.photosets.removePhoto") },
         { "api_key",      FLICKR_API_KEY },
         { "photoset_id",  pImageset->sImagesetID },
         { "photo_id",     pPhoto->sPhotoID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosetsRemovePhoto, lengthof(ApiPhotosetsRemovePhoto)) );
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_GROUP )
   {
      FLICKR_APIARG ApiGroupsPoolsRemove[] =
      {
         { "method",       _T("flickr.groups.pools.remove") },
         { "api_key",      FLICKR_API_KEY },
         { "group_id",     pImageset->sImagesetID },
         { "photo_id",     pPhoto->sPhotoID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiGroupsPoolsRemove, lengthof(ApiGroupsPoolsRemove)) );
   }
   pImageset->aPhotoIds.Remove(pPhoto->sPhotoID);
   if( pImageset->aPhotoIds.GetSize() == 0 ) pImageset->pAccount->aImagesets.RemoveAll();
   return S_OK;
}

HRESULT CFlickrAPI::GetImagesetPhotoList(TFlickrImageset* pImageset, CFlickrPhotoList& aList)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   // Check that another day hasn't passed; we use the day-change to refresh
   // various folders, such as the "Todays hot tags"-folder.
   SYSTEMTIME stNow; ::GetLocalTime(&stNow);
   if( stNow.wDay != m_stThisDay.wDay ) {
      HR( _RefreshEntirePhotoListCache() );
      m_stThisDay = stNow;
      m_dwThisCount = 0;
   }
   // Read and populate the photolist
   HR( _ReadImagesetPhotoList(pImageset) );
   for( int i = 0; i < pImageset->aPhotoIds.GetSize(); i++ ) {
      const CString& sPhotoID = pImageset->aPhotoIds[i];
      for( int j = 0; j < m_aPhotos.GetSize(); j++ ) {
         if( sPhotoID == m_aPhotos[j]->sPhotoID ) aList.Add(m_aPhotos[j]);
      }
   }
   return S_OK;
}

HRESULT CFlickrAPI::UpdateImagesetPhotoList(TFlickrImageset* pImageset)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   HR( _ReadImagesetPhotoList(pImageset) );
   return S_OK;
}

HRESULT CFlickrAPI::RefreshImagesetPhotoList(TFlickrImageset* pImageset)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pImageset == NULL ) return E_INVALIDARG;
   pImageset->aPhotoIds.RemoveAll();
   return S_OK;
}

HRESULT CFlickrAPI::GetPhoto(LPCWSTR pstrPhotoID, TFlickrPhoto** ppPhoto)
{
   ATLASSERT(pstrPhotoID);
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   // Valid photo ID given?
   if( pstrPhotoID[0] == '\0' ) return E_FAIL;
   // Lookup in PhotoID hash-map...
   DWORD dwHashIdx = _UInt4Hash(pstrPhotoID) % lengthof(m_aPhotoHashmap);
   int iIndex = m_aPhotoHashmap[dwHashIdx];
   if( iIndex > 0 && iIndex < m_aPhotos.GetSize() && m_aPhotos[iIndex]->sPhotoID == pstrPhotoID ) {
      *ppPhoto = m_aPhotos[iIndex];
      return S_OK;
   }
   // Lookup in entire Photo cache...
   for( int i = 0; i < m_aPhotos.GetSize(); i++ ) {
      if( m_aPhotos[i]->sPhotoID == pstrPhotoID ) {
         m_aPhotoHashmap[dwHashIdx] = i;
         *ppPhoto = m_aPhotos[i];
         return S_OK;
      }
   }
   // Might be the display name (title.jpg)...
   WCHAR wszTitle[MAX_PATH] = { 0 };
   wcscpy_s(wszTitle, lengthof(wszTitle), pstrPhotoID);
   ::PathRemoveExtension(wszTitle);
   for( int i = 0; i < m_aPhotos.GetSize(); i++ ) {
      if( m_aPhotos[i]->sTitle.CompareNoCase(wszTitle) == 0 ) {
         *ppPhoto = m_aPhotos[i];
         return S_OK;
      }
      if( m_aPhotos[i]->sFilename.CompareNoCase(wszTitle) == 0 ) {
         *ppPhoto = m_aPhotos[i];
         return S_OK;
      }
   }
   // It could be a temp photo (temporarily spawned by the Shell)
   for( int i = 0; i < m_aTempPhotos.GetSize(); i++ ) {
      if( m_aTempPhotos[i]->sPhotoID == pstrPhotoID ) {
         *ppPhoto = m_aTempPhotos[i];
         return S_OK;
      }
   }
   // It could be a deleted photo?
   // Must avoid excessive REST lookups. Bail out early.
   for( int i = 0; i < m_aDeletedPhotos.GetSize(); i++ ) {
      if( m_aDeletedPhotos[i]->sPhotoID == pstrPhotoID ) {
         return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
      }
   }
   // It could be an unlisted photo...
   CComPtr<IXMLDOMDocument> spDoc;
   FLICKR_APIARG ApiPhotosGetInfo[] =
   {
      { "method",       _T("flickr.photos.getInfo") },
      { "api_key",      FLICKR_API_KEY },
      { "photo_id",     pstrPhotoID }
   };
   HRESULT Hr = _SubmitFlickrRestAction(ApiPhotosGetInfo, lengthof(ApiPhotosGetInfo), &spDoc);
   if( SUCCEEDED(Hr) ) Hr = _ReadImagesetPhotoListResult(NULL, spDoc, L"rsp/photo");
   if( SUCCEEDED(Hr) ) return *ppPhoto = m_aPhotos[ m_aPhotos.GetSize() - 1 ], S_OK;
   // No, we cannot find this photo...
   return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
}

HRESULT CFlickrAPI::ReadPhotoExtendedInfo(TFlickrPhoto* pPhoto)
{
   // Already populated?
   if( !pPhoto->sLicenseID.IsEmpty() ) return S_OK;
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( !pPhoto->sLicenseID.IsEmpty() ) return S_OK;
   pPhoto->sLicenseID = _T("0");
   // Query detailed photo info...
   CComPtr<IXMLDOMDocument> spDoc;
   FLICKR_APIARG ApiPhotosGetInfo[] =
   {
      { "method",       _T("flickr.photos.getInfo") },
      { "api_key",      FLICKR_API_KEY },
      { "photo_id",     pPhoto->sPhotoID },
      { "secret",       pPhoto->sSecret },
   };
   HR( _SubmitFlickrRestAction(ApiPhotosGetInfo, lengthof(ApiPhotosGetInfo), &spDoc) );
   _XmlGetResultNode(spDoc, L"rsp/photo/@license", pPhoto->sLicenseID);
   _XmlGetResultNode(spDoc, L"rsp/photo/@originalsecret", pPhoto->sOrigSecret);
   _XmlGetResultNode(spDoc, L"rsp/photo/@originalformat", pPhoto->sOrigFormat);
   _XmlGetResultNode(spDoc, L"rsp/photo/description", pPhoto->sDescription);
   _XmlGetResultNode(spDoc, L"rsp/photo/owner/@username", pPhoto->sOwnerUsername);
   // Collect tags of photo
   CSimpleArray<CString> aTags;
   HR( _XmlGetResultList(spDoc, L"rsp/photo/tags/tag", L"@raw", aTags) );
   for( int i = 0; i < aTags.GetSize(); i++ ) {
      if( !pPhoto->sKeywords.IsEmpty() ) pPhoto->sKeywords += L"; ";
      pPhoto->sKeywords += aTags[i];
   }
   return S_OK;
}

HRESULT CFlickrAPI::ReadPhotoExifInfo(TFlickrPhoto* pPhoto)
{
   // Already populated?
   if( !pPhoto->sExifCameraMake.IsEmpty() ) return S_OK;
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( !pPhoto->sExifCameraMake.IsEmpty() ) return S_OK;
   pPhoto->sExifCameraMake = _T(" ");
   // Has access to EXIF data?
   if( pPhoto->pAccount == NULL || pPhoto->pAccount->Access != FLICKRFS_ACCOUNT_OWNED ) return S_OK;
   // Query Exif photo info...
   CComPtr<IXMLDOMDocument> spDoc;
   FLICKR_APIARG ApiPhotosGetExif[] =
   {
      { "method",       _T("flickr.photos.getExif") },
      { "api_key",      FLICKR_API_KEY },
      { "photo_id",     pPhoto->sPhotoID },
      { "secret",       pPhoto->sSecret },
   };
   HR( _SubmitFlickrRestAction(ApiPhotosGetExif, lengthof(ApiPhotosGetExif), &spDoc) );
   _XmlGetResultNode(spDoc, L"rsp/photo/exif[@tag='ImageSize']/raw", pPhoto->sExifDimensions);
   _XmlGetResultNode(spDoc, L"rsp/photo/exif[@tag='ImageWidth']/raw", pPhoto->sExifImageWidth);
   _XmlGetResultNode(spDoc, L"rsp/photo/exif[@tag='ImageHeight']/raw", pPhoto->sExifImageHeight);
   _XmlGetResultNode(spDoc, L"rsp/photo/exif[@tag='BitsPerSample']/raw", pPhoto->sExifBitsPrSample);
   _XmlGetResultNode(spDoc, L"rsp/photo/exif[@tag='XResolution']/raw", pPhoto->sExifXResolution);
   _XmlGetResultNode(spDoc, L"rsp/photo/exif[@tag='YResolution']/raw", pPhoto->sExifYResolution);
   _XmlGetResultNode(spDoc, L"rsp/photo/exif[@tag='Make']/raw", pPhoto->sExifCameraMake);
   _XmlGetResultNode(spDoc, L"rsp/photo/exif[@tag='Model']/raw", pPhoto->sExifCameraModel);
   return S_OK;
}

HRESULT CFlickrAPI::GetPhotoUrls(TFlickrPhoto* pPhoto, CSimpleArray<CString>& aURLs)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   CComPtr<IXMLDOMDocument> spDoc;
   FLICKR_APIARG ApiPhotosGetSizes[] =
   {
      { "method",       _T("flickr.photos.getSizes") },
      { "api_key",      FLICKR_API_KEY },
      { "photo_id",     pPhoto->sPhotoID }
   };
   HR( _SubmitFlickrRestAction(ApiPhotosGetSizes, lengthof(ApiPhotosGetSizes), &spDoc) );
   HR( _XmlGetResultList(spDoc, L"rsp/sizes/size", L"@source", aURLs) );
   return S_OK;
}

HRESULT CFlickrAPI::GetPhotoUploadUrl(TFlickrAccount* pAccount, FLICKR_APIARG* pArgs, int nArgs, CString& sURI, CString& sSignature)
{
   // Check that we own the account, otherwise upload isn't possible
   if( pAccount == NULL ) return E_POINTER;
   if( pAccount->Access != FLICKRFS_ACCOUNT_OWNED ) return AtlHresultFromWin32(ERROR_WRITE_PROTECT);
   // Return the URI and MD5 signature
   sURI.Format(_T("%s%s"), YAHOO_FLICKR_URL_UPLOAD, YAHOO_FLICKR_UPLOAD_URLPATH);
   sSignature = _RestBuildActionUrl(FLICKR_APITYPE_UPLOAD, pArgs, nArgs);
   return S_OK;
}

HRESULT CFlickrAPI::GetPhotoThumbUrl(LPCWSTR pstrPhotoID, VFS_STREAMTYPE Type, int cxyThumb, CString& sURI)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   TFlickrPhoto* pPhoto = NULL;
   HR( GetPhoto(pstrPhotoID, &pPhoto) );
   // Getting the original photo out of Flickr is a little tricky. Not all photos are available in
   // original format. It may require your own account, a pro account, a common-creative license or
   // other things. In addition we can only handle JPEG images since our files must not change
   // file-extension while alive.
   sURI.Format(_T("farm%s.%s/%s/%s_%s.jpg"), pPhoto->sFarmID, YAHOO_FLICKR_URL_STATIC, pPhoto->sServerID, pPhoto->sPhotoID, pPhoto->sSecret);
   if( Type == VFS_STREAMTYPE_FILE )
   {
      if( pPhoto->sOrigFormat == _T("jpg") ) {
         sURI.Format(_T("farm%s.%s/%s/%s_%s_o.jpg"), pPhoto->sFarmID, YAHOO_FLICKR_URL_STATIC, pPhoto->sServerID, pPhoto->sPhotoID, pPhoto->sOrigSecret);
      }
      else {
         CSimpleArray<CString> aURLs;
         GetPhotoUrls(pPhoto, aURLs);
         for( int i = 0; i < aURLs.GetSize(); i++ ) {
            const CString& sURL = aURLs[i];
            if( sURL.Find(_T(".jpg")) > 0 ) sURI = sURL;
         }
         if( sURI.Left(7) == _T("http://") ) sURI = sURI.Mid(7);
         if( sURI.Left(8) == _T("https://") ) sURI = sURI.Mid(8);
      }
   }
   if( Type == VFS_STREAMTYPE_THUMBNAIL )
   {
      if( cxyThumb <= 75 ) sURI.Format(_T("farm%s.%s/%s/%s_%s_s.jpg"), pPhoto->sFarmID, YAHOO_FLICKR_URL_STATIC, pPhoto->sServerID, pPhoto->sPhotoID, pPhoto->sSecret);
      else if( cxyThumb <= 100 ) sURI.Format(_T("farm%s.%s/%s/%s_%s_t.jpg"), pPhoto->sFarmID, YAHOO_FLICKR_URL_STATIC, pPhoto->sServerID, pPhoto->sPhotoID, pPhoto->sSecret);
      else if( cxyThumb <= 240 ) sURI.Format(_T("farm%s.%s/%s/%s_%s_m.jpg"), pPhoto->sFarmID, YAHOO_FLICKR_URL_STATIC, pPhoto->sServerID, pPhoto->sPhotoID, pPhoto->sSecret);
      else if( cxyThumb <= 500 ) sURI.Format(_T("farm%s.%s/%s/%s_%s.jpg"), pPhoto->sFarmID, YAHOO_FLICKR_URL_STATIC, pPhoto->sServerID, pPhoto->sPhotoID, pPhoto->sSecret);
   }
   return S_OK;
}

HRESULT CFlickrAPI::SetPhotoProperty(TFlickrPhoto* pPhoto, REFPROPERTYKEY pkey, const CComPropVariant& v)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   CCoTaskString strValue;
   HR( ::PropVariantToStringAlloc(v, &strValue) );
   if( pkey == PKEY_Title )
   {
      FLICKR_APIARG ApiPhotosSetMeta[] =
      {
         { "method",       _T("flickr.photos.setMeta") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "title",        strValue },
         { "auth_token",   pPhoto->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosSetMeta, lengthof(ApiPhotosSetMeta)) );
      pPhoto->sTitle = strValue;
      pPhoto->sFilename = _ConstructFilename(pPhoto->sTitle, pPhoto->sPhotoID);
      return S_OK;
   }
   if( pkey == PKEY_Comment )
   {
      FLICKR_APIARG ApiPhotosSetMeta[] =
      {
         { "method",       _T("flickr.photos.setMeta") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "description",  strValue },
         { "auth_token",   pPhoto->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosSetMeta, lengthof(ApiPhotosSetMeta)) );
      pPhoto->sDescription = strValue;
      return S_OK;
   }
   if( pkey == PKEY_Keywords )
   {
      CString sKeywords = strValue;
      sKeywords.Replace(_T("; "), _T(","));
      FLICKR_APIARG ApiPhotosSetTags[] =
      {
         { "method",       _T("flickr.photos.setTags") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "tags",         sKeywords },
         { "auth_token",   pPhoto->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosSetTags, lengthof(ApiPhotosSetTags)) );
      pPhoto->sKeywords = strValue;
      HR( _RefreshAccountPhotosCache(pPhoto->pAccount) );
      return S_OK;
   }
   if( pkey == PKEY_DateCreated )
   {
      return ::PropVariantToFileTime(v, PSTF_UTC, &pPhoto->ftDateTaken);
   }
   if( pkey == PKEY_DateModified )
   {
      return ::PropVariantToFileTime(v, PSTF_UTC, &pPhoto->ftDatePosted);
   }
   return E_INVALIDARG;
}

HRESULT CFlickrAPI::AddPhotoProperty(TFlickrPhoto* pPhoto, REFPROPERTYKEY pkey, const CComPropVariant& v)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   CCoTaskString strValue;
   HR( ::PropVariantToStringAlloc(v, &strValue) );
   if( pkey == PKEY_Keywords )
   {
      CString sKeywords = strValue;
      sKeywords.Replace(_T("; "), _T(","));
      FLICKR_APIARG ApiPhotosAddTags[] =
      {
         { "method",       _T("flickr.photos.addTags") },
         { "api_key",      FLICKR_API_KEY },
         { "photo_id",     pPhoto->sPhotoID },
         { "tags",         sKeywords },
         { "auth_token",   pPhoto->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosAddTags, lengthof(ApiPhotosAddTags)) );
      if( !pPhoto->sKeywords.IsEmpty() ) pPhoto->sKeywords += _T("; ");
      pPhoto->sKeywords += strValue;
      return S_OK;
   }
   return E_INVALIDARG;
}

HRESULT CFlickrAPI::SetImagesetProperty(TFlickrImageset* pImageset, REFPROPERTYKEY pkey, const CComPropVariant& v)
{
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   CCoTaskString strValue;
   HR( ::PropVariantToStringAlloc(v, &strValue) );
   if( pkey == PKEY_Title )
   {
      if( pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET )
      {
         FLICKR_APIARG ApiPhotosetsEditMeta[] =
         {
            { "method",       _T("flickr.photosets.editMeta") },
            { "api_key",      FLICKR_API_KEY },
            { "photoset_id",  pImageset->sImagesetID },
            { "title",        strValue },
            { "auth_token",   pImageset->pAccount->sAuthToken }
         };
         HR( _SubmitFlickrRestAction(ApiPhotosetsEditMeta, lengthof(ApiPhotosetsEditMeta)) );
         pImageset->sTitle = strValue;
         pImageset->sFilename = _ConstructFilename(pImageset->sTitle, pImageset->sImagesetID);
         return S_OK;
      }
      if( pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH )
      {
         TCHAR szRegPath[250] = { 0 };
         ::wnsprintf(szRegPath, lengthof(szRegPath) - 1, _T("%s%s\\%s\\%s\\%s"), REGPATH_FLICKRFS, REGPATH_FLICKRFS_ACCOUNTS, pImageset->pAccount->sNSID, REGPATH_FLICKRFS_COLLECTIONS, pImageset->sImagesetID);
         CRegKey regImageset;
         if( regImageset.Open(HKEY_CURRENT_USER, szRegPath) != ERROR_SUCCESS ) return E_ACCESSDENIED;
         regImageset.SetStringValue(_T("Name"), strValue);
         pImageset->sTitle = strValue;
         if( pImageset->sKeywords.IsEmpty() ) {
            regImageset.SetStringValue(_T("Tags"), strValue);
            pImageset->sKeywords = strValue;
            pImageset->aPhotoIds.RemoveAll();
         }
         return S_OK;
      }
   }
   if( pkey == PKEY_Comment )
   {
      if( pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET )
      {
         FLICKR_APIARG ApiPhotosetsEditMeta[] =
         {
            { "method",       _T("flickr.photosets.editMeta") },
            { "api_key",      FLICKR_API_KEY },
            { "photoset_id",  pImageset->sImagesetID },
            { "title",        pImageset->sTitle },
            { "description",  strValue },
            { "auth_token",   pImageset->pAccount->sAuthToken }
         };
         HR( _SubmitFlickrRestAction(ApiPhotosetsEditMeta, lengthof(ApiPhotosetsEditMeta)) );
         pImageset->sDescription = strValue;
         return S_OK;
      }
      if( pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH )
      {
         TCHAR szRegPath[250] = { 0 };
         ::wnsprintf(szRegPath, lengthof(szRegPath) - 1, _T("%s%s\\%s\\%s\\%s"), REGPATH_FLICKRFS, REGPATH_FLICKRFS_ACCOUNTS, pImageset->pAccount->sNSID, REGPATH_FLICKRFS_COLLECTIONS, pImageset->sImagesetID);
         CRegKey regImageset;
         if( regImageset.Open(HKEY_CURRENT_USER, szRegPath) != ERROR_SUCCESS ) return E_ACCESSDENIED;
         regImageset.SetStringValue(_T("Description"), strValue);
         pImageset->sDescription = strValue;
         return S_OK;
      }
   }
   if( pkey == PKEY_Keywords )
   {
      if( pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH )
      {
         TCHAR szRegPath[250] = { 0 };
         ::wnsprintf(szRegPath, lengthof(szRegPath) - 1, _T("%s%s\\%s\\%s\\%s"), REGPATH_FLICKRFS, REGPATH_FLICKRFS_ACCOUNTS, pImageset->pAccount->sNSID, REGPATH_FLICKRFS_COLLECTIONS, pImageset->sImagesetID);
         CRegKey regImageset;
         if( regImageset.Open(HKEY_CURRENT_USER, szRegPath) != ERROR_SUCCESS ) return E_ACCESSDENIED;
         regImageset.SetStringValue(_T("Tags"), strValue);
         pImageset->sKeywords = strValue;
         pImageset->aPhotoIds.RemoveAll();
         return S_OK;
      }
   }
   return E_INVALIDARG;
}

// Implementation

HRESULT CFlickrAPI::_InitAPI()
{
   ::ZeroMemory(m_aPhotoHashmap, sizeof(m_aPhotoHashmap));
   ::ZeroMemory(&m_VolumeInfo, sizeof(m_VolumeInfo));
   ::GetLocalTime(&m_stThisDay);
   m_dwThisCount = 0;
   DWORD dwRegType = REG_DWORD, cbRegSize = sizeof(m_dwThisCount);
   ::SHGetValue(HKEY_CURRENT_USER, REGPATH_FLICKRFS, L"Disabled", &dwRegType, &m_dwThisCount, &cbRegSize);
   return S_OK;
}

HRESULT CFlickrAPI::_InitCrypto()
{
   if( !::CryptAcquireContext(&m_hCrypt.m_h, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET) ) return AtlHresultFromLastError();
   return S_OK;
}

HRESULT CFlickrAPI::_InitDummyAccount()
{
   m_pGuestAccount = new TFlickrAccount;
   m_pGuestAccount->sNSID = FLICKRFS_ACCOUNTSTR_GUEST;
   m_pGuestAccount->Access = FLICKRFS_ACCOUNT_GUEST;
   m_pGuestAccount->bIsFamily = FALSE;
   m_pGuestAccount->bIsFriend = FALSE;
   m_pGuestAccount->bIsPrimary = FALSE;
   m_pGuestAccount->bIsVirtual = FALSE;
   m_pGuestAccount->bIsProAccount = FALSE;
   return S_OK;
}

HRESULT CFlickrAPI::_RefreshAccountCache()
{  
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   // BUG: Memory leak. A massive memory leak occours because we don't delete these pointers.
   //      Unfortunately there are many references to these (ie. live NSE Items) so cleanup is
   //      a real problem here!!
   m_aAccounts.RemoveAll();
   m_aTempPhotos.RemoveAll();
   m_aDeletedPhotos.RemoveAll();
   // Re-read the account list and match up photos with new account entries
   _ReadAccountList();
   for( int i = 0; i < m_aPhotos.GetSize(); i++ ) {
      GetAccountFromNSID(m_aPhotos[i]->sOwnerNSID, &m_aPhotos[i]->pAccount);
   }
   return S_OK;
}

HRESULT CFlickrAPI::_RefreshEntirePhotoListCache()
{
   // Remove all photo-lists from all accounts. They will re-populate on the next
   // folder refresh.
   for( int i = 0; i < m_aAccounts.GetSize(); i++ ) _RefreshAccountPhotosCache(m_aAccounts[i]);
   // Reset volume info (since bandwidth requirements may change soon)
   m_VolumeInfo.ullUploadMaxBytes = 0;
   return S_OK;
}

HRESULT CFlickrAPI::_RefreshAccountPhotosCache(TFlickrAccount* pAccount)
{
   ATLASSERT(pAccount);
   if( pAccount == NULL ) return E_INVALIDARG;
   for( int i = 0; i < pAccount->aImagesets.GetSize(); i++ ) {
      pAccount->aImagesets[i]->aPhotoIds.RemoveAll();
   }
   return S_OK;
}

HRESULT CFlickrAPI::_ReadAccountList()
{
   // Initiate a fresh start
   ::SetErrorInfo(0, NULL);
   // Already populated?
   if( m_aAccounts.GetSize() > 0 ) return S_OK;
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( m_aAccounts.GetSize() > 0 ) return S_OK;
   // Build user list from Windows Registry...
   CRegKey regAccounts;
   if( regAccounts.Open(HKEY_CURRENT_USER, REGPATH_FLICKRFS REGPATH_FLICKRFS_ACCOUNTS, KEY_READ) != ERROR_SUCCESS ) return S_OK;
   for( DWORD iIndex = 0; true; iIndex++ ) {
      TCHAR szNSID[120] = { 0 };
      DWORD cchNSID = lengthof(szNSID);
      regAccounts.EnumKey(iIndex, szNSID, &cchNSID, NULL);
      if( szNSID[0] == '\0' ) break;
      CRegKey regAccount;
      if( regAccount.Open(regAccounts, szNSID, KEY_READ) != ERROR_SUCCESS ) break;
      TFlickrAccount* pAccount = new TFlickrAccount;
      TCHAR szUserName[60] = { 0 };
      TCHAR szFullName[200] = { 0 };
      TCHAR szLocation[200] = { 0 };
      TCHAR szAuthToken[60] = { 0 };
      ULONG cchUserName = lengthof(szUserName);
      ULONG cchFullName = lengthof(szFullName);
      ULONG cchLocation = lengthof(szLocation);
      ULONG cchAuthToken = lengthof(szAuthToken);
      DWORD dwIsFriend = 0, dwIsFamily = 0, dwIsPrimary = 0, dwIsPro = 0, dwAccess = 0;
      regAccount.QueryStringValue(_T("User Name"), szUserName, &cchUserName);
      regAccount.QueryStringValue(_T("Full Name"), szFullName, &cchFullName);
      regAccount.QueryStringValue(_T("Location"), szLocation, &cchLocation);
      regAccount.QueryStringValue(_T("Auth Token"), szAuthToken, &cchAuthToken);
      regAccount.QueryDWORDValue(_T("Is Friend"), dwIsFriend);
      regAccount.QueryDWORDValue(_T("Is Family"), dwIsFamily);
      regAccount.QueryDWORDValue(_T("Is Primary"), dwIsPrimary);
      regAccount.QueryDWORDValue(_T("Is Pro Account"), dwIsPro);
      regAccount.QueryDWORDValue(_T("Access"), dwAccess);
      pAccount->sNSID = szNSID;
      pAccount->sUserName = szUserName;
      pAccount->sFullName = szFullName;
      pAccount->sLocation = szLocation;
      pAccount->sAuthToken = szAuthToken;
      pAccount->sFilename = _ConstructFilename(szUserName, szNSID);
      pAccount->bIsFriend = (dwIsFriend == 1);
      pAccount->bIsFamily = (dwIsFamily == 1);
      pAccount->bIsVirtual = (szNSID[0] == '<');
      pAccount->bIsProAccount = (dwIsPro == 1);
      pAccount->bIsPrimary = (dwIsPrimary == 1);
      pAccount->Access = (TFlickrAccountType) dwAccess;
      m_aAccounts.Add(pAccount);
   }
   return S_OK;
}

HRESULT CFlickrAPI::_ReadImagesetList(TFlickrAccount* pAccount)
{
   ATLASSERT(pAccount);
   if( pAccount == NULL ) return E_INVALIDARG;
   // Already populated?
   if( pAccount->aImagesets.GetSize() > 0 ) return S_OK;
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pAccount->aImagesets.GetSize() > 0 ) return S_OK;
   // Populate list of imagesets (Photosets, groups, tag search folder, virtual folder, etc)
   if( !pAccount->bIsVirtual )
   {
      // Get the Photoset list...
      CComPtr<IXMLDOMDocument> spDoc;
      FLICKR_APIARG ApiPhotosetsGetList[] =
      {
         { "method",      _T("flickr.photosets.getList") },
         { "api_key",     FLICKR_API_KEY },
         { "user_id",     pAccount->sNSID },
         { "auth_token",  pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosetsGetList, lengthof(ApiPhotosetsGetList), &spDoc) );
      long lCount = 0;
      CComPtr<IXMLDOMNodeList> spNodeList;
      HR( _XmlGetResultList(spDoc, L"rsp/photosets/photoset", &spNodeList, &lCount) );
      for( long iIndex = 0; iIndex < lCount; iIndex++ ) {
         CComPtr<IXMLDOMNode> spNode;
         HR( spNodeList->get_item(iIndex, &spNode) );
         TFlickrImageset* pImageset = new TFlickrImageset;
         _XmlGetResultNode(spNode, L"@id", pImageset->sImagesetID);
         _XmlGetResultNode(spNode, L"@secret", pImageset->sSecret);
         _XmlGetResultNode(spNode, L"@server", pImageset->sServerID);
         _XmlGetResultNode(spNode, L"@farm", pImageset->sFarmID);
         _XmlGetResultNode(spNode, L"@primary", pImageset->sPrimaryID);
         _XmlGetResultNode(spNode, L"title", pImageset->sTitle);
         _XmlGetResultNode(spNode, L"description", pImageset->sDescription);
         pImageset->sFilename = _ConstructFilename(pImageset->sTitle, pImageset->sImagesetID);
         pImageset->lPriority = pAccount->aImagesets.GetSize() + 1;
         pImageset->Type = FLICKRFS_IMAGESET_PHOTOSET;
         pImageset->pAccount = pAccount;
         pAccount->aImagesets.Add(pImageset);
      }
   }
   if( pAccount->Access == FLICKRFS_ACCOUNT_OWNED )
   {
      // Get the Public Group list...
      CComPtr<IXMLDOMDocument> spDoc;
      FLICKR_APIARG ApiGroupsPoolsGetGroups[] =
      {
         { "method",      _T("flickr.groups.pools.getGroups") },
         { "api_key",     FLICKR_API_KEY },
         { "auth_token",  pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiGroupsPoolsGetGroups, lengthof(ApiGroupsPoolsGetGroups), &spDoc) );
      long lCount = 0;
      CComPtr<IXMLDOMNodeList> spNodeList;
      HR( _XmlGetResultList(spDoc, L"rsp/groups/group", &spNodeList, &lCount) );
      for( long iIndex = 0; iIndex < lCount; iIndex++ ) {
         CComPtr<IXMLDOMNode> spNode;
         HR( spNodeList->get_item(iIndex, &spNode) );
         TFlickrImageset* pImageset = new TFlickrImageset;
         _XmlGetResultNode(spNode, L"@nsid", pImageset->sImagesetID);
         _XmlGetResultNode(spNode, L"@name", pImageset->sTitle);
         pImageset->sFilename = _ConstructFilename(pImageset->sTitle, pImageset->sImagesetID);
         pImageset->lPriority = pAccount->aImagesets.GetSize() + 1;
         pImageset->Type = FLICKRFS_IMAGESET_GROUP;
         pImageset->pAccount = pAccount;
         pAccount->aImagesets.Add(pImageset);
      }
   }
   // Lookup tag-folders, favourites and other static imagesets...
   TCHAR szRegPath[250] = { 0 };
   ::wnsprintf(szRegPath, lengthof(szRegPath) - 1, _T("%s%s\\%s\\%s"), REGPATH_FLICKRFS, REGPATH_FLICKRFS_ACCOUNTS, pAccount->sNSID, REGPATH_FLICKRFS_COLLECTIONS);
   CRegKey regImagesets;
   if( regImagesets.Open(HKEY_CURRENT_USER, szRegPath, KEY_READ) == ERROR_SUCCESS ) {
      for( DWORD iIndex = 0; true; iIndex++ ) {
         TCHAR szImagesetID[120] = { 0 };
         DWORD cchszImagesetID = lengthof(szImagesetID);
         regImagesets.EnumKey(iIndex, szImagesetID, &cchszImagesetID, NULL);
         if( szImagesetID[0] == '\0' ) break;
         CRegKey regImageset;
         if( regImageset.Open(regImagesets, szImagesetID, KEY_READ) == ERROR_SUCCESS ) {
            TFlickrImageset* pImageset = new TFlickrImageset;
            DWORD dwType = 0;
            TCHAR szTitle[120] = { 0 };
            TCHAR szDescription[200] = { 0 };
            TCHAR szKeywords[200] = { 0 };
            ULONG cchTitle = lengthof(szTitle);
            ULONG cchKeywords = lengthof(szKeywords);
            ULONG cchDescription = lengthof(szDescription);
            regImageset.QueryStringValue(_T("Name"), szTitle, &cchTitle);
            regImageset.QueryStringValue(_T("Tags"), szKeywords, &cchKeywords);
            regImageset.QueryStringValue(_T("Description"), szDescription, &cchDescription);
            regImageset.QueryDWORDValue(_T("Type"), dwType);
            pImageset->sTitle = szTitle;
            pImageset->sDescription = szDescription;
            pImageset->sImagesetID = szImagesetID;
            pImageset->sKeywords = szKeywords;
            pImageset->sFilename = _ConstructFilename(szTitle, szImagesetID);
            pImageset->lPriority = pAccount->aImagesets.GetSize() + 1;
            pImageset->Type = (TFlickrImagesetType) dwType;
            pImageset->pAccount = pAccount;
            pAccount->aImagesets.Add(pImageset);
         }
      }
   }
   return S_OK;
}

HRESULT CFlickrAPI::_ReadImagesetPhotoList(TFlickrImageset* pImageset)
{
   ATLASSERT(pImageset);
   if( pImageset == NULL ) return E_INVALIDARG;
   // Already populated?
   if( pImageset->aPhotoIds.GetSize() > 0 ) return S_OK;
   // Did we recently read all the accounts?
   HR( _ReadAccountList() );
   // Query the photo list...
   LPCWSTR pstrPattern = NULL;
   CComPtr<IXMLDOMDocument> spDoc;
   if( pImageset->Type == FLICKRFS_IMAGESET_PHOTOSET ) 
   {
      FLICKR_APIARG ApiPhotosetsGetPhotos[] =
      {
         { "method",       _T("flickr.photosets.getPhotos") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS },
         { "photoset_id",  pImageset->sImagesetID },
         { "user_id",      pImageset->pAccount->sNSID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosetsGetPhotos, lengthof(ApiPhotosetsGetPhotos), &spDoc) );
      pstrPattern = L"rsp/photoset/photo";
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_FAVOURITES ) 
   {
      FLICKR_APIARG ApiFavoritesGetList[] =
      {
         { "method",       _T("flickr.favorites.getList") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS },
         { "user_id",      pImageset->pAccount->sNSID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiFavoritesGetList, lengthof(ApiFavoritesGetList), &spDoc) );
      pstrPattern = L"rsp/photos/photo";
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_PUBLIC_FAVOURITES ) 
   {
      FLICKR_APIARG ApiFavoritesGetPublicList[] =
      {
         { "method",       _T("flickr.favorites.getPublicList") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS },
         { "user_id",      pImageset->pAccount->sNSID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiFavoritesGetPublicList, lengthof(ApiFavoritesGetPublicList), &spDoc) );
      pstrPattern = L"rsp/photos/photo";
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_GROUP ) 
   {
      FLICKR_APIARG ApiGroupsPoolsGetPhotos[] =
      {
         { "method",       _T("flickr.groups.pools.getPhotos") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS },
         { "group_id",     pImageset->sImagesetID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiGroupsPoolsGetPhotos, lengthof(ApiGroupsPoolsGetPhotos), &spDoc) );
      pstrPattern = L"rsp/photos/photo";
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_TAGSEARCH ) 
   {
      CString sKeywords = pImageset->sKeywords;
      sKeywords.Replace(_T("; "), _T(","));
      FLICKR_APIARG ApiPhotosSearch[] =
      {
         { "method",       _T("flickr.photos.search") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS },
         { "tags",         sKeywords },
         { "user_id",      pImageset->pAccount->sNSID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosSearch, lengthof(ApiPhotosSearch), &spDoc) );
      pstrPattern = L"rsp/photos/photo";
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_NOTINSET ) 
   {
      FLICKR_APIARG ApiPhotosGetNotInSet[] =
      {
         { "method",       _T("flickr.photos.getNotInSet") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS },
         { "user_id",      pImageset->pAccount->sNSID },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosGetNotInSet, lengthof(ApiPhotosGetNotInSet), &spDoc) );
      pstrPattern = L"rsp/photos/photo";
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_RECENT ) 
   {
      SYSTEMTIME stNow = { 0 };
      FILETIME ftNow = { 0 };
      ::GetSystemTime(&stNow);
      ::SystemTimeToFileTime(&stNow, &ftNow);
      ULONGLONG ullNow = (ULONGLONG) ftNow.dwLowDateTime + ((ULONGLONG) ftNow.dwHighDateTime << 32);
      ullNow -= 10000000I64 * 134774I64 * 24I64 * 60I64 * 60I64;
      time_t tNow = (time_t) ullNow / 10000000I64;
      CString sTimestamp;
      const time_t RECENT_DAYS = 31;
      sTimestamp.Format(_T("%lu"), tNow - (RECENT_DAYS * 24 * 60 * 60));
      FLICKR_APIARG ApiPhotosGetRecentlyUpdated[] =
      {
         { "method",       _T("flickr.photos.recentlyUpdated") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS },
         { "min_date",     sTimestamp },
         { "auth_token",   pImageset->pAccount->sAuthToken }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosGetRecentlyUpdated, lengthof(ApiPhotosGetRecentlyUpdated), &spDoc) );
      pstrPattern = L"rsp/photos/photo";
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_INTERESTING ) 
   {
      FLICKR_APIARG ApiInterestingnessGetList[] =
      {
         { "method",       _T("flickr.interestingness.getList") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS }
      };
      HR( _SubmitFlickrRestAction(ApiInterestingnessGetList, lengthof(ApiInterestingnessGetList), &spDoc) );
      pstrPattern = L"rsp/photos/photo";
   }
   if( pImageset->Type == FLICKRFS_IMAGESET_HOTTAGS ) 
   {
      FLICKR_APIARG ApiTagsGetHotList[] =
      {
         { "method",       _T("flickr.tags.getHotList") },
         { "api_key",      FLICKR_API_KEY },
         { "period",       _T("day") },
         { "count",        _T("5") }
      };
      HR( _SubmitFlickrRestAction(ApiTagsGetHotList, lengthof(ApiTagsGetHotList), &spDoc) );
      CSimpleArray<CString> aTags;
      HR( _XmlGetResultList(spDoc, L"rsp/hottags/tag", L".", aTags) );
      CString sTagList;
      for( int i = 0; i < aTags.GetSize(); i++ ) {
         if( !sTagList.IsEmpty() ) sTagList += _T(",");
         sTagList += aTags[i];
      }
      spDoc.Release();
      FLICKR_APIARG ApiPhotosSearch[] =
      {
         { "method",       _T("flickr.photos.search") },
         { "api_key",      FLICKR_API_KEY },
         { "extras",       FLICKR_PHOTO_EXTRAS },
         { "tags",         sTagList }
      };
      HR( _SubmitFlickrRestAction(ApiPhotosSearch, lengthof(ApiPhotosSearch), &spDoc) );
      pstrPattern = L"rsp/photos/photo";
   }
   if( pstrPattern == NULL ) return E_FAIL;
   // Now that we've got the list, parse it...
   return _ReadImagesetPhotoListResult(pImageset, spDoc, pstrPattern);
}

HRESULT CFlickrAPI::_ReadImagesetPhotoListResult(TFlickrImageset* pImageset, IUnknown* pUnk, LPCWSTR pstrPattern)
{
   ATLASSERT(pUnk);
   ATLASSERT(pstrPattern);
   CComCritSecLock<CComCriticalSection> lock(m_lock);
   if( pImageset != NULL && pImageset->aPhotoIds.GetSize() > 0 ) return S_OK;
   // Extract photo list from resultset...
   long lCount = 0;
   CComPtr<IXMLDOMNodeList> spNodeList;
   HR( _XmlGetResultList(pUnk, pstrPattern, &spNodeList, &lCount) );
   for( long iIndex = 0; iIndex < lCount; iIndex++ ) {
      CComPtr<IXMLDOMNode> spNode;
      HR( spNodeList->get_item(iIndex, &spNode) );
      TFlickrPhoto* pPhoto = new TFlickrPhoto;
      CString sDateTaken, sDatePosted, sIsPublic, sIsFriend, sIsFamily;
      _XmlGetResultNode(spNode, L"@id", pPhoto->sPhotoID);
      _XmlGetResultNode(spNode, L"@secret", pPhoto->sSecret);
      _XmlGetResultNode(spNode, L"@server", pPhoto->sServerID);
      _XmlGetResultNode(spNode, L"@farm", pPhoto->sFarmID);
      _XmlGetResultNode(spNode, L"@title", pPhoto->sTitle);
      _XmlGetResultNode(spNode, L"@owner", pPhoto->sOwnerNSID);
      _XmlGetResultNode(spNode, L"@ispublic", sIsPublic);
      _XmlGetResultNode(spNode, L"@isfriend", sIsFriend);
      _XmlGetResultNode(spNode, L"@isfamily", sIsFamily);
      _XmlGetResultNode(spNode, L"@datetaken", sDateTaken);
      _XmlGetResultNode(spNode, L"@lastupdate", sDatePosted);
      if( pPhoto->sTitle.IsEmpty() ) {
         _XmlGetResultNode(spNode, L"title", pPhoto->sTitle);
         _XmlGetResultNode(spNode, L"description", pPhoto->sDescription);
         _XmlGetResultNode(spNode, L"owner/@nsid", pPhoto->sOwnerNSID);
         _XmlGetResultNode(spNode, L"owner/@username", pPhoto->sOwnerUsername);
         _XmlGetResultNode(spNode, L"dates/@taken", sDateTaken);
         _XmlGetResultNode(spNode, L"dates/@posted", sDatePosted);
         _XmlGetResultNode(spNode, L"visibility/@isfriend", sIsFriend);
         _XmlGetResultNode(spNode, L"visibility/@isfamily", sIsFamily);
      }
      // Adjust values
      pPhoto->bIsPublic = (sIsPublic == _T("1"));
      pPhoto->bIsFriend = (sIsFriend == _T("1"));
      pPhoto->bIsFamily = (sIsFamily == _T("1"));
      pPhoto->bIsSpawned = FALSE;
      // Resolve account
      pPhoto->pAccount = m_pGuestAccount;
      if( pImageset != NULL && pPhoto->sOwnerNSID.IsEmpty() ) pPhoto->pAccount = pImageset->pAccount;
      GetAccountFromNSID(pPhoto->sOwnerNSID, &pPhoto->pAccount);
      pPhoto->bIsOwned = (pPhoto->pAccount->Access == FLICKRFS_ACCOUNT_OWNED);
      // Various dates
      pPhoto->ftDateTaken.dwLowDateTime = pPhoto->ftDateTaken.dwHighDateTime = 0;
      pPhoto->ftDatePosted.dwLowDateTime = pPhoto->ftDatePosted.dwHighDateTime = 0;
      if( !sDateTaken.IsEmpty() ) {
         // YYYY-MM-DD HH:NN:SS
         if( sDateTaken.GetLength() < 19 ) sDateTaken += _T("                    ");
         LPCTSTR pstr = sDateTaken;
         SYSTEMTIME st = { (WORD) _ttoi(pstr), (WORD) _ttoi(pstr + 5), 0U, (WORD) _ttoi(pstr + 8), (WORD) _ttoi(pstr + 11), (WORD) _ttoi(pstr + 14), (WORD) _ttoi(pstr + 17), 0 };
         ::SystemTimeToFileTime(&st, &pPhoto->ftDateTaken);
      }
      if( !sDatePosted.IsEmpty() ) {
         // UNIX time
         time_t t = (time_t) _ttol(sDatePosted);
         LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
         pPhoto->ftDatePosted.dwLowDateTime = (DWORD) ll;
         pPhoto->ftDatePosted.dwHighDateTime = ll >> 32;
      } 
      // Filename and extension
      pPhoto->sFilename = _ConstructFilename(pPhoto->sTitle, pPhoto->sPhotoID);
      pPhoto->sExtension = _T(".jpg");
      if( pPhoto->sTitle.IsEmpty() ) pPhoto->sTitle = pPhoto->sPhotoID;
      // Add to Imageset list
      if( pImageset != NULL ) pImageset->aPhotoIds.Add(pPhoto->sPhotoID);
      // Is it already in global cache or will be added last...
      int iListIndex = 0;
      bool bFound = false;
      for( ; !bFound && iListIndex < m_aPhotos.GetSize(); iListIndex++ ) {
         if( m_aPhotos[iListIndex]->sPhotoID == pPhoto->sPhotoID ) { bFound = true; break; }
      }
      // Add to hash map...
      DWORD dwHashIdx = _UInt4Hash(pPhoto->sPhotoID) % lengthof(m_aPhotoHashmap);
      m_aPhotoHashmap[dwHashIdx] = iListIndex;
      // Add to global photo cache?
      if( bFound ) delete pPhoto; else m_aPhotos.Add(pPhoto);
   }
   return lCount > 0 ? S_OK : S_FALSE;
}

CString CFlickrAPI::_ConstructFilename(LPCTSTR pstrDesiredName, LPCTSTR pstrFallbackName) const
{
   CString sFilename = CString(pstrDesiredName).Left(MAX_PATH - 5);
   static LPCTSTR s_pstrInvalidWin32Chars = _T("./\\'\":;<>|*?");
   for( LPCTSTR p = s_pstrInvalidWin32Chars; *p != '\0'; p++ ) sFilename.Remove(*p);
   if( sFilename.IsEmpty() ) sFilename = pstrFallbackName;
   return sFilename;
}

// Flickr REST API

HRESULT CFlickrAPI::_SubmitFlickrRestAction(FLICKR_APIARG* pArgs, int nArgs, IXMLDOMDocument** ppXmlDoc /*= NULL*/)
{
   ATLTRACE(L"CFlickrAPI::SubmitFlickrRestAction  method='%s'\n", pArgs[0].pstrValue);
   // Reset COM error
   ::SetErrorInfo(0, NULL);
   // Temporary return value
   CComPtr<IXMLDOMDocument> spTempDoc;
   if( ppXmlDoc == NULL ) ppXmlDoc = &spTempDoc;
   // Build Flickr url and submit HTTP request
   CString sURL = _RestBuildActionUrl(FLICKR_APITYPE_REST, pArgs, nArgs);
   LPBYTE pData = NULL; DWORD cbData = 0;
   HR( _RestSendRequest(YAHOO_FLICKR_URL_ROOT, sURL, _T("POST"), 0, NULL, 0, pData, cbData) );
   HRESULT Hr = _RestExtractResult(reinterpret_cast<LPSTR>(pData), ppXmlDoc);
   free(pData);
   return Hr;
}

CString CFlickrAPI::_RestBuildActionUrl(FLICKR_APITYPE ApiType, FLICKR_APIARG* pArgs, int nArgs)
{
   // First fixup args...
   ATLASSERT(nArgs>0);
   if( strcmp(pArgs[nArgs - 1].pstrName, "auth_token") == 0 && pArgs[nArgs - 1].pstrValue[0] == '\0' ) --nArgs;
   if( strcmp(pArgs[nArgs - 1].pstrName, "user_id") == 0    && pArgs[nArgs - 1].pstrValue[0] == '<' )  --nArgs;
   if( strcmp(pArgs[nArgs - 1].pstrName, "secret") == 0     && pArgs[nArgs - 1].pstrValue[0] == '\0' ) --nArgs;
   // Sort the api methods...
   for( int a = 0; a < nArgs; a++ ) {
      for( int b = a + 1; b < nArgs; b++ ) {
         if( strcmp(pArgs[a].pstrName, pArgs[b].pstrName) > 0 ) {
            FLICKR_APIARG temp = pArgs[a];
            pArgs[a] = pArgs[b];
            pArgs[b] = temp;
         }
      }
   }
   // Generate timestamp (to cheat any proxy/router that defies no-cache headers)
   static DWORD s_dwTimestamp = 0;
   TCHAR szTimestamp[20] = { 0 };
   ::wsprintf(szTimestamp, _T("%lu"), ++s_dwTimestamp);
   // Create signature for MD5 checksum
   CString sApiSig = FLICKR_SECRET_KEY;
   for( int x = 0; x < nArgs; x++ ) {
      sApiSig += pArgs[x].pstrName;
      sApiSig += pArgs[x].pstrValue;
   }
   if( ApiType == FLICKR_APITYPE_REST ) {
      sApiSig += _T("zz");
      sApiSig += szTimestamp;
   }
   TCHAR szMd5Signature[33] = { 0 };
   _MD5Hash(CW2AEX<>(sApiSig, CP_UTF8), szMd5Signature);
   // The upload session is only interested in the MD5 hash
   if( ApiType == FLICKR_APITYPE_UPLOAD ) return szMd5Signature;
   // Create final API url.
   // NOTE: Conversion of URL path to UTF-8 and escaping is done by WinHTTP by default
   //       so no need to take that into consideration now.
   CString sUrlSig = YAHOO_FLICKR_API_URLPATH;
   if( ApiType == FLICKR_APITYPE_AUTH ) sUrlSig = YAHOO_FLICKR_AUTH_URLPATH;
   sUrlSig += _T("?");
   for( int i = 0; i < nArgs; i++ ) {
      if( i > 0 ) sUrlSig += _T("&");
      sUrlSig += pArgs[i].pstrName;
      sUrlSig += _T("=");
      sUrlSig += pArgs[i].pstrValue;
   }
   if( ApiType == FLICKR_APITYPE_REST ) {
      sUrlSig += _T("&zz=");
      sUrlSig += szTimestamp;
   }
   sUrlSig += _T("&api_sig=");
   sUrlSig += szMd5Signature;
   return sUrlSig;
}

HRESULT CFlickrAPI::_RestSendRequest(LPCWSTR pstrHost, LPCTSTR pstrUrl, LPCTSTR pstrMethod, DWORD dwFlags, LPSTR pstrPost, DWORD cbPost, LPBYTE& pstrData, DWORD& cbData)
{
   HR( _CheckQueryQuota() );
   HR( InitHttpService() );
   cbData = 0;
   CAutoInternetHandle hConnect( ::WinHttpConnect(m_hInternet, pstrHost, INTERNET_DEFAULT_HTTPS_PORT, 0) );
   if( hConnect == NULL ) return AtlHresultFromLastError();
   CAutoInternetHandle hRequest( ::WinHttpOpenRequest(hConnect, pstrMethod, pstrUrl, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) );
   if( hRequest == NULL ) return AtlHresultFromLastError();
   BOOL bResult = ::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, pstrPost, cbPost, cbPost, 0);
   if( !bResult ) return AtlHresultFromLastError();
   bResult = ::WinHttpReceiveResponse(hRequest, NULL);
   if( !bResult ) return AtlHresultFromLastError();
   DWORD dwPos = 0;
   DWORD cbAlloc = 20 * 1024;
   pstrData = (LPBYTE) malloc(cbAlloc);
   while( pstrData != NULL ) {
      DWORD cbAvail = 0;
      bResult = ::WinHttpQueryDataAvailable(hRequest, &cbAvail);
      if( !bResult || cbAvail == 0 ) break;
      if( dwPos + cbAvail >= cbAlloc - 1 ) {
         while( dwPos + cbAvail >= cbAlloc - 1 ) cbAlloc *= 2;
         pstrData = (LPBYTE) realloc(pstrData, cbAlloc);
         if( pstrData == NULL ) return E_OUTOFMEMORY;
      }
      DWORD dwBytesRead = 0;
      bResult = ::WinHttpReadData(hRequest, pstrData + dwPos, cbAvail, &dwBytesRead);
      if( !bResult ) break;
      dwPos += dwBytesRead;
   }   
   if( !bResult ) {
      HRESULT Hr = AtlHresultFromLastError();
      free(pstrData); pstrData = NULL; cbData = 0;
      return Hr;
   }
   pstrData[dwPos] = '\0';
   return S_OK;
}

HRESULT CFlickrAPI::_CheckQueryQuota()
{
   // Must somehow prevent excessive REST lookups on the Flickr-site. If the PC has an index-service/search/anti-virus
   // or anything installed that causes too high load on Flickr, we'll simply disable the tool on the PC!!!
   const DWORD MAX_QUOTA = 3000;
   ++m_dwThisCount;
   if( m_dwThisCount == MAX_QUOTA ) ::SHSetValue(HKEY_CURRENT_USER, REGPATH_FLICKRFS, L"Disabled", REG_DWORD, &m_dwThisCount, sizeof(m_dwThisCount));
   return m_dwThisCount < MAX_QUOTA ? S_OK : AtlHresultFromWin32(ERROR_SERVICE_DISABLED);
}

HRESULT CFlickrAPI::_RestExtractResult(LPCSTR pstrXML, IXMLDOMDocument** ppDoc)
{
   ATLASSERT(pstrXML);
   if( pstrXML == NULL ) return E_FAIL;
   CComPtr<IXMLDOMDocument> spDoc;
   HRESULT Hr = E_UNEXPECTED;
   if( FAILED(Hr) ) Hr = spDoc.CoCreateInstance(L"Msxml2.DOMDocument.6.0");
   if( FAILED(Hr) ) Hr = spDoc.CoCreateInstance(L"Msxml2.DOMDocument.4.0");
   if( FAILED(Hr) ) Hr = spDoc.CoCreateInstance(L"Msxml2.DOMDocument");
   if( FAILED(Hr) ) return Hr;
   HR( spDoc->put_async(VARIANT_FALSE) );
   CComVariant vXML;
   HR( ::InitVariantFromBuffer(pstrXML, (UINT) strlen(pstrXML), &vXML) );
   VARIANT_BOOL vbSuccess = VARIANT_FALSE;
   HR( spDoc->load(vXML, &vbSuccess) );
   if( vbSuccess == VARIANT_FALSE ) return E_FAIL;
   CString sStatus;
   HR( _XmlGetResultNode(spDoc, L"rsp/@stat", sStatus) );
   if( sStatus == _T("ok") ) return spDoc.CopyTo(ppDoc);
   CString sErrCode, sErrorMsg;
   _XmlGetResultNode(spDoc, L"rsp/err/@code", sErrCode);
   _XmlGetResultNode(spDoc, L"rsp/err/@msg", sErrorMsg);
   // Translate error code if possible and create COM error report
   Hr = E_FAIL;
   static DWORD s_aErrors[] = { 1, ERROR_NOT_FOUND, 2, ERROR_USER_PROFILE_LOAD, 3, ERROR_BAD_ARGUMENTS, 4, ERROR_ACCESS_DENIED, 10, ERROR_SERVICE_DISABLED, 98, ERROR_ACCESS_DENIED, 99, ERROR_ACCESS_DENIED, 100, ERROR_SERVICE_DISABLED, 105, ERROR_SERVICE_DISABLED };
   for( int i = 0; i < lengthof(s_aErrors); i += 2 ) if( _ttol(sErrCode) == (long) s_aErrors[i] ) Hr = AtlHresultFromWin32(s_aErrors[i + 1]);
   return AtlReportError(CLSID_ShellFolder, sErrorMsg, GUID_NULL, Hr);
}

HRESULT CFlickrAPI::_XmlGetResultNode(IUnknown* pUnk, LPCWSTR pstrPattern, CString& sValue)
{
   ATLASSERT(pUnk);
   ATLASSERT(pstrPattern);
   if( pUnk == NULL ) return E_FAIL;
   CComQIPtr<IXMLDOMNode> spNode = pUnk;
   if( spNode == NULL ) return E_NOINTERFACE;
   CComPtr<IXMLDOMNode> spResult;
   HR( spNode->selectSingleNode(CComBSTR(pstrPattern), &spResult) );
   if( spResult == NULL ) return E_FAIL;
   CComBSTR bstr;
   HR( spResult->get_text(&bstr) );
   sValue = bstr;
   return S_OK;
}

HRESULT CFlickrAPI::_XmlGetResultList(IUnknown* pUnk, LPCWSTR pstrPattern, IXMLDOMNodeList** ppList, long* pCount)
{
   ATLASSERT(pUnk);
   ATLASSERT(pstrPattern);
   if( pUnk == NULL ) return E_FAIL;
   CComQIPtr<IXMLDOMNode> spNode = pUnk;
   if( spNode == NULL ) return E_NOINTERFACE;
   HR( spNode->selectNodes(CComBSTR(pstrPattern), ppList) );
   if( *ppList == NULL ) return E_FAIL;
   HR( (*ppList)->get_length(pCount) );
   return S_OK;
}

HRESULT CFlickrAPI::_XmlGetResultList(IUnknown* pUnk, LPCWSTR pstrPattern, LPCWSTR pstrAttribute, CSimpleArray<CString>& aList)
{
   long lCount = 0;
   CComPtr<IXMLDOMNodeList> spNodeList;
   HR( _XmlGetResultList(pUnk, pstrPattern, &spNodeList, &lCount) );
   for( long iIndex = 0; iIndex < lCount; iIndex++ ) {
      CComPtr<IXMLDOMNode> spNode;
      HR( spNodeList->get_item(iIndex, &spNode) );
      CString sValue;
      HR( _XmlGetResultNode(spNode, pstrAttribute, sValue) );
      if( !aList.Add(sValue) ) return E_OUTOFMEMORY;
   }
   return S_OK;
}

BOOL CFlickrAPI::_MD5Hash(LPCSTR data, LPTSTR pstrResult)
{
   CAutoCryptHashHandle hHash;
   if( !::CryptCreateHash(m_hCrypt, CALG_MD5, 0, 0, &hHash.m_h) ) return FALSE;
   SIZE_T len = strlen(data);
   if( !::CryptHashData(hHash, (LPCBYTE) data, (DWORD) len, 0) ) return FALSE;
   BYTE bHash[16];
   DWORD dwHashLen = 16;
   if( !::CryptGetHashParam(hHash, HP_HASHVAL, bHash, &dwHashLen, 0) ) return FALSE;
   for( int i = 0; i < 16; i++ ) ::wsprintf(pstrResult + (i * 2), _T("%02x"), bHash[i]);
   return TRUE;
}

DWORD CFlickrAPI::_UInt4Hash(LPCWSTR pstr) const
{
   DWORD dwHash = 0x811C9DC5;
   for( ; *pstr != '\0'; pstr++ ) dwHash += (dwHash * 8) + (*pstr);
   return dwHash;
}

// The "Add New Account" helpers

HRESULT CFlickrAPI::_DoAccountWebAuthentication(HWND hWnd, LPCTSTR pstrUsername, TFlickrAccountType AccountType, CString& sAuthToken)
{
   // Only "managed accounts" require authentication; the other account types
   // have limited functionality.
   if( AccountType != FLICKRFS_ACCOUNT_OWNED ) return S_OK;

   CComPtr<IXMLDOMDocument> spDoc;
   FLICKR_APIARG ApiAuthGetFrob[] =
   {
      { "method",       _T("flickr.auth.getFrob") },
      { "api_key",      FLICKR_API_KEY },
   };
   HR( _SubmitFlickrRestAction(ApiAuthGetFrob, lengthof(ApiAuthGetFrob), &spDoc) );
   CString sFrob;
   HR( _XmlGetResultNode(spDoc, L"rsp/frob", sFrob) );

   // Ask user to go the YAHOO website and change permissions for
   // our glorious Flickr application.
   FLICKR_APIARG ApiAuthPerms[] =
   {
      { "api_key",      FLICKR_API_KEY },
      { "frob",         sFrob },
      { "perms",        _T("delete") },
   };
   CString sAuth = _RestBuildActionUrl(FLICKR_APITYPE_AUTH, ApiAuthPerms, lengthof(ApiAuthPerms));
   CString sURL;
   sURL.Format(_T("https://%s%s"), YAHOO_FLICKR_URL_ROOT, sAuth);
   if( DoWebAuthentication(hWnd, sURL) != IDOK ) return E_ABORT;

   // Make sure the user completes the authentication; ask for the access token
   // until we get it or the user cancels the operation.
   UINT nRes = IDCANCEL;
   do 
   {
      FLICKR_APIARG ApiAuthGetToken[] =
      {
         { "method",       _T("flickr.auth.getToken") },
         { "api_key",      FLICKR_API_KEY },
         { "frob",         sFrob },
      };
      spDoc.Release();
      if( SUCCEEDED( _SubmitFlickrRestAction(ApiAuthGetToken, lengthof(ApiAuthGetToken), &spDoc) ) ) {
         // Did we get the access token?
         _XmlGetResultNode(spDoc, L"rsp/auth/token", sAuthToken);
         if( !sAuthToken.IsEmpty() ) return S_OK;
      }

      nRes = DoWebAuthenticationCompleted(hWnd, sURL);
   } 
   while( nRes == IDOK );

   return AtlHresultFromWin32(ERROR_NOT_AUTHENTICATED);
}

HRESULT CFlickrAPI::_DoDefaultAccountSetup(LPCTSTR pstrUsername, TFlickrAccountType AccountType, CString sAuthToken, BOOL bIsPrimary, BOOL bAddFriends, BOOL bAddFamily)
{
   CWaitCursor cursor;

   // Get NSID from a Flickr username...
   CComPtr<IXMLDOMDocument> spDoc;
   FLICKR_APIARG ApiPeopleFindByUsername[] =
   {
      { "method",       _T("flickr.people.findByUsername") },
      { "api_key",      FLICKR_API_KEY },
      { "username",     pstrUsername },
   };
   HR( _SubmitFlickrRestAction(ApiPeopleFindByUsername, lengthof(ApiPeopleFindByUsername), &spDoc) );
   CString sNSID;
   HR( _XmlGetResultNode(spDoc, L"rsp/user/@nsid", sNSID) );
   spDoc.Release();

   // Get account details...
   FLICKR_APIARG ApiPeopleGetInfo[] =
   {
      { "method",       _T("flickr.people.getInfo") },
      { "api_key",      FLICKR_API_KEY },
      { "user_id",      sNSID },
   };
   HR( _SubmitFlickrRestAction(ApiPeopleGetInfo, lengthof(ApiPeopleGetInfo), &spDoc) );
   CString sUsername, sFullName, sLocation, sIsPro, sIsFriend, sIsFamily;
   _XmlGetResultNode(spDoc, L"rsp/person/@ispro", sIsPro);
   _XmlGetResultNode(spDoc, L"rsp/person/@friend", sIsFriend);
   _XmlGetResultNode(spDoc, L"rsp/person/@family", sIsFamily);
   _XmlGetResultNode(spDoc, L"rsp/person/username", sUsername);
   _XmlGetResultNode(spDoc, L"rsp/person/realname", sFullName);
   _XmlGetResultNode(spDoc, L"rsp/person/location", sLocation);
   spDoc.Release();

   // Create account in Registry database...
   CRegKey regAccounts;
   if( regAccounts.Create(HKEY_CURRENT_USER, REGPATH_FLICKRFS REGPATH_FLICKRFS_ACCOUNTS) != ERROR_SUCCESS ) return E_ACCESSDENIED;
   CRegKey regAccount;
   if( regAccount.Create(regAccounts, sNSID) != ERROR_SUCCESS ) return E_ACCESSDENIED;
   regAccount.SetStringValue(_T("User Name"), sUsername);
   regAccount.SetStringValue(_T("Full Name"), sFullName);
   regAccount.SetStringValue(_T("Location"), sLocation);
   regAccount.SetStringValue(_T("Auth Token"), sAuthToken);
   regAccount.SetDWORDValue(_T("Is Friend"), sIsFriend == _T("1") ? 1 : 0);
   regAccount.SetDWORDValue(_T("Is Family"), sIsFamily == _T("1") ? 1 : 0);
   regAccount.SetDWORDValue(_T("Is Pro Account"), sIsPro == _T("1") ? 1 : 0);
   regAccount.SetDWORDValue(_T("Is Primary"), bIsPrimary);
   regAccount.SetDWORDValue(_T("Access"), AccountType);

   CComBSTR bstr;
   CRegKey regCollections;
   if( regCollections.Create(regAccount, REGPATH_FLICKRFS_COLLECTIONS) != ERROR_SUCCESS ) return E_ACCESSDENIED;

   if( AccountType == FLICKRFS_ACCOUNT_OWNED ) {
      HR( _CreateAccountFolder(regCollections, FLICKRFS_FOLDERSTR_FAVOURITES, IDS_FOLDER_FAVOURITES, FLICKRFS_IMAGESET_FAVOURITES) );
      HR( _CreateAccountFolder(regCollections, FLICKRFS_FOLDERSTR_NOTINSET, IDS_FOLDER_NOTINSET, FLICKRFS_IMAGESET_NOTINSET) );
      HR( _CreateAccountFolder(regCollections, FLICKRFS_FOLDERSTR_RECENT, IDS_FOLDER_RECENT, FLICKRFS_IMAGESET_RECENT) );
   }
   else {
      HR( _CreateAccountFolder(regCollections, FLICKRFS_FOLDERSTR_FAVOURITES, IDS_FOLDER_FAVOURITES, FLICKRFS_IMAGESET_PUBLIC_FAVOURITES) );
   }

   regCollections.Close();
   regAccount.Close();

   // Add friends and family...
   if( AccountType == FLICKRFS_ACCOUNT_OWNED && (bAddFriends || bAddFamily) ) 
   {
      FLICKR_APIARG ApiContactsGetList[] =
      {
         { "method",       _T("flickr.contacts.getList") },
         { "api_key",      FLICKR_API_KEY },
         { "username",     pstrUsername },
         { "auth_token",   sAuthToken }
      };
      _SubmitFlickrRestAction(ApiContactsGetList, lengthof(ApiContactsGetList), &spDoc);
      long lCount = 0;
      CComPtr<IXMLDOMNodeList> spNodeList;
      _XmlGetResultList(spDoc, L"rsp/contacts/contact", &spNodeList, &lCount);
      for( long iIndex = 0; iIndex < lCount; iIndex++ ) {
         CComPtr<IXMLDOMNode> spNode;
         HR( spNodeList->get_item(iIndex, &spNode) );
         CString sUsername, sIsFriend, sIsFamily;
         _XmlGetResultNode(spNode, L"@username", sUsername);
         _XmlGetResultNode(spNode, L"@friend", sIsFriend);
         _XmlGetResultNode(spNode, L"@family", sIsFamily);
         if( (bAddFriends && sIsFriend == _T("1")) 
             || (bAddFamily && sIsFamily == _T("1")) ) 
         {
            _DoDefaultAccountSetup(sUsername, FLICKRFS_ACCOUNT_GUEST, _T(""), FALSE, FALSE, FALSE);
         }
      }
   }

   return S_OK;
}

HRESULT CFlickrAPI::_CreateAccountFolder(HKEY hKeyCollection, LPCTSTR pszFolderRegName, UINT uDisplayName, TFlickrImagesetType ImagesetType)
{
   if( hKeyCollection == NULL ) return E_FAIL;
   CRegKey regFolder;
   if( regFolder.Create(hKeyCollection, pszFolderRegName) != ERROR_SUCCESS ) return E_ACCESSDENIED;
   CComBSTR bstrName; bstrName.LoadString(uDisplayName);
   regFolder.SetStringValue(_T("Name"), bstrName);
   regFolder.SetDWORDValue(_T("Type"), ImagesetType);
   regFolder.Close();
   return S_OK;
}

