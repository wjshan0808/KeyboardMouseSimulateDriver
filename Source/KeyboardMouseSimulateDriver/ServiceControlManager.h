#pragma once
#ifndef __C_SERVICE_CONTROL_MANAGER_H__
#define	__C_SERVICE_CONTROL_MANAGER_H__

class CServiceControlManager
{
public:
  CServiceControlManager();
  virtual ~CServiceControlManager();

  static bool Create(const wchar_t* szServiceFile, const wchar_t* szServiceName);
  static bool Delete(const wchar_t* szServiceName);
  static bool Start(const wchar_t* szServiceName);
  static bool Stop(const wchar_t* szServiceName);

};

#endif // !__C_SERVICE_CONTROL_MANAGER_H__
