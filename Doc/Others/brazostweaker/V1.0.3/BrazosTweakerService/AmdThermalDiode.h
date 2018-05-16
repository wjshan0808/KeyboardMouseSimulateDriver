/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#include "../WinRing0/OlsApi.h"

#include <exception>



/// <summary>
/// Queries the thermal diode of AMD CPUs (10h and 11h families).
/// </summary>
class AmdThermalDiode
{
public:

	/// <summary>
	/// Constructor.
	/// Beware that a std::exception is thrown if the diode cannot be found.
	/// </summary>
	AmdThermalDiode()
	{
		_pciAddress = FindPciDeviceById(PCI_AMD_VENDOR_ID, PCI_AMD_10H_MISC_DEVICE_ID, 0);

		if (_pciAddress == 0xFFFFFFFF)
		{
			_pciAddress = FindPciDeviceById(PCI_AMD_VENDOR_ID, PCI_AMD_11H_MISC_DEVICE_ID, 0);

			if (_pciAddress == 0xFFFFFFFF)
				throw std::exception("AMD thermal diode not found.");
		}
	}

	/// <summary>
	/// Returns the current CPU temperature (roughly in °C) or -1 if an error occurred.
	/// </summary>
	float GetTemp() const
	{
		DWORD value;
		if (ReadPciConfigDwordEx(_pciAddress, REPORTED_TEMPERATURE_CONTROL_REGISTER, &value))
			return (value >> 21) / 8.0f;

		return -1;
	}


private:

	static const WORD PCI_AMD_VENDOR_ID = 0x1022;
	static const WORD PCI_AMD_10H_MISC_DEVICE_ID = 0x1203;
	static const WORD PCI_AMD_11H_MISC_DEVICE_ID = 0x1303;
	static const DWORD REPORTED_TEMPERATURE_CONTROL_REGISTER = 0xA4;

	DWORD _pciAddress;
};
