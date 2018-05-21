#include "stdafx.h"
#include "KeyboardMouseSimulateDriver.h" 
#include "KeyboardMouseSimulateDriverDefines.h"
#include "ServiceControlManager.h"

#include <cwchar>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstddef>

#include <conio.h>
#include <Windows.h>


//https://wiki.osdev.org/%228042%22_PS/2_Controller
//https://wiki.osdev.org/PS/2_Mouse
//https://wiki.osdev.org/PS/2_Keyboard
//// 对 8042 键盘控制芯片进行编程    
//// 允许 鼠标 接口   
//write_to_port(0x64, 0xa8);
//// 通知 8042 下个字节的发向 0x60 的数据将发给 鼠标  
//write_to_port(0x64, 0xd4);
//// 允许 鼠标 发数据  
//write_to_port(0x60, 0xf4);
//// 通知 8042,下个字节的发向 0x60 的数据应放向 8042 的命令寄存器  
//write_to_port(0x64, 0x60);
//// 许可键盘及 鼠标 接口及中断  
//write_to_port(0x60, 0x47);


//
typedef BOOL(WINAPI *PFN_IsWow64Process)(HANDLE hProcess, PBOOL bIsWow64Process);
typedef UINT(WINAPI *PFN_GetSystemWow64Directory)(LPTSTR szBuffer, UINT nBufferSize);

//
#define KEYBOARD_CMD     0x64
#define KEYBOARD_DATA    0x60
#define MOUSE_META_DATA  0x08


int g_nDriverType = 0;    //驱动加载类型
bool g_bIs64Bits = false;
bool g_bMouseWheel = false;
unsigned int g_nMouseMetaData = MOUSE_META_DATA;
wchar_t g_szDriverId[MAX_PATH] = { 0 };



#pragma region Utilities

bool _stdcall Is64Bits()
{
#ifdef _WIN64

  return true;

#else

  HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
  if (NULL == hKernel32)
  {
    // This shouldn't happen, but if we can't get kernel32's module handle then assume we are on x86.
    // We won't ever install 32-bit drivers on 64-bit machines,  
    // we just want to catch it up front to give users a better error message. 
    return false;
  }


  //PFN_IsWow64Process pfnIsWow64Process = (PFN_IsWow64Process)GetProcAddress(hKernel32, "IsWow64Process");
  //if (NULL == pfnIsWow64Process)
  //  return false;

  //BOOL bIsWow64Process = FALSE;
  //if (pfnIsWow64Process(GetCurrentProcess(), &bIsWow64Process))
  //  return true;
  //return false;


  PFN_GetSystemWow64Directory pfnGetSystemWow64Directory =
    (PFN_GetSystemWow64Directory)GetProcAddress(hKernel32, "GetSystemWow64DirectoryW"); // IsWow64Process
  if (NULL == pfnGetSystemWow64Directory)
  {
    // This most likely means we are running on Windows 2000,  
    // which didn't have this API and didn't have a 64-bit counterpart. 
    return false;
  }

  TCHAR szSystemWow64Directory[32767] = { 0 }; // 32767
  if (0 == pfnGetSystemWow64Directory(szSystemWow64Directory, _countof(szSystemWow64Directory)))
  {
    if (ERROR_CALL_NOT_IMPLEMENTED == GetLastError())
      return false;
  }
  // GetSystemWow64Directory succeeded  so we are on a 64-bit OS. 
  return true;

#endif
}

short _stdcall KeyStatus(unsigned int nKey)
{
  //https://msdn.microsoft.com/zh-cn/library/ms646301.aspx
  return GetKeyState(nKey);
}

/*
 *  bGetOrSet : True->Get, False->Set
 */
bool _stdcall CursorPosition(POINT &stPosition, bool bGetOrSet)
{
  //Set mouse speed, see https://msdn.microsoft.com/en-us/library/ms724947(v=vs.85).aspx
  BOOL bResult = FALSE;

  if (bGetOrSet)
  {
    //https://msdn.microsoft.com/en-us/library/windows/desktop/ms648390(v=vs.85).aspx 
    bResult = GetCursorPos(&stPosition);
  }
  else
  {
    //https://msdn.microsoft.com/en-us/library/windows/desktop/ms648394(v=vs.85).aspx
    bResult = SetCursorPos(stPosition.x, stPosition.y);
  }

  return bResult == TRUE ? true : false;
}

