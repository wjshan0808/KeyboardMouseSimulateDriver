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
// CExplorerCommandProvider

class ATL_NO_VTABLE CExplorerCommandProvider : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public IExplorerCommandProvider
{
public:
   CRefPtr<CShellFolder> m_spFolder;   // Reference to folder
   HWND m_hWnd;                        // Reference to owner window
   HMENU m_hMenu;                      // Menu that holds the explorer commands

   CExplorerCommandProvider();
   void FinalRelease();

   BEGIN_COM_MAP(CExplorerCommandProvider)
      COM_INTERFACE_ENTRY(IExplorerCommandProvider)
   END_COM_MAP()

   HRESULT Init(CShellFolder* pFolder, HWND hWnd, HMENU hMenu);

   // IExplorerCommandProvider

   STDMETHOD(GetCommands)(IUnknown* punkSite, REFIID riid, LPVOID* ppv);        
   STDMETHOD(GetCommand)(REFGUID rguidCommandId, REFIID riid, LPVOID* ppv);

   // Implementation

   HRESULT _GetCommand(HMENU hMenu, UINT uMenuID, REFIID riid, LPVOID* ppRetVal);
};


///////////////////////////////////////////////////////////////////////////////
// CExplorerCommand

class ATL_NO_VTABLE CExplorerCommand : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public IObjectWithSiteImpl<CExplorerCommand>,
   public IExplorerCommand
{
public:
   CRefPtr<CExplorerCommandProvider> m_spProvider;  // Reference to command provider
   WCHAR m_wszTitle[100];                           // Title of comamnd (MENUITEMINFO buffer)
   MENUITEMINFO m_mii;                              // Information about command

   BEGIN_COM_MAP(CExplorerCommand)
      COM_INTERFACE_ENTRY(IExplorerCommand)
      COM_INTERFACE_ENTRY(IObjectWithSite)
   END_COM_MAP()

   HRESULT Init(CExplorerCommandProvider* pProvider, const MENUITEMINFO& mii);

   // IExplorerCommand

   STDMETHOD(GetTitle)(IShellItemArray* psiItemArray, LPWSTR* ppszName);        
   STDMETHOD(GetIcon)(IShellItemArray* psiItemArray, LPWSTR* ppszIcon);        
   STDMETHOD(GetToolTip)(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip);        
   STDMETHOD(GetCanonicalName)(GUID* pguidCommandName);        
   STDMETHOD(GetState)(IShellItemArray* psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState);        
   STDMETHOD(Invoke)(IShellItemArray* psiItemArray, IBindCtx* pbc);        
   STDMETHOD(GetFlags)(EXPCMDFLAGS* pFlags);        
   STDMETHOD(EnumSubCommands)(IEnumExplorerCommand* *ppEnum);

   // Static members

   static GUID GetCanonicalName(UINT wID);
   static UINT GetMenuID(GUID guid);
};

