/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>

#include <exception>

#ifdef DEBUG
#include <list>
#endif



class CpuLoadMonitor
{
public:

	CpuLoadMonitor()
	{
		_dll = LoadLibraryA("ntdll.dll");
		if (_dll == NULL)
			throw std::exception("Cannot load \"ntdll.dll\".");

		_function = (NtQuerySystemInformationFunction)GetProcAddress(_dll, "NtQuerySystemInformation");
		if (_function == NULL)
		{
			FreeLibrary(_dll);
			throw std::exception("Cannot find entry point \"NtQuerySystemInformation\".");
		}

		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);

		_numCpus = systemInfo.dwNumberOfProcessors;

		// merge the previous state and the new state into a single _state buffer
		_state = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[2 * _numCpus];
		memset(_state, 0, 2 * _numCpus * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));

#ifdef DEBUG
		_histories = new std::list<float>[_numCpus];
#endif
	}

	~CpuLoadMonitor()
	{
		FreeLibrary(_dll);
		delete[] _state;
#ifdef DEBUG
		delete[] _histories;
#endif
	}


	/// <summary>Returns the number of CPUs/cores in the system.</summary>
	size_t GetNumCpus() const
	{
		return _numCpus;
	}

	/// <summary>Gets the load of every CPU/core since the last call, in percent.</summary>
	/// <param name="loads">
	/// Optional array to be filled (length: GetNumCpus()).
	/// If NULL, the state is reinitialized, i.e., the next call will return the load since this call.
	/// </param>
	/// <returns>True if successful, otherwise false.</returns>
	bool GetLoads(float* loads)
	{
		// use the second half of the _state buffer for the new state
		SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* newState = _state + _numCpus;
		if (_function(SystemProcessorPerformanceInformation, newState, (ULONG)(_numCpus * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)), NULL) != 0)
			return false;

		for (size_t i = 0; i < _numCpus; i++)
		{
			const SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION& newInfo = newState[i];
			SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION& oldInfo = _state[i];

			if (loads != NULL)
			{
				const LONGLONG deltaIdle = newInfo.IdleTime.QuadPart - oldInfo.IdleTime.QuadPart;
				const LONGLONG deltaTotal = deltaIdle + (newInfo.UserTime.QuadPart - oldInfo.UserTime.QuadPart); // ignore non-idle kernel ticks

				loads[i] = (deltaTotal == 0 ? 100.0f : 100 * (float)(1 - (double)deltaIdle / deltaTotal));

#ifdef DEBUG
				_histories[i].push_front(loads[i]);
#endif
			}

			oldInfo = newInfo;
		}

		return true;
	}

	/// <summary>Gets the average CPU/core load since the last call, in percent.</summary>
	float GetAverageLoad()
	{
		float* loads = new float[_numCpus];

		if (!GetLoads(loads))
		{
			delete[] loads;
			return 100;
		}

		float sum = 0;
		for (size_t i = 0; i < _numCpus; i++)
			sum += loads[i];

		delete[] loads;

		return sum / _numCpus;
	}


private:

	typedef NTSTATUS (WINAPI *NtQuerySystemInformationFunction)(__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
		__inout PVOID SystemInformation, __in ULONG SystemInformationLength, __out_opt PULONG ReturnLength);

	HMODULE _dll;
	NtQuerySystemInformationFunction _function;

	size_t _numCpus;
	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* _state;

#ifdef DEBUG
	std::list<float> *_histories;
#endif
};