long long _stdcall Checkout()
{
  return (std::time(nullptr) + 60 * 60 * 8);
}

#pragma endregion


#pragma region Driver

HANDLE g_hDriver = INVALID_HANDLE_VALUE;

BOOL _stdcall ReadPortValue(HANDLE pHandle, WORD nAddress, PDWORD pValue, BYTE nSize = MAPVK_VSC_TO_VK)
{
  DWORD nReturned = 0;

  if (TYPE_DRIVER_WINIO == g_nDriverType)
  {
    WinIoPort stPort;
    stPort.m_nPortSize = nSize;
    stPort.m_nPortAddress = nAddress;

#ifdef _WIN64

    return DeviceIoControl(pHandle, IOCTL_WINIO_READPORT, &stPort, sizeof(WinIoPort), pValue, sizeof(DWORD), &nReturned, NULL);

#elif _WIN32

    // If this is a 64 bit OS, we must use the driver to access I/O ports 
    // even if the application is 32 bit
    if (!g_bIs64Bits)
    {
      switch (nSize)
      {
      case 1:
        *pValue = _inp(nAddress);
        break;
      case 2:
        *pValue = _inpw(nAddress);
        break;
      case 4:
        *pValue = _inpd(nAddress);
        break;
      }
      return true;
    }
    return DeviceIoControl(pHandle, IOCTL_WINIO_READPORT, &stPort, sizeof(WinIoPort), pValue, sizeof(DWORD), &nReturned, NULL);

#endif
  }
  else if (TYPE_DRIVER_WINRING0 == g_nDriverType)
  {
    WORD nValue = 0;
    BOOL nResult = DeviceIoControl(pHandle, IOCTL_OLS_READ_IO_PORT_BYTE, &nAddress, sizeof(nAddress), &nValue, sizeof(nValue), &nReturned, NULL);

    *pValue = nValue;

    return nResult;
  }
  else
    return false;
}

BOOL _stdcall WritePortValue(HANDLE pHandle, WORD nAddress, DWORD nValue, BYTE nSize = MAPVK_VSC_TO_VK)
{
  DWORD nReturned = 0;

  if (TYPE_DRIVER_WINIO == g_nDriverType)
  {
    WinIoPort stPort;
    stPort.m_nPortSize = nSize;
    stPort.m_nPortValue = nValue;
    stPort.m_nPortAddress = nAddress;

#ifdef _WIN64

    return DeviceIoControl(pHandle, IOCTL_WINIO_WRITEPORT, &stPort, sizeof(WinIoPort), NULL, 0, &nReturned, NULL);

#elif _WIN32

    // If this is a 64 bit OS, we must use the driver to access I/O ports 
    // even if the application is 32 bit
    if (!g_bIs64Bits)
    {
      switch (nSize)
      {
      case 1:
        _outp(nAddress, nValue);
        break;
      case 2:
        _outpw(nAddress, (WORD)nValue);
        break;
      case 4:
        _outpd(nAddress, nValue);
        break;
      }
      return true;
    }
    return DeviceIoControl(pHandle, IOCTL_WINIO_WRITEPORT, &stPort, sizeof(WinIoPort), NULL, 0, &nReturned, NULL);

#endif
  }
  else if (TYPE_DRIVER_WINRING0 == g_nDriverType)
  {
    WinRing0Port stPort;
    stPort.m_nPort = nAddress;
    stPort.m_nPortSize = (unsigned char)nValue;

    DWORD nLength = offsetof(WinRing0Port, m_nPortSize) + sizeof(stPort.m_nPortSize);
    return DeviceIoControl(pHandle, IOCTL_OLS_WRITE_IO_PORT_BYTE, &stPort, nLength, NULL, 0, &nReturned, NULL);
  }
  else
    return false;
}

