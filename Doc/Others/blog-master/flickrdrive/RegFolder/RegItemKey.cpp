
#include "stdafx.h"

#include "RegFileSystem.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CRegItemKey defines

///////////////////////////////////////////////////////////////////////////////
// CRegItemKey

CRegItemKey::CRegItemKey(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem) :
	CNseBaseItem(pFolder, pidlFolder, pidlItem, bReleaseItem)
{
   if( pidlItem == NULL ) 
   {
      // Create empty root item
      static REGKEYPIDLINFO s_Info = { 0 };
      s_Info.magic = REG_MAGICID_KEY;
      m_pRegInfo = &s_Info;
   }
   else 
   {
      // Extract item data
      m_pRegInfo = reinterpret_cast<const REGKEYPIDLINFO*>(pidlItem);
   }
}

/**
 * Get the item type.
 */
BYTE CRegItemKey::GetType()
{
   return REG_MAGICID_KEY;   
}

/**
 * Get system icon index.
 * This is slightly faster for the Shell than using GetExtractIcon().
 * Return S_FALSE if no system index exists.
 */
HRESULT CRegItemKey::GetSysIcon(UINT uIconFlags, int* pIconIndex)
{
   // Use our SHGetFileSysIcon() method to get the System Icon index
   return ::SHGetFileSysIcon(m_pRegInfo->cName, FILE_ATTRIBUTE_DIRECTORY, uIconFlags, pIconIndex);
}
HRESULT CRegItemKey::GetExtractIcon(REFIID riid, LPVOID* ppRetVal)
{
	return ::SHCreateFileExtractIcon(_T("Folder"), FILE_ATTRIBUTE_DIRECTORY, riid, ppRetVal);
}
/**
 * Get information about column definition.
 * We return details of a column requested, plus information about the 
 * combined set of properties supported by items contained in this folders. We 
 * decide which columns to show by default here too.
 */
HRESULT CRegItemKey::GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column)
{
   static VFS_COLUMNINFO aColumns[] = {
      { PKEY_ItemNameDisplay,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_DateModified,               SHCOLSTATE_TYPE_DATE | SHCOLSTATE_ONBYDEFAULT | SHCOLSTATE_SLOW,  0 },
      { PKEY_ItemPathDisplay,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_RegistryType,               SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_RegistryValueType,          SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_RegistryValue,              SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    VFS_COLF_WRITEABLE },
      { PKEY_FileAttributes,             SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ItemType,                   SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ItemTypeText,               SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ParsingPath,                SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ItemPathDisplayNarrow,      SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },      
      { PKEY_SFGAOFlags,                 SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_Volume_IsRoot,              SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_InfoTip,           SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_TileInfo,          SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_FullDetails,       SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_PreviewTitle,      SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_PreviewDetails,    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_ExtendedTileInfo,  SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
   };
   if( iColumn >= lengthof(aColumns) ) return E_FAIL;
   Column = aColumns[iColumn];
   return S_OK;
}

/**
 * Return item information.
 * We support the properties for the columns as well as a number of
 * administrative information bits (such as what properties to display
 * in the Details panel).
 */
HRESULT CRegItemKey::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   if( pkey == PKEY_ParsingName ) {
      return ::InitPropVariantFromString(m_pRegInfo->cName, &v);
   } 
   if( pkey == PKEY_ItemName ) {
      return ::InitPropVariantFromString(m_pRegInfo->cName, &v);
   }
   // For convenience we only display the name CURRENT_USER (not HKEY_CURRENT_USER) for
   // the root items. This also demonstrates how the parsing name can be different from
   // the display name.
   if( pkey == PKEY_ItemNameDisplay ) {
      return ::InitPropVariantFromString(m_pRegInfo->cName, &v);
   }
   // Return our custom Registry properties and standard time-related properties
   // for this item.
   if( pkey == PKEY_RegistryType ) {
      return ::InitPropVariantFromUInt32(0UL, &v);
   }
   if( pkey == PKEY_DateModified ) {
      const WIN32_FIND_DATA wfd = GetFindData();
      return ::InitPropVariantFromFileTime(&wfd.ftLastWriteTime, &v);
   }
   // Returning file-attributes allow keys to be sorted/displayed
   // before values in the list.
   if( pkey == PKEY_FileAttributes ) {
      return ::InitPropVariantFromUInt32(FILE_ATTRIBUTE_DIRECTORY, &v);
   }
   return CNseBaseItem::GetProperty(pkey, v);
}

