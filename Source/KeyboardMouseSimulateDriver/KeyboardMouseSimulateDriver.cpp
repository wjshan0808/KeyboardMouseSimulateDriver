#include "stdafx.h"
#include "KeyboardMouseSimulateDriver.h" 
#include "KeyboardMouseSimulateDriverDefines.h"
#include "ServiceControlManager.h"

#include <cwchar>
#include <cstdlib>
#include <ctime>
#include <conio.h>


HANDLE g_hDriver = INVALID_HANDLE_VALUE;
bool g_bIs64BitOS = false;

bool _stdcall Is64BitOS()
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

  PFN_GetSystemWow64Directory pfnGetSystemWow64Directory = (PFN_GetSystemWow64Directory)GetProcAddress(hKernel32, "GetSystemWow64DirectoryW");
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


BOOL _stdcall GetPortValue(HANDLE pHandle, WORD nAddress, PDWORD pValue, BYTE nSize)
{
  DWORD nReturned = 0;
  Port stPort;
  stPort.m_nPortSize = nSize;
  stPort.m_nPortAddress = nAddress;

#ifdef _WIN64
  return DeviceIoControl(pHandle, IOCTL_WINIO_READPORT, &stPort, sizeof(Port), pValue, sizeof(DWORD), &nReturned, NULL);
#elif _WIN32
  // If this is a 64 bit OS, we must use the driver to access I/O ports 
  // even if the application is 32 bit
  if (!g_bIs64BitOS)
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
  return DeviceIoControl(pHandle, IOCTL_WINIO_READPORT, &stPort, sizeof(Port), pValue, sizeof(DWORD), &nReturned, NULL);
#endif
}

BOOL _stdcall SetPortValue(HANDLE pHandle, WORD nAddress, DWORD nValue, BYTE nSize)
{
  DWORD nReturned = 0;
  Port stPort;
  stPort.m_nPortSize = nSize;
  stPort.m_nPortValue = nValue;
  stPort.m_nPortAddress = nAddress;

#ifdef _WIN64
  return DeviceIoControl(pHandle, IOCTL_WINIO_WRITEPORT, &stPort, sizeof(Port), NULL, 0, &nReturned, NULL);
#elif _WIN32
  // If this is a 64 bit OS, we must use the driver to access I/O ports 
  // even if the application is 32 bit
  if (!g_bIs64BitOS)
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
  return DeviceIoControl(pHandle, IOCTL_WINIO_WRITEPORT, &stPort, sizeof(Port), NULL, 0, &nReturned, NULL);
#endif
}


void _stdcall KBCWait4IBE(HANDLE pHandle)
{
  DWORD nValue = 0;
  do
  {
    GetPortValue(pHandle, CTL_IO_KEYBOARD_CMD, &nValue, MAPVK_VSC_TO_VK);
  } while (nValue & 0x02); //反复检查键盘输入缓冲区, 等待为空
}


void _stdcall Uninitialize()
{
  if (INVALID_HANDLE_VALUE != g_hDriver)
  {
    // Disable I/O port access if running on a 32 bit OS
    if (!g_bIs64BitOS)
    {
      DWORD nBytesReturned;
      DeviceIoControl(g_hDriver, IOCTL_WINIO_DISABLEDIRECTIO, NULL, 0, NULL, 0, &nBytesReturned, NULL);
    }
    CloseHandle(g_hDriver);
  }
  CServiceControlManager::Stop(L"WINIO");
  CServiceControlManager::Delete(L"WINIO");
}
//// 对 8042 键盘控制芯片进行编程    
//// 允许 鼠标 接口   
//io_write_to_io_port(0x64, 0xa8);
//
//// 通知 8042 下个字节的发向 0x60 的数据将发给 鼠标  
//io_write_to_io_port(0x64, 0xd4);
//
//// 允许 鼠标 发数据  
//io_write_to_io_port(0x60, 0xf4);
//
//// 通知 8042,下个字节的发向 0x60 的数据应放向 8042 的命令寄存器  
//io_write_to_io_port(0x64, 0x60);
//
//// 许可键盘及 鼠标 接口及中断  
//io_write_to_io_port(0x60, 0x47);
//此功能被用来模拟来自于Mouse发送的数据.
bool _stdcall MouseDown(unsigned int nMouseValue)
{
  BOOL bResult = true;

  KBCWait4IBE(g_hDriver);
  unsigned int nMapVirtualKey = MapVirtualKey(nMouseValue, MAPVK_VK_TO_VSC);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_CMD, 0xD3, MAPVK_VSC_TO_VK); //0xD3

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, nMapVirtualKey, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, 0x00, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, 0x00, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, 0x00, MAPVK_VSC_TO_VK);

  return bResult ? true : false;
}

