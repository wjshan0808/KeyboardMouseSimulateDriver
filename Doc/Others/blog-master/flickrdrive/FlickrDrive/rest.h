/////////////////////////////////////////////////////////////////////////////
// FlickDrive Shell Extension
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2009 Bjarke Viksoe.
//
// This is an implementation of the Flickr REST API.
// Documentation available here: 
//   http://www.flickr.com/services/api/
//

#pragma once

#include <atlstr.h>
#include <winhttp.h>


///////////////////////////////////////////////////////////////////////////////
// Flickr API Secret

// Due to security, my Flickr API Secret key is not included in this distribution.
// Create a password.h file in the \FlickrDrive folder with the following lines:
//  #define FLICKR_API_KEY             _T("33333333333333333333333333333333")
//  #define FLICKR_SECRET_KEY          _T("0123456789012345")
// where 333... is your Flickr app key and 0123... is the secret. Both can be 
// obtained from the Flickr website.
// More info at https://www.flickr.com/services/apps/create/

#include "password.h"


///////////////////////////////////////////////////////////////////////////////
// Flickr API Definitions

#define YAHOO_FLICKR_URL_ROOT          _T("api.flickr.com")
#define YAHOO_FLICKR_URL_STATIC        _T("static.flickr.com")
#define YAHOO_FLICKR_URL_UPLOAD        _T("up.flickr.com")

#define YAHOO_FLICKR_API_URLPATH       _T("/services/rest/")
#define YAHOO_FLICKR_AUTH_URLPATH      _T("/services/auth/")
#define YAHOO_FLICKR_UPLOAD_URLPATH    _T("/services/upload/")

#define BROWSER_USERAGENT              _T("Mozilla/5.0 (compatible; MSIE 11.0; Windows NT 6.0; FlickrDrive)")
#define FLICKR_PHOTO_EXTRAS            _T("date_taken,last_update")

#define REGPATH_FLICKRFS               _T("SOFTWARE\\viksoe.dk\\FlickrDrive")
#define REGPATH_FLICKRFS_SETTINGS      _T("\\Setting")
#define REGPATH_FLICKRFS_ACCOUNTS      _T("\\Accounts")
#define REGPATH_FLICKRFS_COLLECTIONS   _T("Collections")

#define FLICKRFS_ACCOUNTSTR_PUBLIC     _T("<public>")
#define FLICKRFS_ACCOUNTSTR_GUEST      _T("<guest>")

#define FLICKRFS_FOLDERSTR_INTERESTING _T("<interesting>")
#define FLICKRFS_FOLDERSTR_FAVOURITES  _T("<favourites>")
#define FLICKRFS_FOLDERSTR_NOTINSET    _T("<notinset>")
#define FLICKRFS_FOLDERSTR_HOTTAGS     _T("<hottags>")
#define FLICKRFS_FOLDERSTR_RECENT      _T("<recent>")

typedef struct tagFLICKR_APIARG
{
   LPCSTR pstrName;
   LPCTSTR pstrValue;
} FLICKR_APIARG;

typedef struct tagFLICKR_VOLUMEINFO
{
   ULONGLONG ullUploadMaxBytes;
   ULONGLONG ullUploadUsedBytes;
   ULONGLONG ullUploadRemainingBytes;
} FLICKR_VOLUMEINFO;

struct CAutoInternetHandle
{
   HINTERNET m_h;
   CAutoInternetHandle(HINTERNET h = NULL) : m_h(h) { };
   ~CAutoInternetHandle() { if( m_h != NULL ) ::WinHttpCloseHandle(m_h); };
   operator HINTERNET() const { return m_h; };
};

struct CAutoCryptContextHandle
{
   HCRYPTPROV m_h;
   CAutoCryptContextHandle(HCRYPTPROV h = NULL) : m_h(h) { };
   ~CAutoCryptContextHandle() { if( m_h != NULL ) ::CryptReleaseContext(m_h, 0UL); };
   operator HCRYPTPROV() const { return m_h; };
};

