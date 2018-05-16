/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#include "Thread.h"
#include "Parameters.h"
#include "Registry.h"
#include "CpuLoadMonitor.h"
#include "TurboManager.h"

#include <PowrProf.h>

#undef min
#undef max



/// <summary>
/// Measures periodically the average CPU load and invokes P-state transitions when appropriate.
/// </summary>
class CnQMonitorThread : public Thread
{
public:

	CnQMonitorThread(const CustomCnQProfile& profile) :
		Thread(),
		_profile(profile)
	{
		_swPStateOffset = TurboManager::GetNumBoostedStates();

		// we can only switch to software P-states
		_profile.MinPState = std::max(_swPStateOffset, profile.MinPState);
		_profile.MaxPState = std::max(_swPStateOffset, profile.MaxPState);
	}

	~CnQMonitorThread()
	{
		Finish();
	}


protected:

	void Run()
	{
		if (_profile.Ganged)
			RunGanged();
		else
			RunUnganged();
	}

private:

	CustomCnQProfile _profile;
	int _swPStateOffset;

	CpuLoadMonitor _loadMonitor;


	void RunGanged()
	{
		const CustomCnQProfile& profile = _profile;

		// set the initial P-state to the min P-state
		int currentPState = profile.MinPState;
		SwitchToPState(currentPState);

		// exit if there is only a single allowed P-state
		if (profile.MaxPState == profile.MinPState)
		{
			Sleep(1); // let the transition complete
			return;
		}

		int samplesAbove = 0;
		int samplesBelow = 0;

		// initialize CPU load sampling
		_loadMonitor.GetLoads(NULL);

		while (Wait(profile.SamplingInterval))
		{
			const float load = _loadMonitor.GetAverageLoad();

			if (load > profile.ThresholdUp)
			{
				samplesAbove++;

				if (samplesAbove == profile.NumSamplesUp && currentPState > profile.MinPState)
				{
					currentPState = (profile.AggressiveUp ? profile.MinPState : currentPState - 1);
					SwitchToPState(currentPState); // the transition will complete during the wait period
					samplesAbove = 0;
				}
			}
			else
				samplesAbove = 0;

			if (load < profile.ThresholdDown)
			{
				samplesBelow++;

				if (samplesBelow == profile.NumSamplesDown && currentPState < profile.MaxPState)
				{
					currentPState++;
					SwitchToPState(currentPState); // the transition will complete during the wait period
					samplesBelow = 0;
				}
			}
			else
				samplesBelow = 0;
		}

		// leave the cores in the min P-state
		Sleep(1); // let the last transition complete in case the stop command interrupted the wait period
		SwitchToPState(profile.MinPState);
		Sleep(1); // let the transition complete
	}

	void RunUnganged()
	{
		const CustomCnQProfile& profile = _profile;

		const int numCores = (int)_loadMonitor.GetNumCpus();

		float* loads = new float[numCores];

		int* currentPStates = new int[numCores];
		int* newPStates = new int[numCores];
		int* samplesAboves = new int[numCores];
		int* samplesBelows = new int[numCores];

		// set the initial P-state to the min P-state
		const unsigned int initialPState = profile.MinPState;
		SwitchToPState(initialPState);

		// exit if there is only a single allowed P-state
		if (profile.MaxPState == profile.MinPState)
		{
			Sleep(1); // let the transition complete
			return;
		}

		for (int i = 0; i < numCores; i++)
		{
			currentPStates[i] = initialPState;
			samplesAboves[i] = 0;
			samplesBelows[i] = 0;
		}

		// initialize CPU load sampling
		_loadMonitor.GetLoads(NULL);

		while (Wait(profile.SamplingInterval))
		{
			if (!_loadMonitor.GetLoads(loads))
				break;

			for (int i = 0; i < numCores; i++)
			{
				const float& load = loads[i];
				int& currentPState = currentPStates[i];
				int& newPState = newPStates[i];
				int& samplesAbove = samplesAboves[i];
				int& samplesBelow = samplesBelows[i];

				if (load > profile.ThresholdUp)
				{
					samplesAbove++;

					if (samplesAbove == profile.NumSamplesUp && currentPState > profile.MinPState)
					{
						newPState = (profile.AggressiveUp ? profile.MinPState : currentPState - 1);
						samplesAbove = 0;
					}
				}
				else
					samplesAbove = 0;

				if (load < profile.ThresholdDown)
				{
					samplesBelow++;

					if (samplesBelow == profile.NumSamplesDown && currentPState < profile.MaxPState)
					{
						newPState = currentPState + 1;
						samplesBelow = 0;
					}
				}
				else
					samplesBelow = 0;
			}

			for (int i = 0; i < numCores; i++)
			{
				if (newPStates[i] != currentPStates[i])
				{
					SwitchToPState(newPStates[i], i); // the transition will complete during the wait period
					currentPStates[i] = newPStates[i];
				}
			}
		}

		// leave the cores in the initial P-state
		Sleep(1); // let the last transition complete in case the stop command interrupted the wait period
		SwitchToPState(initialPState);
		Sleep(1); // let the transition complete

		delete[] loads;

		delete[] currentPStates;
		delete[] newPStates;
		delete[] samplesAboves;
		delete[] samplesBelows;
	}


