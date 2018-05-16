#include "stdafx.h"
#include "ServiceControlManager.h"
#include <Windows.h>

CServiceControlManager::CServiceControlManager()
{
}
CServiceControlManager::~CServiceControlManager()
{
}


bool CServiceControlManager::Create(const wchar_t* szServiceFile, const wchar_t* szServiceName)
{
  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms684323(v=vs.85).aspx
  SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (NULL == hSCManager)
    return false;

  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms682450(v=vs.85).aspx
  SC_HANDLE hService = CreateService(hSCManager, szServiceName, szServiceName,
    SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START /* SERVICE_AUTO_START */, 
    SERVICE_ERROR_NORMAL, szServiceFile, NULL, NULL, NULL, NULL, NULL);

  if (NULL == hService)
  {
    CloseServiceHandle(hSCManager);
    return false;
  }

  CloseServiceHandle(hService);
  CloseServiceHandle(hSCManager);

  return true;
}

bool CServiceControlManager::Delete(const wchar_t* szServiceName)
{
  SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (NULL == hSCManager)
    return false;

  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms684330(v=vs.85).aspx
  SC_HANDLE hService = OpenService(hSCManager, szServiceName, SERVICE_ALL_ACCESS);
  if (NULL == hService)
  {
    CloseServiceHandle(hSCManager);
    return false;
  }

  DWORD nBytesNeeded;
  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms684932(v=vs.85).aspx
  BOOL bResult = QueryServiceConfig(hService, NULL, 0, &nBytesNeeded);
  if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
  {
    DWORD nBufferSize = nBytesNeeded;
    LPQUERY_SERVICE_CONFIG pServiceConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, nBufferSize);
    bResult = QueryServiceConfig(hService, pServiceConfig, nBufferSize, &nBytesNeeded);

    if (bResult && SERVICE_DEMAND_START == pServiceConfig->dwStartType)
    {
      //https://msdn.microsoft.com/en-us/library/windows/desktop/ms682562(v=vs.85).aspx
      bResult = DeleteService(hService);
    }
    LocalFree(pServiceConfig);
  }

  CloseServiceHandle(hService);
  CloseServiceHandle(hSCManager);

  return bResult ? true : false;
}

bool CServiceControlManager::Start(const wchar_t* szServiceName)
{
  SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (NULL == hSCManager)
    return false;

  SC_HANDLE hService = OpenService(hSCManager, szServiceName, SERVICE_ALL_ACCESS);
  if (NULL == hService)
  {
    CloseServiceHandle(hSCManager);
    return false;
  }

  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms686321(v=vs.85).aspx
  bool bResult = StartService(hService, 0, NULL) || ERROR_SERVICE_ALREADY_RUNNING == GetLastError();

  CloseServiceHandle(hService);
  CloseServiceHandle(hSCManager);

  return bResult;
}

bool CServiceControlManager::Stop(const wchar_t* szServiceName)
{
  SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (NULL == hSCManager)
    return false;

  SC_HANDLE hService = OpenService(hSCManager, szServiceName, SERVICE_ALL_ACCESS);
  if (NULL == hService)
  {
    CloseServiceHandle(hSCManager);
    return false;
  }

  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms682108(v=vs.85).aspx
  SERVICE_STATUS stServiceStatus;
  BOOL bResult = ControlService(hService, SERVICE_CONTROL_STOP, &stServiceStatus);

  CloseServiceHandle(hService);
  CloseServiceHandle(hSCManager);

  return bResult ? true : false;
}