void _stdcall KBCWait4IBE(HANDLE pHandle)
{
  DWORD nValue = 0;
  do
  {
    ReadPortValue(pHandle, KEYBOARD_CMD, &nValue);
  } while (nValue & 0x02);
}

void _stdcall KBCWait4OBF(HANDLE pHandle)
{
  DWORD nValue = 0;
  do
  {
    ReadPortValue(pHandle, KEYBOARD_CMD, &nValue);
  } while ((nValue & 0x20) && (nValue & 0x01));
}


void _stdcall Uninitialize()
{
  if (TYPE_DRIVER_EVENT != g_nDriverType && INVALID_HANDLE_VALUE != g_hDriver)
  {
    // Disable I/O port access if running on a 32 bit OS
    if (TYPE_DRIVER_WINIO == g_nDriverType)
    {
      if (!g_bIs64Bits)
      {
        DWORD nBytesReturned;
        DeviceIoControl(g_hDriver, IOCTL_WINIO_DISABLEDIRECTIO, NULL, 0, NULL, 0, &nBytesReturned, NULL);
      }
    }
    else if (TYPE_DRIVER_WINRING0 == g_nDriverType)
    {
      DWORD nLength, nRefCount = 0;
      DeviceIoControl(g_hDriver, IOCTL_OLS_GET_REFCOUNT, NULL, 0, &nRefCount, sizeof(nRefCount), &nLength, NULL);
    }

    CloseHandle(g_hDriver);
  }

  CServiceControlManager::Stop(g_szDriverId);
  CServiceControlManager::Delete(g_szDriverId);

  g_hDriver = INVALID_HANDLE_VALUE;
}

bool _stdcall KeyDown(unsigned int nKey)
{
  BOOL bResult = true;
  //Keydown 
  //https://msdn.microsoft.com/en-us/library/ms646306(VS.85).aspx
  unsigned int nMapVirtualKey = MapVirtualKey(nKey, MAPVK_VK_TO_VSC);

  if (TYPE_DRIVER_EVENT == g_nDriverType)
  {
    //https://msdn.microsoft.com/en-us/library/ms646304(VS.85).aspx
    keybd_event((unsigned char)nKey, (unsigned char)nMapVirtualKey, 0, 0);
  }
  else
  {
    KBCWait4IBE(g_hDriver);
    //0xD2准备写数据到Output Register中
    bResult &= WritePortValue(g_hDriver, KEYBOARD_CMD, 0xD2);

    KBCWait4IBE(g_hDriver);
    //0x60将写入到Input Register的字节放入到Output Register中，
    bResult &= WritePortValue(g_hDriver, KEYBOARD_DATA, 0xE2); // 0x60); //

    KBCWait4IBE(g_hDriver);
    bResult &= WritePortValue(g_hDriver, KEYBOARD_CMD, 0xD2);

    KBCWait4IBE(g_hDriver);
    bResult &= WritePortValue(g_hDriver, KEYBOARD_DATA, nMapVirtualKey);
  }

  return bResult ? true : false;
}

bool _stdcall KeyUp(unsigned int nKey)
{
  BOOL bResult = true;
  //Keyup
  //https://msdn.microsoft.com/en-us/library/ms646306(VS.85).aspx
  unsigned int nMapVirtualKey = MapVirtualKey(nKey, MAPVK_VK_TO_VSC);

  if (TYPE_DRIVER_EVENT == g_nDriverType)
  {
    //https://msdn.microsoft.com/en-us/library/ms646304(VS.85).aspx
    keybd_event((unsigned char)nKey, (unsigned char)nMapVirtualKey, KEYEVENTF_KEYUP, 0);
  }
  else
  {
    KBCWait4IBE(g_hDriver);
    bResult &= WritePortValue(g_hDriver, KEYBOARD_CMD, 0xD2);

    KBCWait4IBE(g_hDriver);
    bResult &= WritePortValue(g_hDriver, KEYBOARD_DATA, 0xE0); // 0x60); //

    KBCWait4IBE(g_hDriver);
    bResult &= WritePortValue(g_hDriver, KEYBOARD_CMD, 0xD2);

    KBCWait4IBE(g_hDriver);
    bResult &= WritePortValue(g_hDriver, KEYBOARD_DATA, nMapVirtualKey | 0x80);
  }

  return bResult ? true : false;
}


