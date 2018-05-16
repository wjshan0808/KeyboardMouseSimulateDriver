
#include "stdafx.h"

#include "RegFileSystem.h"
#include "ShellFolder.h"


///////////////////////////////////////////////////////////////////////////////
// CRegItemValue

CRegItemValue::CRegItemValue(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem) :
	CNseBaseItem(pFolder, pidlFolder, pidlItem, bReleaseItem)
{
   // Extract item data
   m_pRegInfo = reinterpret_cast<const REGKEYPIDLINFO*>(pidlItem);
}

/**
 * Get the item type.
 */
BYTE CRegItemValue::GetType()
{
   return REG_MAGICID_VALUE;   
}


/**
* Get system icon index.
* This is slightly faster for the Shell than using GetExtractIcon().
* Return S_FALSE if no system index exists.
*/
HRESULT CRegItemValue::GetSysIcon(UINT uIconFlags, int* pIconIndex)
{
	// Use our SHGetFileSysIcon() method to get the System Icon index
	return ::SHGetFileSysIcon(m_pRegInfo->cPath, FILE_ATTRIBUTE_NORMAL, uIconFlags, pIconIndex);
}
HRESULT CRegItemValue::GetExtractIcon(REFIID riid, LPVOID* ppRetVal)
{
	return ::SHCreateFileExtractIcon(_T("image.jpg"), FILE_ATTRIBUTE_NORMAL, riid, ppRetVal);
}

/**
 * Return item information.
 * We support the properties for the columns as well as a number of
 * administrative information bits (such as what properties to display
 * in the Details panel).
 */
HRESULT CRegItemValue::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   if( pkey == PKEY_ItemName ) {
      return ::InitPropVariantFromString(m_pRegInfo->cName, &v);
   }
   if( pkey == PKEY_ParsingName ) {
      return ::InitPropVariantFromString(m_pRegInfo->cName, &v);
   }
   if( pkey == PKEY_ItemNameDisplay ) {
      return ::InitPropVariantFromString(m_pRegInfo->cName, &v);
   }
   if (pkey == PKEY_ItemType) {
	   return ::InitPropVariantFromString(PathFindExtension(m_pRegInfo->cName), &v);
   }
   if (pkey == PKEY_ItemTypeText) {
	   return ::InitPropVariantFromString(PathFindExtension(m_pRegInfo->cName), &v);
   }
   if (pkey == PKEY_FileName) {
	   return ::InitPropVariantFromString(m_pRegInfo->cName, &v);
   }
   return CNseBaseItem::GetProperty(pkey, v);
}

/**
 * Set a property on the item.
 */
HRESULT CRegItemValue::SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v)
{
   return CNseBaseItem::SetProperty(pkey, v);
}

/**
 * Return SFGAOF flags for this item.
 * These flags tells the Shell about the capabilities of this item. Here we
 * can toggle the ability to copy/delete/rename an item.
 */
SFGAOF CRegItemValue::GetSFGAOF(SFGAOF dwMask)
{
   // We support DELETE and RENAME of values, and not really a Property Sheet.
   // Unfortunately the Shell won't display writeable properties without the
   // sheet flag!?
   return SFGAO_CANDELETE 
			| SFGAO_CANRENAME
			| SFGAO_HASPROPSHEET
			| SFGAO_FILESYSTEM;
}

/**
 * Return file information.
 * We use this to return a simple structure with key information about
 * our item.
 */
VFS_FIND_DATA CRegItemValue::GetFindData()
{
   VFS_FIND_DATA wfd = { 0 };
   wcscpy_s(wfd.cFileName, lengthof(wfd.cFileName), m_pRegInfo->cName);

   wfd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL 
							| FILE_ATTRIBUTE_VIRTUAL;

   if (m_pRegInfo->bIsSpawned) {
	   wfd.cAlternateFileName[1] = VFS_HACK_SAVEAS_JUNCTION;
   }
   return wfd;
}

/**
 * Rename this item.
 */
HRESULT CRegItemValue::Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName)
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
HRESULT CRegItemValue::Delete()
{
	if (::DeleteFile(m_pRegInfo->cPath)) {
		return S_OK;
	}
	else {
		return AtlHresultFromLastError();
	}
}

// Static members

/**
 * Serialize item from static data.
 */
PCITEMID_CHILD CRegItemValue::GenerateITEMID(const WIN32_FIND_DATA& wfd)
{
   // Serialize data from a WIN23_FIND_DATA structure to a child PIDL.
   REGKEYPIDLINFO data = { 0 };
   data.magic = REG_MAGICID_VALUE;
   wcscpy_s(data.cName, lengthof(data.cName), wfd.cFileName);

   data.bIsSpawned = (wfd.cAlternateFileName[1] == VFS_HACK_SAVEAS_JUNCTION);

   // Hotfix of (default) item; which is internally known as '\0'
   CComBSTR bstrDefault;
   bstrDefault.LoadString(IDS_REGVAL_DEFAULT);
   if( bstrDefault == data.cName ) data.cName[0] = '\0';
   // Return serialized data
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

/**
 * Serialize item from static data.
 */
PCITEMID_CHILD CRegItemValue::GenerateITEMID(const REGKEYPIDLINFO& src)
{
   // Serialize data from our REGKEYPIDLINFO structure to a child PIDL.
   REGKEYPIDLINFO data = src;
   data.magic = REG_MAGICID_VALUE;
   return CNseBaseItem::GenerateITEMID(&data, sizeof(data));
}

HRESULT CRegItemValue::GetIdentity(LPWSTR pstrFilename)
{
	wcscpy_s(pstrFilename, MAX_PATH, _T("tmpfile2"));
	return S_OK;
}
