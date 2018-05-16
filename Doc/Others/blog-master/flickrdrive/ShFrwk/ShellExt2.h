/////////////////////////////////////////////////////////////////////////////
// Shell Extension wrappers
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


///////////////////////////////////////////////////////////////////////////////
// Undocumented Shell stuff

#define SFVM_SELECTIONCHANGED          8   /* undocumented */
#define SFVM_DRAWMENUITEM              9   /* undocumented */
#define SFVM_MEASUREMENUITEM          10   /* undocumented */
#define SFVM_EXITMENULOOP             11   /* undocumented */
#define SFVM_VIEWRELEASE              12   /* undocumented */
#define SFVM_GETNAMELENGTH            13   /* undocumented */
#define SFVM_WINDOWCLOSING            16   /* undocumented */
#define SFVM_LISTREFRESHED            17   /* undocumented */
#define SFVM_WINDOWFOCUSED            18   /* undocumented */
#define SFVM_REGISTERCOPYHOOK         20   /* undocumented */
#define SFVM_COPYHOOKCALLBACK         21   /* undocumented */
#define SFVM_ADDINGOBJECT             29   /* undocumented */
#define SFVM_REMOVINGOBJECT           30   /* undocumented */
#define SFVM_GETCOMMANDDIR            33   /* undocumented */
#define SFVM_GETCOLUMNSTREAM          34   /* undocumented */
#define SFVM_CANSELECTALL             35   /* undocumented */
#define SFVM_ISSTRICTREFRESH          37   /* undocumented */
#define SFVM_ISCHILDOBJECT            38   /* undocumented */
#define SFVM_GETEXTVIEWS              40   /* undocumented */
#define SFVM_DELAYWINDOWCREATE        74   /* undocumented */
#define SFVM_FORCEWEBVIEW             75   /* undocumented */
#define SFVM_GETCUSTOMVIEWINFO        77   /* undocumented */
#define SFVM_ENUMERATEDITEMS          79   /* undocumented */
#define SFVM_GETVIEWDATA              80   /* undocumented */
#define SFVM_GETWEBVIEWLAYOUT         82   /* undocumented */
#define SFVM_GETWEBVIEWCONTENT        83   /* undocumented */
#define SFVM_GETWEBVIEWTASKS          84   /* undocumented */
#define SFVM_GETWEBVIEWTHEME          86   /* undocumented */
#define SFVM_SORTLISTDATA             87   /* undocumented */
#define SFVM_SETEMPTYTEXT             91   /* undocumented */
#define SFVM_GETDEFERREDVIEWSETTINGS  92   /* undocumented */
#define SFVM_GETFOLDERTYPE            94   /* undocumented */
#define SFVM_SETFOLDERTYPE            95   /* undocumented */
#define SFVM_QUERYCREATEVIEWWINDOW    100  /* undocumented */
#define SFVM_SETNOTIFICATIONWINDOW    106  /* undocumented */

// SHShellFolderView_Message messages
#define SFVM_GETARRANGECOLUMN     0x0002   /* undocumented */
#define SFVM_GETITEMCOUNT         0x0004   /* undocumented */
#define SFVM_GETITEMPIDL          0x0005   /* undocumented */
#define SFVM_SETREDRAW            0x0008   /* undocumented */
#define SFVM_ISDROPONSOURCE       0x000A   /* undocumented */
#define SFVM_MOVEICONS            0x000B   /* undocumented */
#define SFVM_GETDRAGPOINT         0x000C   /* undocumented */
#define SFVM_GETDROPPOINT         0x000D   /* undocumented */
#define SFVM_ISDROPONBACKGROUND   0x000F   /* undocumented */
#define SFVM_TOGGLEAUTOARRANGE    0x0011   /* undocumented */
#define SFVM_LINEUPICONS          0x0012   /* undocumented */
#define SFVM_GETAUTOARRANGE       0x0013   /* undocumented */
#define SFVM_GETSELECTEDCOUNT     0x0014   /* undocumented */
#define SFVM_GETITEMSPACING       0x0015   /* undocumented */
#define SFVM_REFRESHOBJECT        0x0016   /* undocumented */

// Shell verb strings
#define CMDSTR_COPYA         "copy"
#define CMDSTR_PASTEA        "paste"
#define CMDSTR_DELETEA       "delete"
#define CMDSTR_RENAMEA       "rename"
#define CMDSTR_PROPERTIESA   "properties"


//////////////////////////////////////////////////////////////////////////////
// SHGNO vs SHGDNF

#ifdef _WIN32_WINNT_WIN7

// The Windows SDK for Win7 redefines SHGNO to SHGDNF; let's redefine it back
typedef SHGDNF SHGNO;

#endif // _WIN32_WINNT_WIN7

//////////////////////////////////////////////////////////////////////////////
// IShellFolderViewType

MIDL_INTERFACE("49422C1E-1C03-11D2-8DAB-0000F87A556C")
IShellFolderViewType : public IUnknown
{
   STDMETHOD(EnumViews)(ULONG grfFlags, IEnumIDList** ppenum) PURE;
   STDMETHOD(GetDefaultViewName)(DWORD  uFlags, LPWSTR* ppwszName) PURE;
   STDMETHOD(GetViewTypeProperties)(LPCITEMIDLIST pidl, DWORD* pdwFlags)  PURE;
   STDMETHOD(TranslateViewPidl)(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlView, LPCITEMIDLIST* ppidlOut) PURE;
};

#define SFVTFLAG_NOTIFY_CREATE  0x00000001
#define SFVTFLAG_NOTIFY_RESORT  0x00000002


//////////////////////////////////////////////////////////////////////////////
// IShellFolderSearchable

MIDL_INTERFACE("4E1AE66C-204B-11D2-8DB3-0000F87A556C")
IShellFolderSearchable : public IUnknown
{
   STDMETHOD(FindString)(LPCWSTR pwszTarget, DWORD *pdwFlags, IUnknown *punkOnAsyncSearch, LPITEMIDLIST *ppidlOut) PURE;
   STDMETHOD(CancelAsyncSearch)(LPCITEMIDLIST pidlSearch, DWORD *pdwFlags) PURE;
   STDMETHOD(InvalidateSearch)(LPCITEMIDLIST pidlSearch, DWORD *pdwFlags) PURE;
};

MIDL_INTERFACE("F98D8294-2BBC-11d2-8DBD-0000F87A556C")
IShellFolderSearchableCallback : public IUnknown
{
   STDMETHOD(RunBegin)(DWORD dwReserved) PURE;
   STDMETHOD(RunEnd)(DWORD dwReserved) PURE;
};


//////////////////////////////////////////////////////////////////////////////
// IInitializeFolderInfoTip

MIDL_INTERFACE("CAFEC873-94B2-47A4-AA4A-6A54F2DF865D")
IInitializeFolderInfoTip : public IUnknown
{
   STDMETHOD(Initialize)(IUnknown* innerQueryInfo, IUnknown *punkToWalk) PURE;
};


//////////////////////////////////////////////////////////////////////////////
// IObjectWithBackReferences

#ifndef __IObjectWithBackReferences_INTERFACE_DEFINED__
#define __IObjectWithBackReferences_INTERFACE_DEFINED__

MIDL_INTERFACE("321A6A6A-D61F-4BF3-97AE-14BE2986BB36")
IObjectWithBackReferences : public IUnknown
{
   STDMETHOD(RemoveBackReferences)() PURE;
};

#endif // __IObjectWithBackReferences_INTERFACE_DEFINED__

