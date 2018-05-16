#pragma once

/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#include "../WinRing0/OlsApi.h"



/// <summary>
/// Manages the Turbo/hardware boost feature of newer CPUs.
/// </summary>
class TurboManager
{
public:

	/// <summary>Returns true if the CPU supports Turbo.</summary>
	static bool IsSupported()
	{
		// cached
		static int result = -1;
		if (result == -1)
		{
			DWORD eax, ebx, ecx, edx;
			if (Cpuid(0x80000007u, &eax, &ebx, &ecx, &edx))
				result = ((edx >> 9) & 1);
			else
				result = 0;
		}

		return (result != 0);
	}


	/// <summary>
	/// Tries to enable/disable the Turbo and returns true if successful.
	/// If the number of boosted P-states is unlocked, it is set appropriately.
	/// </summary>
	static bool Set(bool enable)
	{
		if (!IsSupported())
			return false;

		DWORD lower;
		if (!ReadPciConfigDwordEx(0xC4, 0x15C, &lower))
			return false;

		const bool isLocked = ((lower & 0x80000000u) != 0);

		DWORD newLower = (lower & 0xFFFFFFFCu) | (enable ? 3 : 0);
		// set the number of boosted states if unlocked
		if (!isLocked)
			newLower = (newLower & 0xFFFFFFFBu) | (enable ? 1 << 2 : 0);

		return (newLower == lower || WritePciConfigDwordEx(0xC4, 0x15C, newLower));
	}

	/// <summary>
	/// Tries to set the number of idle cores (in C1) for the Turbo to kick in and
	/// returns true if successful.
	/// </summary>
	static bool SetNumIdleCores(unsigned int num)
	{
		if (!IsSupported())
			return false;

		DWORD lower;
		if (!ReadPciConfigDwordEx(0xC4, 0x15C, &lower))
			return false;

		const bool isLocked = ((lower & 0x80000000u) != 0);
		if (isLocked)
			return false;

		if (!ReadPciConfigDwordEx(0xC4, 0x16C, &lower))
			return false;

		const DWORD newLower = (lower & 0xFFFFF1FFu) | ((num & 7) << 9);

		return (newLower == lower || WritePciConfigDwordEx(0xC4, 0x16C, newLower));
	}


	/// <summary>Returns true if the Turbo is enabled and if there are boosted P-states.</summary>
	static bool IsEnabled()
	{
		if (!IsSupported())
			return false;

		DWORD lower;
		if (!ReadPciConfigDwordEx(0xC4, 0x15C, &lower))
			return true; // assume it's enabled by default if supported

		return ((lower & 7) == 7); // check if enabled and if there is a boosted state
	}

	/// <summary>Returns the number of boosted (Turbo) P-states.</summary>
	static int GetNumBoostedStates()
	{
		if (!IsSupported())
			return 0;

		DWORD lower;
		if (!ReadPciConfigDwordEx(0xC4, 0x15C, &lower))
			return 1; // assume it's enabled by default if supported

		return (lower >> 2) & 1;
	}
};