/**
 * Return SFGAOF flags for this item.
 * These flags tells the Shell about the capabilities of this item. Here we
 * can toggle the ability to copy/delete/rename an item.
 */
SFGAOF CRegItemKey::GetSFGAOF(SFGAOF dwMask)
{
   return SFGAO_FOLDER
	   | SFGAO_CANCOPY
	   | SFGAO_BROWSABLE
	   | SFGAO_HASPROPSHEET
	   | SFGAO_FILESYSANCESTOR;
}

/**
 * Return file information.
 * We use this to return a simple structure with basic information about
 * our item.
 */
VFS_FIND_DATA CRegItemKey::GetFindData()
{
   VFS_FIND_DATA wfd = { 0 };
   wcscpy_s(wfd.cFileName, lengthof(wfd.cFileName), m_pRegInfo->cName);
   wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

   //wfd.ftLastWriteTime = m_pRegInfo->ftLastWrite;

   return wfd;
}

/**
 * Return the folder settings.
 * The Folder Settings structure contains optional values for the initial
 * display of the folder.
 */
VFS_FOLDERSETTINGS CRegItemKey::GetFolderSettings()
{
   VFS_FOLDERSETTINGS Settings = { 0 };
   // Control default view-mode for the 1st, 2nd and other tree levels.
   if( ::ILIsEmpty(m_pidlFolder) ) Settings.ViewMode = FLVM_ICONS, Settings.cxyIcon = 96;
   else if( ILIsChild(m_pidlFolder) ) Settings.ViewMode = FLVM_ICONS;
   else Settings.ViewMode = FLVM_DETAILS;
   return Settings;
}

/**
 * Return the menu for the item.
 */
HMENU CRegItemKey::GetMenu()
{
   return ::LoadMenu(_pModule->GetResourceInstance(), MAKEINTRESOURCE(IDM_FOLDER));
}

/**
 * Execute a menu command.
 */
HRESULT CRegItemKey::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   switch( Cmd.wMenuID ) {
   case DFM_CMD_NEWFOLDER:       return _DoNewKey(Cmd, IDS_UNNAMED);
   case ID_NEW_KEY:              return _DoNewKey(Cmd, IDS_UNNAMED);
   case ID_NEW_VALUE_DEFAULT:    return _DoNewValue(Cmd, IDS_EMPTY, REG_SZ);
   case ID_NEW_VALUE_SZ:         return _DoNewValue(Cmd, IDS_UNNAMED, REG_SZ);
   case ID_NEW_VALUE_DWORD:      return _DoNewValue(Cmd, IDS_UNNAMED, REG_DWORD);
   case ID_NEW_VALUE_QWORD:      return _DoNewValue(Cmd, IDS_UNNAMED, REG_QWORD);      
   case ID_NEW_VALUE_EXPAND_SZ:  return _DoNewValue(Cmd, IDS_UNNAMED, REG_EXPAND_SZ);
   }
   return E_NOTIMPL;
}

/**
 * Create an NSE Item instance from a child PIDL.
 */