struct CAutoCryptHashHandle
{
   HCRYPTHASH m_h;
   CAutoCryptHashHandle(HCRYPTHASH h = NULL) : m_h(h) { };
   ~CAutoCryptHashHandle() { if( m_h != NULL ) ::CryptDestroyHash(m_h); };
   operator HCRYPTHASH() const { return m_h; };
};

struct TFlickrAccount;

typedef enum FLICKR_APITYPE { FLICKR_APITYPE_REST, FLICKR_APITYPE_UPLOAD, FLICKR_APITYPE_AUTH };

extern HRESULT DoDisplayErrorMessage(HWND hWnd, UINT nTitle, HRESULT Hr);


///////////////////////////////////////////////////////////////////////////////
// TFlickrPhoto

typedef struct TFlickrPhoto
{
   // Info
   CString sPhotoID;
   CString sSecret;
   CString sTitle;
   CString sKeywords;
   CString sDescription;
   CString sServerID;
   CString sFarmID;
   CString sOrigFormat;
   CString sOrigSecret;
   CString sFilename;
   CString sExtension;
   // Parent
   TFlickrAccount* pAccount;
   CString sOwnerUsername;
   CString sOwnerNSID;
   // Misc
   BOOL bIsPublic;
   BOOL bIsFriend;
   BOOL bIsFamily;
   BOOL bIsSpawned;
   BOOL bIsOwned;
   FILETIME ftDateTaken;
   FILETIME ftDatePosted;
   CString sLicenseID;
   CString sTagsRaw;
   CString sTags;
   // Exif
   CString sExifDimensions;
   CString sExifLongitude;
   CString sExifLattitude;
   CString sExifCameraMake;
   CString sExifCameraModel;
   CString sExifImageWidth;
   CString sExifImageHeight;
   CString sExifBitsPrSample;
   CString sExifXResolution;
   CString sExifYResolution;
} TFlickrPhoto;

typedef CSimpleValArray<TFlickrPhoto*> CFlickrPhotoList;
typedef CSimpleArray<CString> CFlickrPhotoIdList;


///////////////////////////////////////////////////////////////////////////////
// TFlickrImageset

typedef enum TFlickrImagesetType
{
   FLICKRFS_IMAGESET_UNKNOWN             = 0,
   FLICKRFS_IMAGESET_PHOTOSET            = 1,
   FLICKRFS_IMAGESET_TAGSEARCH           = 2,
   FLICKRFS_IMAGESET_FAVOURITES          = 3,
   FLICKRFS_IMAGESET_NOTINSET            = 4,
   FLICKRFS_IMAGESET_INTERESTING         = 5,
   FLICKRFS_IMAGESET_HOTTAGS             = 6,
   FLICKRFS_IMAGESET_RECENT              = 7,
   FLICKRFS_IMAGESET_GROUP               = 8,
   FLICKRFS_IMAGESET_PUBLIC_FAVOURITES   = 9,
};

typedef struct TFlickrImageset
{
   // Info
   CString sTitle;
   CString sFilename;
   CString sKeywords;
   CString sDescription;
   TFlickrImagesetType Type;
   // Photoset
   CString sImagesetID;
   CString sPrimaryID;
   CString sSecret;
   CString sServerID;
   CString sFarmID;
   long lPriority;
   // Parent
   TFlickrAccount* pAccount;
   // Photos
   CFlickrPhotoIdList aPhotoIds;
} TFlickrImageset;

typedef CSimpleValArray<TFlickrImageset*> CFlickrImagesetList;


///////////////////////////////////////////////////////////////////////////////
// TFlickrAccount

typedef enum TFlickrAccountType
{
   FLICKRFS_ACCOUNT_GUEST         = 0,
   FLICKRFS_ACCOUNT_OWNED         = 1,
   FLICKRFS_ACCOUNT_PUBLIC        = 2,
};

