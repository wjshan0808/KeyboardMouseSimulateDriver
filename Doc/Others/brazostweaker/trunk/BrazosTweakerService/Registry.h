/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>



/// <summary>
/// Some helper functions for reading values from the registry.
/// </summary>
class Registry
{
public:

	static bool GetString(HKEY key, const char* value, std::string& result)
	{
		if (key == NULL)
			return false;

		char buffer[MAX_PATH];
		DWORD size = MAX_PATH;

		LSTATUS status = RegQueryValueEx(key, value, NULL, NULL, (LPBYTE)buffer, &size);
		if (status == ERROR_SUCCESS)
		{
			result = std::string(buffer, size);
			return true;
		}

		if (status == ERROR_MORE_DATA)
		{
			char* buffer2 = new char[size];

			if (RegQueryValueEx(key, value, NULL, NULL, (LPBYTE)buffer2, &size) == ERROR_SUCCESS)
			{
				result = std::string(buffer2, size);
				delete[] buffer2;
				return true;
			}

			delete[] buffer2;
		}

		return false;
	}

	static bool GetString(HKEY key, const std::string& value, std::string& result)
	{
		return GetString(key, value.c_str(), result);
	}


	static bool GetDword(HKEY key, const char* value, DWORD& result)
	{
		if (key == NULL)
			return false;

		DWORD temp;
		DWORD size = sizeof(DWORD);
		if (RegQueryValueEx(key, value, NULL, NULL, (LPBYTE)&temp, &size) == ERROR_SUCCESS)
		{
			result = temp;
			return true;
		}

		return false;
	}

	static bool GetDword(HKEY key, const char* value, int& result)
	{
		DWORD temp;
		if (GetDword(key, value, temp))
		{
			result = (int)temp;
			return true;
		}

		return false;
	}

	static bool GetBool(HKEY key, const char* value, bool& result)
	{
		DWORD temp;
		if (GetDword(key, value, temp))
		{
			result = (temp != 0);
			return true;
		}

		return false;
	}
};