bool _stdcall MouseUp(unsigned int nMouseValue)
{
  BOOL bResult = true;

  //KBCWait4IBE(g_hDriver);
  //unsigned int nMapVirtualKey = nMouseValue;// MapVirtualKey(nMouseValue, MAPVK_VK_TO_VSC);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_CMD, 0xD3, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, 0x08, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, 0x00, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, 0x00, MAPVK_VSC_TO_VK);
  
  return bResult ? true : false;
}


short _stdcall KeyStatus(unsigned int nKeyValue)
{
  return GetKeyState(nKeyValue);
}

//此功能被用来模拟来自于Keyboard发送的数据, 如果中断被允许, 则会触发一个中断处理.
bool _stdcall KeyDown(unsigned int nKeyValue)
{
  BOOL bResult = true;
  //Keydown
  KBCWait4IBE(g_hDriver);
  //https://msdn.microsoft.com/en-us/library/ms646306(VS.85).aspx
  unsigned int nMapVirtualKey = MapVirtualKey(nKeyValue, MAPVK_VK_TO_VSC);

  KBCWait4IBE(g_hDriver);
  //0xD2准备写数据到Output Register中
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_CMD, 0xD2, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  //0x60将写入到Input Register的字节放入到Output Register中，
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, 0x60, MAPVK_VSC_TO_VK); // 0xE2

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_CMD, 0xD2, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, nMapVirtualKey, MAPVK_VSC_TO_VK);

  return bResult ? true : false;
}

bool _stdcall KeyUp(unsigned int nKeyValue)
{
  BOOL bResult = true;
  //Keyup
  KBCWait4IBE(g_hDriver);
  unsigned int nMapVirtualKey = MapVirtualKey(nKeyValue, MAPVK_VK_TO_VSC);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_CMD, 0xD2, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, 0x60, MAPVK_VSC_TO_VK); // 0xE0

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_CMD, 0xD2, MAPVK_VSC_TO_VK);

  KBCWait4IBE(g_hDriver);
  bResult &= SetPortValue(g_hDriver, CTL_IO_KEYBOARD_DATA, nMapVirtualKey | 0x80, MAPVK_VSC_TO_VK);

  return bResult ? true : false;
}


bool _stdcall EventKeyDown(unsigned int nKeyValue)
{
  //Keydown 
  unsigned int nMapVirtualKey = MapVirtualKey(nKeyValue, MAPVK_VK_TO_VSC);
   
  //https://msdn.microsoft.com/en-us/library/ms646304(VS.85).aspx
  keybd_event(nKeyValue, nMapVirtualKey, 0, 0);

  return true;
}

bool _stdcall EventKeyUp(unsigned int nKeyValue)
{
  //Keyup 
  unsigned int nMapVirtualKey = MapVirtualKey(nKeyValue, MAPVK_VK_TO_VSC);

  keybd_event(nKeyValue, nMapVirtualKey, KEYEVENTF_KEYUP, 0);

  return true;
}


int _stdcall Initialize()
{
  g_bIs64BitOS = Is64BitOS();

  g_hDriver = CreateFile(L"\\\\.\\WINIO", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  // If the driver is not running, install it
  if (INVALID_HANDLE_VALUE == g_hDriver)
  {
    TCHAR szModuleFileName[32768] = { 0 }; // 32768
    if (GetModuleFileName(GetModuleHandle(NULL), szModuleFileName, sizeof(szModuleFileName)))
    {
      PWSTR szLastSlash = std::wcsrchr(szModuleFileName, '\\');
      if (NULL != szLastSlash)
        szLastSlash[1] = '\0';
      if (g_bIs64BitOS)
        wcscat_s(szModuleFileName, L"winio64.sys");
      else
        wcscat_s(szModuleFileName, L"winio32.sys");
    }

    if (!CServiceControlManager::Create(szModuleFileName, L"WINIO"))
      return 1;
    if (!CServiceControlManager::Start(L"WINIO"))
      return 2;
  }

  g_hDriver = CreateFile(L"\\\\.\\WINIO", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (INVALID_HANDLE_VALUE == g_hDriver)
    return 3;

  // Enable I/O port access for this process if running on a 32 bit OS
  if (!g_bIs64BitOS)
  {
    DWORD nBytesReturned = 0;
    if (!DeviceIoControl(g_hDriver, IOCTL_WINIO_ENABLEDIRECTIO, NULL, 0, NULL, 0, &nBytesReturned, NULL))
    {
      return GetLastError();
    }
  }
  return 0;
}


long long _stdcall Checkout()
{
  return (std::time(nullptr) + 60 * 60 * 8);
}