typedef struct TFlickrAccount
{
   CString sNSID;
   CString sUserName;
   CString sFullName;
   CString sLocation;
   CString sAuthToken;
   CString sFilename;
   TFlickrAccountType Access;
   BOOL bIsFriend;
   BOOL bIsFamily;
   BOOL bIsVirtual;
   BOOL bIsPrimary;
   BOOL bIsProAccount;
   // Sets
   CFlickrImagesetList aImagesets;
} TFlickrAccount;

typedef CSimpleValArray<TFlickrAccount*> CFlickrAccountList;


///////////////////////////////////////////////////////////////////////////////
// CFlickrAPI

class CFlickrAPI
{
public:
   CFlickrAccountList m_aAccounts;         // Account cache
   CFlickrPhotoList m_aPhotos;             // Photo cache
   CFlickrPhotoList m_aTempPhotos;         // Temporary photo cache (for uploaded files)
   CFlickrPhotoList m_aDeletedPhotos;      // Deleted photo cache (for garbage collection)
   CAutoInternetHandle m_hInternet;        // WinHTTP handle
   CAutoCryptContextHandle m_hCrypt;       // MS Crypto handle
   int m_aPhotoHashmap[501];               // Hashmap for photo cache
   FLICKR_VOLUMEINFO m_VolumeInfo;         // Upload/bandwidth information
   TFlickrAccount* m_pGuestAccount;        // Dummy account for system folders
   SYSTEMTIME m_stThisDay;                 // Last known date
   DWORD m_dwThisCount;                    // Count of queries today
   CComCriticalSection m_lock;             // Semaphore

   // Constructor / destructor

   CFlickrAPI();
   ~CFlickrAPI();

   // Operations

   HRESULT Init();

   HRESULT InitHttpService();

   HRESULT WelcomeAccount(HWND hWnd);
   HRESULT AddAccount(HWND hWnd);
   HRESULT AddAccount(LPCTSTR pstrUsername);
   HRESULT CheckAccount(LPCTSTR pstrUsername);
   HRESULT RemoveAccount(TFlickrAccount* pAccount);
   HRESULT GetAccountList(CFlickrAccountList& aList);
   HRESULT GetAccountFromPrimary(TFlickrAccount** ppAccount);
   HRESULT GetAccountFromNSID(LPCWSTR pstrNSID, TFlickrAccount** ppAccount);
   HRESULT GetAccountFromUsername(LPCWSTR pstrName, TFlickrAccount** ppAccount);
   HRESULT RefreshAccountList(TFlickrAccount* pAccount);

   HRESULT AddImageset(TFlickrAccount* pAccount, LPCWSTR pstrName, TFlickrImagesetType Type);
   HRESULT DeleteImageset(TFlickrImageset* pImageset);
   HRESULT AddPhotoToImageset(TFlickrImagesetType Type, TFlickrPhoto* pPhoto);
   HRESULT AddPhotoToImageset(TFlickrImageset* pImageset, TFlickrPhoto* pPhoto);
   HRESULT RemovePhotoFromImageset(TFlickrImageset* pImageset, TFlickrPhoto* pPhoto);
   HRESULT SetImagesetProperty(TFlickrImageset* pImageset, REFPROPERTYKEY pkey, const CComPropVariant& v);
   HRESULT FindImageset(TFlickrAccount* pAccount, LPCWSTR pstrName, TFlickrImageset** ppImageset);
   HRESULT GetImagesetList(TFlickrAccount* pAccount, CFlickrImagesetList& aList);
   HRESULT GetImagesetPhotoList(TFlickrImageset* pImageset, CFlickrPhotoList& aList);
   HRESULT RefreshImagesetPhotoList(TFlickrImageset* pImageset);
   HRESULT UpdateImagesetPhotoList(TFlickrImageset* pImageset);

