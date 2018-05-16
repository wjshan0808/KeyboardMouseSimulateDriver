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


///////////////////////////////////////////////////////////////////////////////
// CTransferSource

class ATL_NO_VTABLE CTransferSource :
   public CComObjectRootEx<CComSingleThreadModel>,
   public ITransferSource
{
public:
   CRefPtr<CShellFolder> m_spFolder;                      // Reference to folder
   CComDynamicUnkArray m_aSinks;                          // List of Advise sinks
   CInterfaceArray<IPropertyChangeArray> m_aPropChanges;  // List of Property Changes

   BEGIN_COM_MAP(CTransferSource)
      COM_INTERFACE_ENTRY(ITransferSource)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder);
   void FinalRelease();

   // ITransferSource

   STDMETHOD(Advise)(ITransferAdviseSink* psink, DWORD* pdwCookie);        
   STDMETHOD(Unadvise)(DWORD dwCookie);        
   STDMETHOD(SetProperties)(IPropertyChangeArray* pproparray);        
   STDMETHOD(OpenItem)(IShellItem* psi, DWORD dwFlags, REFIID riid, LPVOID* ppv);
   STDMETHOD(MoveItem)(IShellItem* psi, IShellItem* psiParentDst, LPCWSTR pszNameDst, DWORD dwFlags, IShellItem** ppsiNew);        
   STDMETHOD(RecycleItem)(IShellItem* psiSource, IShellItem* psiParentDest, DWORD dwFlags, IShellItem** ppsiNewDest);        
   STDMETHOD(RemoveItem)(IShellItem* psiSource, DWORD dwFlags);        
   STDMETHOD(RenameItem)(IShellItem* psiSource, LPCWSTR pszNewName, DWORD dwFlags, IShellItem** ppsiNewDest);        
   STDMETHOD(LinkItem)(IShellItem* psiSource, IShellItem* psiParentDest, LPCWSTR pszNewName, DWORD dwLinkFlags, IShellItem** ppsiNewDest);
   STDMETHOD(ApplyPropertiesToItem)(IShellItem* psiSource, IShellItem** ppsiNew);        
   STDMETHOD(GetDefaultDestinationName)(IShellItem* psiSource, IShellItem* psiParentDest, LPWSTR* ppszDestinationName);       
   STDMETHOD(EnterFolder)(IShellItem* psiChildFolderDest);        
   STDMETHOD(LeaveFolder)(IShellItem* psiChildFolderDest);

   // Implementation

   HRESULT _UpdateTransferState(TRANSFER_ADVISE_STATE State);
   HRESULT _UpdateTransferProgress(ULONGLONG ullPos, ULONGLONG ullSize);
   HRESULT _ReportPropertyError(IShellItem* pShellItem, REFPROPERTYKEY pkey, HRESULT Hr);
   HRESULT _SendShellNotify(LONG wEventId, DWORD dwFlags, CNseItem* pItem1, CNseItem* pItem2 = NULL);
};


///////////////////////////////////////////////////////////////////////////////
// CTransferDestination

class ATL_NO_VTABLE CTransferDestination :
   public CComObjectRootEx<CComSingleThreadModel>,
   public ITransferDestination,
   public IServiceProvider
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CComDynamicUnkArray m_aSinks;

   BEGIN_COM_MAP(CTransferDestination)
      COM_INTERFACE_ENTRY(ITransferDestination)
      COM_INTERFACE_ENTRY(IServiceProvider)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder);

   // ITransferDestination

   STDMETHOD(Advise)(ITransferAdviseSink* psink, DWORD* pdwCookie);
   STDMETHOD(Unadvise)(DWORD dwCookie);
   STDMETHOD(CreateItem)(LPCWSTR pszName, DWORD dwAttributes, ULONGLONG ullSize, DWORD dwFlags, REFIID riidItem, LPVOID* ppvItem, REFIID riidResources, LPVOID* ppvResources);

   // IServiceProvider

   STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, LPVOID* ppv);

   // Implementation

   HRESULT _SendShellNotify(LONG wEventId, DWORD dwFlags, CNseItem* pItem1, CNseItem* pItem2 = NULL);
};


///////////////////////////////////////////////////////////////////////////////
// CTransferMediumItem

class ATL_NO_VTABLE CTransferMediumItem :
   public CComObjectRootEx<CComSingleThreadModel>,
   public ITransferMediumItem
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CPidl m_pidl;

   BEGIN_COM_MAP(CTransferMediumItem)
      COM_INTERFACE_ENTRY(ITransferMediumItem)
      COM_INTERFACE_ENTRY(IRelatedItem)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, PCUITEMID_CHILD pidl);

   // ITransferMediumItem

   STDMETHOD(GetItemIDList)(PIDLIST_ABSOLUTE* ppidl);
   STDMETHOD(GetItem)(IShellItem** ppsi);
};


///////////////////////////////////////////////////////////////////////////////
// CShellItemResources

class ATL_NO_VTABLE CShellItemResources :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IShellItemResources
{
public:
   CRefPtr<CShellFolder> m_spFolder;
   CNseItemPtr m_spItem;

   BEGIN_COM_MAP(CShellItemResources)
      COM_INTERFACE_ENTRY(IShellItemResources)      
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, const WIN32_FIND_DATA wfd);
   HRESULT Init(CShellFolder* pFolder, PCUITEMID_CHILD pidlChild);

   // IShellItemResources

   STDMETHOD(GetAttributes)(DWORD* pdwAttributes);
   STDMETHOD(GetSize)(ULONGLONG* pullSize);
   STDMETHOD(GetTimes)(FILETIME* pftCreation, FILETIME* pftWrite, FILETIME* pftAccess);
   STDMETHOD(SetTimes)(const FILETIME* pftCreation, const FILETIME* pftWrite, const FILETIME* pftAccess);
   STDMETHOD(GetResourceDescription)(const SHELL_ITEM_RESOURCE* pcsir, LPWSTR* ppszDescription);
   STDMETHOD(EnumResources)(IEnumResources** ppenumr);
   STDMETHOD(SupportsResource)(const SHELL_ITEM_RESOURCE* pcsir);
   STDMETHOD(OpenResource)(const SHELL_ITEM_RESOURCE* pcsir, REFIID riid, LPVOID* ppv);
   STDMETHOD(CreateResource)(const SHELL_ITEM_RESOURCE* pcsir, REFIID riid, LPVOID* ppv);
   STDMETHOD(MarkForDelete)(VOID);
};