CNseItem* CRegItemKey::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem)
{
   // Use a pidl-wrapper to validate the correctness of the PIDL and spawn
   // an NSE Item based on the type.
   CPidlMemPtr<REGKEYPIDLINFO> pItem = pidlItem;
   if( pItem.IsType(REG_MAGICID_KEY) ) 
	   return new CRegItemKey(pFolder, pidlFolder, pidlItem, bReleaseItem);
   if( pItem.IsType(REG_MAGICID_VALUE) ) 
	   return new CRegItemValue(pFolder, pidlFolder, pidlItem, bReleaseItem);
   return NULL;
}

/**
 * Create an NSE Item from static data.
 */
CNseItem* CRegItemKey::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd)
{
   if( IsBitSet(wfd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ) {
      return new CRegItemKey(pFolder, pidlFolder, CRegItemKey::GenerateITEMID(wfd), TRUE);
   }
   return new CRegItemValue(pFolder, pidlFolder, CRegItemValue::GenerateITEMID(wfd), TRUE);
}

/**
 * Look up a single child item (Registry key or value) by name.
 */
HRESULT CRegItemKey::GetChild(LPCWSTR pwstrName, SHGNO ParseType, CNseItem** pItem)
{
	WIN32_FIND_DATA wfd = { 0 };
	std::wstring sPath = m_pRegInfo->cPath;
	sPath += _T("\\");
	sPath += pwstrName;
	HANDLE hFind = FindFirstFile(sPath.c_str(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return AtlHresultFromLastError();
	}
	FindClose(hFind);
	REGKEYPIDLINFO data = { 0 };
	wcscpy_s(data.cName, lengthof(data.cName), wfd.cFileName);
	wcscpy_s(data.cPath, lengthof(data.cPath), sPath.c_str());
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		*pItem = new CRegItemKey(m_pFolder, m_pidlFolder, CRegItemKey::GenerateITEMID(data), TRUE);
	}
	else {
		*pItem = new CRegItemValue(m_pFolder, m_pidlFolder, CRegItemValue::GenerateITEMID(data), TRUE);
	}
	return *pItem != NULL ? S_OK : E_OUTOFMEMORY;
}

/**
 * Retrieve the list of children of the current folder item.
 */
HRESULT CRegItemKey::EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aItems)
{
	std::wstring sPath = IsRoot() ? _T("C:") : m_pRegInfo->cPath;
	std::wstring sPattern = sPath;
	sPattern += _T("\\");
	sPattern += _T("*");
	WIN32_FIND_DATA wfd = { 0 };
	HANDLE hFind = FindFirstFile(sPattern.c_str(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return E_HANDLE;
	}

	while (FindNextFile(hFind, &wfd)) {
		if (!wcscmp(wfd.cFileName, _T("."))) continue;
		if (!wcscmp(wfd.cFileName, _T(".."))) continue;
		
		if (SHFilterEnumItem(grfFlags, wfd) != S_OK) continue;

		REGKEYPIDLINFO data = { 0 };
		wcscpy_s(data.cName, lengthof(data.cName), wfd.cFileName);
		
		wcscpy_s(data.cPath, lengthof(data.cPath), sPath.c_str());
		wcscat_s(data.cPath, lengthof(data.cPath), _T("\\"));
		wcscat_s(data.cPath, lengthof(data.cPath), wfd.cFileName);
		
		data.bIsSpawned = FALSE;
		
		//data.ftLastWrite = wfd.ftLastWriteTime;

		if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			aItems.Add(new CRegItemKey(m_pFolder, m_pFolder->m_pidlFolder, CRegItemKey::GenerateITEMID(data), TRUE));
		}else
		{
			aItems.Add(new CRegItemValue(m_pFolder, m_pFolder->m_pidlFolder, CRegItemValue::GenerateITEMID(data), TRUE));
		}
	}
	::FindClose(hFind);
   return S_OK;
}

/**
 * Create a new sub-key.
 */
HRESULT CRegItemKey::CreateFolder()
{
   // Create this item as a new Registry key
	CreateDirectory(m_pRegInfo->cPath, NULL);
    return S_OK;
}

/**
 * Rename this item.
 */
