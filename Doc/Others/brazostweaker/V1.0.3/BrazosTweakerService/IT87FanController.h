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
/// Manipulates the environment controller of IT87xxF SuperIO chips.
/// </summary>
class IT87FanController
{
public:

	/// <summary>
	/// Constructor.
	/// Beware that a std::exception is thrown if the controller cannot be found.
	/// </summary>
	IT87FanController() :
		_isInitialized(false)
	{
		static const BYTE SUPERIO_REG = 0x2E; // port for the SuperIO register index
		static const BYTE SUPERIO_VAL = 0x2F; // port for the SuperIO register value

		// enter the MB PnP mode
		WriteIoPortByte(SUPERIO_REG, 0x87);
		WriteIoPortByte(SUPERIO_REG, 0x01);
		WriteIoPortByte(SUPERIO_REG, 0x55);
		WriteIoPortByte(SUPERIO_REG, 0x55);

		// check the device ID in registers 0x20 and 0x21
		WORD deviceID = ReadWord(SUPERIO_REG, SUPERIO_VAL, 0x20);
		if ((deviceID >> 8) != 0x87)
			throw std::exception("No IT87xxF chip found.");

		// select logical device #4, the environment controller, by writing 0x04 to register 0x07
		WriteByte(SUPERIO_REG, SUPERIO_VAL, 0x07, 0x04);

		// read the environment controller's ISA address from registers 0x60 and 0x61
		// the register port for the controller is _isaAddress + 0x05, the value port _isaAddress + 0x06
		_isaAddress = ReadWord(SUPERIO_REG, SUPERIO_VAL, 0x60);

		// exit the MB PnP mode
		WriteByte(SUPERIO_REG, SUPERIO_VAL, 0x02, 0x02);
	}

	~IT87FanController()
	{
		if (_isInitialized)
		{
			WriteByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x13, _backup13);
			WriteByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x14, _backup14);

			for (BYTE i = 0; i < 3; i++)
			{
				WriteByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x15 + i, _backups[i]);
			}
		}
	}


	void Initialize()
	{
		// fan controller's Main Control Register (index 0x13)
		_backup13 = ReadByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x13);
		// enable the tachometer for all 3 fans, set the fans to be independent and enable SmartGuardian
		WriteByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x13, 0x77);

		// fan controller's Control Register (index 0x14)
		_backup14 = ReadByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x14);
		// set safe (?) PWM values: active high polarity, 3 MHz base clock, 20% minimum duty, all 3 fans on
		WriteByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x14, 0xDF);

		// set all three fans to full speed
		for (BYTE i = 0; i < 3; i++)
		{
			_backups[i] = ReadByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x15 + i);
			WriteByte(_isaAddress + 0x05, _isaAddress + 0x06, 0x15 + i, 0x7F);
		}

		_isInitialized = true;
	}

	void SetSpeed(int index, float speed) const
	{
		if (index < 0 || index > 2)
			return;

		if (speed < 0)
			speed = 0;
		else if (speed > 100)
			speed = 100;

		char value = (char)(speed/100 * 0x7F + 0.5f);

		WriteByte(_isaAddress + 0x05, _isaAddress + 0x06, (BYTE)(0x15 + index), value);
	}


private:

	WORD _isaAddress;

	bool _isInitialized;
	// original register values:
	BYTE _backup13;   // register 0x13
	BYTE _backup14;   // register 0x14
	BYTE _backups[3]; // registers 0x15 to 0x17


	static BYTE ReadByte(WORD regPort, WORD valPort, BYTE regIndex)
	{
		WriteIoPortByte(regPort, regIndex);
		return ReadIoPortByte(valPort);
	}

	static WORD ReadWord(WORD regPort, WORD valPort, BYTE regIndex)
	{
		WriteIoPortByte(regPort, regIndex);
		WORD value = ReadIoPortByte(valPort) << 8;

		WriteIoPortByte(regPort, regIndex + 1);
		value |= ReadIoPortByte(valPort);

		return value;
	}

	static void WriteByte(WORD regPort, WORD valPort, BYTE regIndex, BYTE value)
	{
		WriteIoPortByte(regPort, regIndex);
		WriteIoPortByte(valPort, value);
	}
};
