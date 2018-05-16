/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

/*
 * Main module, tightly related to the BaseService class.
 */

#include "BrazosTweakerService.h"

#include <iostream>

using namespace std;



// prototypes
void WINAPI ServiceMain(__in DWORD dwArgc, __in LPTSTR* lpszArgv);
DWORD WINAPI ServiceCtrlHandlerEx(__in DWORD dwControl, __in DWORD dwEventType,
	__in LPVOID lpEventData, __in LPVOID lpContext);

// single service instance
static ServiceType* _service = NULL;



#ifdef DEBUG
#include <signal.h>

/// <summary>Ctrl+C handler.</summary>
void SigintHandler(int signal)
{
	if (_service != NULL)
		_service->Stop();
}
#endif



/// <summary>Entry point for the program.</summary>
int main(int argc, char* argv[])
{
#ifdef DEBUG

	// if DEBUG is defined, the program is started normally (not as a service)
	// the "service" is stopped by pressing Ctrl+C (SIGINT)

	signal(SIGINT, SigintHandler);

	ServiceMain(0, NULL);
	return 0;

#else

	// try to install the service if the "-i" switch is used
	if (argc > 1 && _stricmp(argv[1], "-i") == 0)
	{
		// try to connect to the local service control manager
		const SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (scm == NULL)
		{
			cerr << "Could not connect to the service control manager." << endl;
			return 1;
		}

		// get the path of this executable in double quotes
		char buffer[MAX_PATH + 2];
		buffer[0] = '\"';
		GetModuleFileName(NULL, buffer + 1, MAX_PATH);
		strcat_s(buffer, "\"");

		// try to open the existing service
		SC_HANDLE service = OpenService(scm, NAME, SERVICE_STOP | DELETE);
		if (service != NULL)
		{
			// try to stop and delete the service
			SERVICE_STATUS serviceStatus;
			if ((!ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus) && GetLastError() != ERROR_SERVICE_NOT_ACTIVE) ||
				!DeleteService(service))
			{
				cerr << "Could not uninstall the service." << endl;
				CloseServiceHandle(service);
				CloseServiceHandle(scm);
				return 4;
			}

			CloseServiceHandle(service);
			service = NULL;
		}

		// try to install the service
		service = CreateService(scm, NAME, DISPLAY_NAME, SERVICE_CHANGE_CONFIG,
			SERVICE_WIN32_OWN_PROCESS, START_TYPE, SERVICE_ERROR_NORMAL,
			buffer, NULL, NULL, NULL, NULL, "");
		if (service == NULL)
		{
			cerr << "Could not install the service." << endl;
			CloseServiceHandle(scm);
			return 2;
		}

		// attempt to set the description
		SERVICE_DESCRIPTION description;
		description.lpDescription = DESCRIPTION;
		ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &description);

#if defined(ENABLE_DELAYED_AUTOSTART) && START_TYPE == SERVICE_AUTO_START
		// attempt to enable the autostart delay supported by Vista+
		SERVICE_DELAYED_AUTO_START_INFO delayedAutoStartInfo;
		delayedAutoStartInfo.fDelayedAutostart = TRUE;
		ChangeServiceConfig2(service, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &delayedAutoStartInfo);
#endif

		CloseServiceHandle(service);
		CloseServiceHandle(scm);

		return 0;
	}

	// try to uninstall the service if the "-u" switch is used
	if (argc > 1 && _stricmp(argv[1], "-u") == 0)
	{
		// try to connect to the local service control manager
		SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
		if (scm == NULL)
		{
			cerr << "Could not connect to the service control manager." << endl;
			return 1;
		}

		// try to open the service
		SC_HANDLE service = OpenService(scm, NAME, SERVICE_STOP | DELETE);
		if (service == NULL)
		{
			cerr << "Could not uninstall the service." << endl;
			CloseServiceHandle(scm);
			return 3;
		}

		// try to stop and delete the service
		SERVICE_STATUS serviceStatus;
		if ((!ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus) && GetLastError() != ERROR_SERVICE_NOT_ACTIVE) ||
		    !DeleteService(service))
		{
			cerr << "Could not uninstall the service." << endl;
			CloseServiceHandle(service);
			CloseServiceHandle(scm);
			return 4;
		}

		CloseServiceHandle(service);
		CloseServiceHandle(scm);

		return 0;
	}

	// try to start the service
	SERVICE_TABLE_ENTRY dispatchTable[] = { { NAME, ServiceMain }, { 0, NULL } };
	if (!StartServiceCtrlDispatcher(dispatchTable))
		return 10;

	// return when the service has terminated
	return 0;

#endif // DEBUG
}


/// <summary>Invoked when the service is started.</summary>
void WINAPI ServiceMain(__in DWORD dwArgc, __in LPTSTR* lpszArgv)
{
	// try to register the service control handler
	SERVICE_STATUS_HANDLE statusHandle = NULL;

#ifndef DEBUG
	if ((statusHandle = RegisterServiceCtrlHandlerEx(NAME, ServiceCtrlHandlerEx, NULL)) == NULL)
		return;
#endif

	// initialize the status
	SERVICE_STATUS status;
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState = SERVICE_STOPPED;
	status.dwControlsAccepted = 0;
	status.dwWin32ExitCode = NO_ERROR;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;

	// try to create the service instance
	try
	{
		_service = new ServiceType(statusHandle, status);
	}
	catch (exception)
	{
#ifndef DEBUG
		SetServiceStatus(statusHandle, &status);
#endif
		return;
	}

	// set our status to Running as soon as possible, because until then
	// the service control manager is blocked
	_service->UpdateStatus(SERVICE_RUNNING);

	// try to initialize the service
	if (!_service->OnInitialize())
	{
		_service->UpdateStatus(SERVICE_STOPPED);
		delete _service;
		_service = NULL;
		return;
	}

	// start the service
	_service->UpdateStatus(SERVICE_RUNNING, SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_POWEREVENT);
	_service->OnRun();

	// wait until the service is stopped
	_service->Wait(INFINITE);

	_service->UpdateStatus(SERVICE_STOP_PENDING);
	_service->OnStop();
	_service->UpdateStatus(SERVICE_STOPPED);

	delete _service;
	_service = NULL;
}

/// <summary>
/// Invoked asynchronously by the service control manager to respond to commands and notifications.
/// </summary>
DWORD WINAPI ServiceCtrlHandlerEx(__in DWORD dwControl, __in DWORD dwEventType,
	__in LPVOID lpEventData, __in LPVOID lpContext)
{
	switch (dwControl)
	{
		case SERVICE_CONTROL_STOP:
		{
			_service->Stop();
			return NO_ERROR;
		}

		case SERVICE_CONTROL_POWEREVENT:
		{
			_service->OnPowerEvent(dwEventType, (PPOWERBROADCAST_SETTING)lpEventData);
			return NO_ERROR;
		}

		// the MSDN suggests returning NO_ERROR even if this event is not handled:
		case SERVICE_CONTROL_INTERROGATE:
			return NO_ERROR;
	}

	return ERROR_CALL_NOT_IMPLEMENTED;
}