HRESULT CRegItemKey::Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName)
{
	WCHAR szNewPath[MAX_PATH] = _T("");
	wcscpy_s(szNewPath, lengthof(szNewPath), m_pRegInfo->cPath);
	wcsrchr(szNewPath, _T('\\'))[1] = _T('\0');
	wcscat_s(szNewPath, pstrNewName);
	::MoveFile(m_pRegInfo->cPath, szNewPath);
	return S_OK;
}

/**
 * Delete this item.
 */
HRESULT CRegItemKey::Delete()
{
	SHFILEOPSTRUCT FileOp = { 0 };
	FileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
	FileOp.pFrom = m_pRegInfo->cPath;
	FileOp.pTo = NULL;
	FileOp.wFunc = FO_DELETE;
	return !SHFileOperation(&FileOp) ? S_OK : E_FAIL;
}

// Static members

/**
 * Serialize item from static data.
 * The Shell sometimes uses this method when it wants to create a brand
 * new folder.
 */
PCITEMID_CHILD CRegItemKey::GenerateITEMID(const WIN32_FIND_DATA& wfd)
{
   // Serialize data from a WIN32_FIND_DATA structure to a child PIDL.
   REGKEYPIDLINFO data = { 0 };
   data.magic = REG_MAGICID_KEY;
   wcscpy_s(data.cName, lengthof(data.cName), wfd.cFileName);
   data.ftLastWrite = wfd.ftLastWriteTime;
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

/**
 * Serialize item from static data.
 */
PCITEMID_CHILD CRegItemKey::GenerateITEMID(const REGKEYPIDLINFO& src)
{
   // Serialize data from our REGKEYPIDLINFO structure to a child PIDL.
   REGKEYPIDLINFO data = src;
   data.magic = REG_MAGICID_KEY;
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

// Implementation

/**
 * Is this the root level of the Registry branches?
 */
BOOL CRegItemKey::_IsTopLevel() const
{
   return ::ILIsEmpty(m_pidlFolder);
}

/**
 * Create a new key.
 * The method is called in response to the "New Folder" menu command.
 */
HRESULT CRegItemKey::_DoNewKey(const VFS_MENUCOMMAND& Cmd, UINT uLabelRes)
{
   CComBSTR bstrLabel;
   bstrLabel.LoadString(uLabelRes);

   std::wstring sPath = m_pRegInfo->cPath;
   sPath += _T("\\");
   sPath += bstrLabel;
   CreateDirectory(sPath.c_str(), NULL);

   // Go into edit-mode for new item
   HR( _AddSelectEdit(Cmd, bstrLabel) );
   return S_OK;
}

/**
 * Create a new value.
 * The method is called in response to one of the "New -> Value (XXX)" range
 * of menu commands.
 */
HRESULT CRegItemKey::_DoNewValue(const VFS_MENUCOMMAND& Cmd, UINT uLabelRes, DWORD dwType)
{
   CComBSTR bstrLabel;
   bstrLabel.LoadString(uLabelRes);

   std::wstring sPath = m_pRegInfo->cPath;
   sPath += _T("\\");
   sPath += bstrLabel;
   HANDLE hFile = CreateFile(sPath.c_str(), FILE_GENERIC_READ|FILE_GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile != INVALID_HANDLE_VALUE) {
	   CloseHandle(hFile);
   }
   else {
	   return AtlHresultFromLastError();
   }
   // Add item to view and begin rename if it's not a (default) value
   DWORD dwFlags = SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE | SVSI_SELECT;
   if( uLabelRes != IDS_EMPTY ) dwFlags |= SVSI_EDIT;
   HR( _AddSelectEdit(Cmd, bstrLabel, dwFlags) );
   return S_OK;
}

HRESULT CRegItemKey::GetIdentity(LPWSTR pstrFilename)
{
	wcscpy_s(pstrFilename, MAX_PATH, _T("c:\\Temp\\tmpfile1"));
	return S_OK;
}