bool _stdcall MouseWheel()
{
  return (2 < GetSystemMetrics(SM_CMOUSEBUTTONS)) && (0 != GetSystemMetrics(SM_MOUSEWHEELPRESENT));
}

bool _stdcall MouseControl(unsigned int nButton, unsigned int nX, unsigned int nY)
{
  //unsigned int nMouseMetaData = 0x08;

  nX = (std::abs((int)nX) & 0xFF);
  nY = (std::abs((int)nY) & 0xFF);


  if (nX < 0)
  {
    g_nMouseMetaData |= 0x10;
    nX = (~nX + 1);//补码(取反+1)
  }
  else
  {
    g_nMouseMetaData &= ~0x10;
  }

  if (nY <= 0)
  {
    g_nMouseMetaData &= ~0x20;
  }
  else
  {
    g_nMouseMetaData |= 0x20;
    nY = (~nY + 1);//补码(取反+1)
  }

  //  Move = 0x0001,
  //  LeftDown = 0x0002,
  //  LeftUp = 0x0004,
  //  RightDown = 0x0008,
  //  RightUp = 0x0010,
  //  MiddleDown = 0x0020,
  //  MiddleUp = 0x0040,
  //  XDown = 0x0080,
  //  XUp = 0x0100,
  //  Wheel = 0x0800,
  //  VirtualDesk = 0x4000,
  //  Absolute = 0x8000
  switch (nButton)
  {
  case 0x0002:
    g_nMouseMetaData |= 0x01;
    break;
  case 0x0004:
    g_nMouseMetaData &= ~0x01;
    break;
  case 0x0008:
    g_nMouseMetaData |= 0x02;
    break;
  case 0x0010:
    g_nMouseMetaData &= ~0x02;
    break;
  case 0x0020:
    g_nMouseMetaData |= 0x04;
    break;
  case 0x0040:
    g_nMouseMetaData &= ~0x04;
    break;
  }

  KBCWait4OBF(g_hDriver);

  KBCWait4IBE(g_hDriver);
  WritePortValue(g_hDriver, KEYBOARD_CMD, 0xD3);
  KBCWait4IBE(g_hDriver);
  WritePortValue(g_hDriver, KEYBOARD_DATA, g_nMouseMetaData); // 0x09

  KBCWait4OBF(g_hDriver);

  KBCWait4IBE(g_hDriver);
  WritePortValue(g_hDriver, KEYBOARD_CMD, 0xD3);
  KBCWait4IBE(g_hDriver);
  WritePortValue(g_hDriver, KEYBOARD_DATA, nX);

  KBCWait4OBF(g_hDriver);

  KBCWait4IBE(g_hDriver);
  WritePortValue(g_hDriver, KEYBOARD_CMD, 0xD3);
  KBCWait4IBE(g_hDriver);
  WritePortValue(g_hDriver, KEYBOARD_DATA, nY);

  KBCWait4OBF(g_hDriver);

  if (MouseWheel())
  {
    KBCWait4IBE(g_hDriver);
    WritePortValue(g_hDriver, KEYBOARD_CMD, 0xD3);
    KBCWait4IBE(g_hDriver);
    WritePortValue(g_hDriver, KEYBOARD_DATA, 0x00);

    KBCWait4OBF(g_hDriver);
  }

  return true;
}

bool _stdcall MouseDown(unsigned int nButtons)
{
  if (TYPE_DRIVER_EVENT == g_nDriverType)
  {
    //https://msdn.microsoft.com/en-us/library/ms646260(VS.85).aspx
    mouse_event(nButtons | MOUSEEVENTF_ABSOLUTE, 0, 0, 0, 0);
  }
  else
  {
    MouseControl(nButtons, 0x00, 0x00);
  }

  return true;
}

bool _stdcall MouseUp(unsigned int nButtons)
{
  if (TYPE_DRIVER_EVENT == g_nDriverType)
  {
    //https://msdn.microsoft.com/en-us/library/ms646260(VS.85).aspx
    mouse_event(nButtons | MOUSEEVENTF_ABSOLUTE, 0, 0, 0, 0);
  }
  else
  {
    MouseControl(nButtons, 0x00, 0x00);
  }

  return true;
}