   HRESULT DeletePhoto(TFlickrPhoto* pPhoto);
   HRESULT ReadPhotoExtendedInfo(TFlickrPhoto* pPhoto);
   HRESULT ReadPhotoExifInfo(TFlickrPhoto* pPhoto);
   HRESULT SetPhotoProperty(TFlickrPhoto* pPhoto, REFPROPERTYKEY pkey, const CComPropVariant& v);
   HRESULT AddPhotoProperty(TFlickrPhoto* pPhoto, REFPROPERTYKEY pkey, const CComPropVariant& v);
   HRESULT GetPhoto(LPCWSTR pstrPhotoID, TFlickrPhoto** ppPhoto);
   HRESULT GetPhotoUrls(TFlickrPhoto* pPhoto, CSimpleArray<CString>& aURLs);
   HRESULT GetPhotoThumbUrl(LPCWSTR pstrPhotoID, VFS_STREAMTYPE Type, int cxyThumb, CString& sURI);
   HRESULT GetPhotoUploadUrl(TFlickrAccount* pAccount, FLICKR_APIARG* pArgs, int nArgs, CString& sURI, CString& sSignature);
   HRESULT SpawnTempPhoto(const WIN32_FIND_DATA wfd);
   HRESULT PostProcessPhotoUpload(LPCTSTR pstrFilename, TFlickrImageset* pImageset, LPSTR pstrXmlResponse);
   HRESULT PostProcessPhotoUpload(TFlickrPhoto *pPhoto, TFlickrImageset* pImageset);

   HRESULT ReadUploadStatus();

   // Implementation

   HRESULT _InitAPI();
   HRESULT _InitCrypto();
   HRESULT _InitDummyAccount();
   HRESULT _RefreshAccountCache();
   HRESULT _RefreshEntirePhotoListCache();
   HRESULT _RefreshAccountPhotosCache(TFlickrAccount* pAccount);
   CString _ConstructFilename(LPCTSTR pstrDesired, LPCTSTR pstrFallback) const;

   HRESULT _ReadAccountList();
   HRESULT _ReadImagesetList(TFlickrAccount* pAccount);
   HRESULT _ReadImagesetPhotoList(TFlickrImageset* pImageset);
   HRESULT _ReadImagesetPhotoListResult(TFlickrImageset* pImageset, IUnknown* pUnk, LPCWSTR pstrPattern);

   // Account setup helpers

   HRESULT _DoAccountWebAuthentication(HWND hWnd, LPCTSTR pstrUsername, TFlickrAccountType AccountType, CString& sAuthToken);
   HRESULT _DoDefaultAccountSetup(LPCTSTR pstrUsername, TFlickrAccountType iAccountType, CString sAuthToken, BOOL bIsPrimary, BOOL bAddFriends, BOOL bAddFamily);
   HRESULT _CreateAccountFolder(HKEY hKeyCollection, LPCTSTR pszFolderRegName, UINT uDisplayName, TFlickrImagesetType ImagesetType);

   // Flickr REST API

   HRESULT _CheckQueryQuota();
   HRESULT _SubmitFlickrRestAction(FLICKR_APIARG* pArgs, int nArgs, IXMLDOMDocument** ppXmlDoc = NULL);
   CString _RestBuildActionUrl(FLICKR_APITYPE ApiType, FLICKR_APIARG* pArgs, int nArgs);
   HRESULT _RestSendRequest(LPCWSTR pstrHost, LPCTSTR pstrUrl, LPCTSTR pstrMethod, DWORD dwFlags, LPSTR pstrPost, DWORD cbPost, LPBYTE& pstrData, DWORD& cbData);
   HRESULT _RestExtractResult(LPCSTR pstrXML, IXMLDOMDocument** ppDoc);
   HRESULT _XmlGetResultNode(IUnknown* pUnk, LPCWSTR pstrPattern, CString& sValue);
   HRESULT _XmlGetResultList(IUnknown* pUnk, LPCWSTR pstrPattern, IXMLDOMNodeList** ppList, long* pCount);
   HRESULT _XmlGetResultList(IUnknown* pUnk, LPCWSTR pstrPattern, LPCWSTR pstrAttribute, CSimpleArray<CString>& aList);
   BOOL _MD5Hash(LPCSTR data, LPTSTR pstrResult);
   DWORD _UInt4Hash(LPCWSTR pstr) const;
};

