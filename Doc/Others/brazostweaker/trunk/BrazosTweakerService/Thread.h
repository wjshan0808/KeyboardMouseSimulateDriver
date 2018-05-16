/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <exception>



/// <summary>Abstract base class for threads.</summary>
class Thread
{
public:

	/// <summary>
	/// Creates a new instance of a thread.
	/// The constructors of derived classes _must_ use this base constructor.
	/// </summary>
	Thread() :
		_thread(NULL)
	{
		_stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		InitializeCriticalSection(&_lock);
	}

	/// <summary>
	/// Destructor. Not thread-safe. ;)
	/// Destructors of derived classes _must_ call the Finish() method
	/// (before cleaning up to make sure resources are not being used in
	/// the object's thread).
	/// </summary>
	virtual ~Thread()
	{
		// we _must_ not call Finish() in the destructor
		// doing so, if Finish() had not been called before (either manually
		// or by the destructor of the derived class), would cause the
		// destructor to first change the object's vtable to this base
		// type Thread and then to call Finish() which waits for the thread
		// to terminate
		// now, thread creation takes some time, so if the object was destructed
		// shortly after calling Start(), the object's vtable could have been
		// altered before the Run() method is called in the object's thread,
		// invoking the pure virtual base implementation Thread::Run()!
		// => cause was hard to determine: altering of vtable before entering the destructor
		// so, Finish() needs to be called either manually before destructing the
		// object or simply by the destructor of the derived class
		CloseHandle(_stopEvent);
		DeleteCriticalSection(&_lock);
	}


	/// <summary>Starts the thread. Thread-safe.</summary>
	void Start()
	{
		Lock();
		{
			if (_thread != NULL)
			{
				Unlock();
				throw std::exception("Thread already started.");
			}

			ResetEvent(_stopEvent);
			_thread = (HANDLE)_beginthreadex(NULL, 0, Invoke, this, 0, NULL);
		}
		Unlock();
	}

	/// <summary>Sends the stop command and returns immediately. Thread-safe.</summary>
	void Stop()
	{
		SetEvent(_stopEvent);
	}

	/// <summary>
	/// Sends the stop command and waits until the thread terminates.
	/// The thread's handle is then closed. Thread-safe.
	/// </summary>
	void Finish()
	{
		Lock();
		{
			if (_thread != NULL)
			{
				Stop();

				WaitForSingleObject(_thread, INFINITE);
				CloseHandle(_thread);
				_thread = NULL;
			}
		}
		Unlock();
	}


protected:

	/// <summary>
	/// Main function of the thread to be implemented by concrete classes.
	/// Deriving classes should check periodically whether the stop command
	/// has been received (by invoking the Wait() method) and return accordingly.
	/// </summary>
	virtual void Run() = NULL;

	/// <summary>
	/// Returns after the specified time or as soon as the stop command is received.
	/// Thread-safe.
	/// </summary>
	/// <param name="ms">Number of milliseconds to wait for (or INFINITE).</param>
	/// <returns>False if the stop command has been received, otherwise true.</returns>
	bool Wait(DWORD ms)
	{
		return (WaitForSingleObject(_stopEvent, ms) != WAIT_OBJECT_0);
	}


private:

	HANDLE _stopEvent;

	CRITICAL_SECTION _lock; // for _thread
	HANDLE _thread;


	/// <summary>Entry point of the thread.</summary>
	static unsigned int WINAPI Invoke(void* argument)
	{
		Thread* thread = (Thread*)argument;
		thread->Run();
		return 0;
		// _endthreadex is automatically invoked after returning
	}


	void Lock()
	{
		EnterCriticalSection(&_lock);
	}

	void Unlock()
	{
		LeaveCriticalSection(&_lock);
	}
};