	void SwitchToPState(unsigned int index, int core = -1) const
	{
		if (index > 4)
			return;

		// translate from hardware P-state to software P-state
		index = std::max(0u, index - _swPStateOffset);

		const int start = (core < 0 ? 0 : core);
		const int end   = (core < 0 ? (int)_loadMonitor.GetNumCpus() : start + 1);
		for (int i = start; i < end; i++)
		{
			WrmsrTx(0xC0010062u, index, 0, (DWORD_PTR)1 << i);
		}
	}
};



/// <summary>
/// Checks periodically if the active power scheme has changed and controls the
/// CnQMonitorThread.
/// </summary>
class CustomCnQThread : public Thread
{
public:

	CustomCnQThread(const Parameters& params, HKEY registryKey) :
		Thread(),
		_params(&params),
		_key(registryKey),
		_monitorThread(NULL),
		_dll(NULL),
		_powerGetActiveSchemeFunction(NULL)
	{
		// use Windows power schemes on Vista+
		OSVERSIONINFO versionInfo;
		versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		if (!GetVersionEx(&versionInfo) || versionInfo.dwMajorVersion < 6)
			return;

		// try to link dynamically to PowrProf.dll
		_dll = LoadLibrary("PowrProf.dll");
		if (_dll == NULL)
			return;

		_powerGetActiveSchemeFunction = (PowerGetActiveSchemeFunction)GetProcAddress(_dll, "PowerGetActiveScheme");
		if (_powerGetActiveSchemeFunction == NULL)
		{
			FreeLibrary(_dll);
			_dll = NULL;
		}
	}

	virtual ~CustomCnQThread()
	{
		// stop the monitor thread, wait for it to terminate and delete it
		delete _monitorThread;

		Finish();

		if (_dll != NULL)
			FreeLibrary(_dll);
	}


protected:

	virtual void Run()
	{
		static const int interval = 1000; // in ms

		const Parameters& params = *_params;
		PowerScheme currentPowerScheme = (PowerScheme)-1;

		while (Wait(interval))
		{
			const PowerScheme newPowerScheme = GetCurrentPowerScheme();
			if (newPowerScheme == currentPowerScheme)
				continue;

			// stop the monitor thread, wait for it to terminate and delete it
			if (_monitorThread != NULL)
			{
				delete _monitorThread;
				_monitorThread = NULL;
			}

			const CustomCnQProfile& profile = params.Profiles[(int)newPowerScheme];

			_monitorThread = new CnQMonitorThread(profile);
			_monitorThread->Start();

			// if only a single P-state is allowed, the thread exits immediately
			// let's clean up immediately too
			if (profile.MinPState == profile.MaxPState)
			{
				delete _monitorThread;
				_monitorThread = NULL;
			}

			currentPowerScheme = newPowerScheme;
		}
	}


private:

	typedef DWORD (WINAPI *PowerGetActiveSchemeFunction)(HKEY, GUID**);

	enum PowerScheme
	{
		Balanced = 0,
		HighPerformance = 1,
		PowerSaver = 2
	};

	const Parameters* _params;
	HKEY _key;

	CnQMonitorThread* _monitorThread;

	HMODULE _dll;
	PowerGetActiveSchemeFunction _powerGetActiveSchemeFunction;


	/// <summary>
	/// Returns the currently active power scheme.
	/// If an error occurs, the default power scheme (Balanced) is returned.
	/// </summary>
	PowerScheme GetCurrentPowerScheme()
	{
		if (_powerGetActiveSchemeFunction != NULL)
		{
			GUID* guid;
			if (_powerGetActiveSchemeFunction(NULL, &guid) != ERROR_SUCCESS)
				return Balanced;

			PowerScheme scheme = Balanced;

			if (*guid == GUID_MAX_POWER_SAVINGS)
				scheme = PowerSaver;
			else if (*guid == GUID_MIN_POWER_SAVINGS)
				scheme = HighPerformance;

			LocalFree(guid);

			return scheme;
		}

		// return the current power scheme stored in the registry
		DWORD scheme;
		if (Registry::GetDword(_key, "PowerScheme", scheme))
			return (PowerScheme)scheme;

		return Balanced;
	}
};