/*
* About Parameters x & y
* mouse move is between [0, 65535];
* if you want to move by logical pixels of desktop,
* you need caculate map between (x / MaxXPixelsOfDesktop * 65535) or (y / MaxYPixelsOfDesktop * 65535)
*/
bool _stdcall MouseMove(unsigned int nX, unsigned int nY)
{
  if (TYPE_DRIVER_EVENT == g_nDriverType)
  {
    //https://msdn.microsoft.com/en-us/library/ms646260(VS.85).aspx
    //https://msdn.microsoft.com/en-us/library/windows/desktop/ms724385(v=vs.85).aspx
    mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE,
      static_cast<unsigned int>(nX * 65535.F / GetSystemMetrics(SM_CXSCREEN)),
      static_cast<unsigned int>(nY * 65535.F / GetSystemMetrics(SM_CYSCREEN)),
      0, 0);
  }
  else
  {
    MouseControl(0x00, nX, nY);
  }

  return true;
}

int _stdcall Initialize(int nDriverType)
{
  g_nDriverType = nDriverType;

  if (TYPE_DRIVER_EVENT == g_nDriverType)
    return 0;

  g_bIs64Bits = Is64Bits();
  g_bMouseWheel = MouseWheel();

  wchar_t szDriverFile[MAX_PATH * 2] = { 0 };
  if (TYPE_DRIVER_WINIO == g_nDriverType)
  {
    std::swprintf(g_szDriverId, sizeof(g_szDriverId), L"%s", NAME_DRIVER_WINIO);
    std::swprintf(szDriverFile, sizeof(szDriverFile), L"\\\\.\\%s", g_szDriverId);
  }
  else if (TYPE_DRIVER_WINRING0 == g_nDriverType)
  {
    std::swprintf(g_szDriverId, sizeof(g_szDriverId), L"%s", NAME_DRIVER_WINRING0);
    std::swprintf(szDriverFile, sizeof(szDriverFile), L"\\\\.\\%s", g_szDriverId);
  }
  else
    return -1;

  // If the driver is not running, install it
  g_hDriver = CreateFile(szDriverFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (INVALID_HANDLE_VALUE == g_hDriver)
  {
    wchar_t szModuleFileName[MAX_PATH * 2] = { 0 };
    if (GetModuleFileName(GetModuleHandle(NULL), szModuleFileName, sizeof(szModuleFileName)))
    {
      wchar_t* szLastSlash = std::wcsrchr(szModuleFileName, '\\');
      if (NULL != szLastSlash)
        szLastSlash[1] = '\0';

      if (TYPE_DRIVER_WINIO == g_nDriverType)
      {
        if (g_bIs64Bits)
          wcscat_s(szModuleFileName, NAME_FILE64_WINIO);
        else
          wcscat_s(szModuleFileName, NAME_FILE32_WINIO);
      }
      else if (TYPE_DRIVER_WINRING0 == g_nDriverType)
      {
        if (g_bIs64Bits)
          wcscat_s(szModuleFileName, NAME_FILE64_WINRING0);
        else
          wcscat_s(szModuleFileName, NAME_FILE32_WINRING0);
      }
      else
        return  -2;

    }

    if (!CServiceControlManager::Create(szModuleFileName, g_szDriverId))
      return -3;
    if (!CServiceControlManager::Start(g_szDriverId))
      return -4;
  }

  g_hDriver = CreateFile(szDriverFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (INVALID_HANDLE_VALUE == g_hDriver)
    return -5;

  // Enable I/O port access for this process if running on a 32 bit OS
  if (TYPE_DRIVER_WINIO == g_nDriverType)
  {
    if (!g_bIs64Bits)
    {
      DWORD nBytesReturned = 0;
      if (!DeviceIoControl(g_hDriver, IOCTL_WINIO_ENABLEDIRECTIO, NULL, 0, NULL, 0, &nBytesReturned, NULL))
        return GetLastError();
    }
  }

  return 0;
}

#pragma endregion

