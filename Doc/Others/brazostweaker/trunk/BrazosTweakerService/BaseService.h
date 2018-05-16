/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <exception>



/// <summary>
/// Base class for concrete Windows services.
/// It is tightly related to the Service.cpp module.
/// </summary>
class BaseService
{
public:

	/// <summary>
	/// Creates a new instance of a service.
	/// Deriving classes must use this base constructor and maintain the signature.
	/// Throwing a std::exception is safe.
	/// </summary>
	BaseService(SERVICE_STATUS_HANDLE statusHandle, const SERVICE_STATUS& status) :
		_statusHandle(statusHandle),
		_status(status)
	{
		if ((_stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
			throw std::exception("Cannot create event.");

		InitializeCriticalSection(&_lock);
	}

	/// <summary>Destructor.</summary>
	virtual ~BaseService()
	{
		CloseHandle(_stopEvent);
		DeleteCriticalSection(&_lock);
	}


	/// <summary>Notifies the service control manager about state transitions.</summary>
	/// <returns>True if successful.</returns>
	bool UpdateStatus(DWORD state, DWORD acceptedControls = 0)
	{
		_status.dwCurrentState = state;
		_status.dwControlsAccepted = acceptedControls;

#ifdef DEBUG
		return true;
#else
		return (SetServiceStatus(_statusHandle, &_status) != FALSE);
#endif
	}

	/// <summary>Informs the service that it should stop.</summary>
	void Stop()
	{
		SetEvent(_stopEvent);
	}

	/// <summary>
	/// Returns after the specified time or as soon as the service is stopped.
	/// </summary>
	/// <param name="ms">Number of milliseconds to wait for (or INFINITE).</param>
	/// <returns>False if the service has been stopped, otherwise true.</returns>
	bool Wait(DWORD ms)
	{
		return (WaitForSingleObject(_stopEvent, ms) != WAIT_OBJECT_0);
	}


	/// <summary>Invoked by the main thread when the service is started.</summary>
	/// <returns>True if successful.</returns>
	virtual bool OnInitialize()
	{
		return true;
	}

	/// <summary>
	/// Invoked by the main thread when the service has been initialized.
	/// You should use the Wait() method to check periodically whether the service
	/// has been stopped and return accordingly.
	/// </summary>
	virtual void OnRun()
	{
	}

	/// <summary>
	/// Invoked by the main thread when the service has been stopped.
	/// </summary>
	virtual void OnStop()
	{
	}

	/// <summary>
	/// Invoked asynchronously by the service control manager when system power
	/// events occur.
	/// </summary>
	/// <param name="dwEventType">Something like PBT_APMRESUMEAUTOMATIC.</param>
	virtual void OnPowerEvent(DWORD eventType, PPOWERBROADCAST_SETTING setting)
	{
	}


protected:

	/// <summary>Enters a mutually exclusive critical section.</summary>
	void Lock()
	{
		EnterCriticalSection(&_lock);
	}

	/// <summary>Leaves a mutually exclusive critical section.</summary>
	void Unlock()
	{
		LeaveCriticalSection(&_lock);
	}


private:

	SERVICE_STATUS_HANDLE _statusHandle;
	SERVICE_STATUS _status;

	HANDLE _stopEvent;

	CRITICAL_SECTION _lock;
};
