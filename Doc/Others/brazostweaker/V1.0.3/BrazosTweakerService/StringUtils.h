/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#include <sstream>
#include <vector>



/// <summary>
/// Some helper functions for working with strings.
/// </summary>
class StringUtils
{
public:

	/// <summary>Returns the string representation (according to std::stringstream) of a variable.</summary>
	template <typename T> static std::string ToString(const T& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	/// <summary>
	/// Splits a string into tokens separated by one or more delimiter characters.
	/// Empty tokens are ignored.
	/// </summary>
	static void Tokenize(std::vector<std::string>& result, const char* string, const char* delimiters)
	{
		// check for NULL and empty strings
		if (string == NULL || delimiters == NULL || string[0] == 0 || delimiters[0] == 0)
			return;

		// create a copy of the string as it will be modified during tokenization
		// (every delimiter will be replaced by a NULL character)
		const size_t stringSize = strlen(string) + 1; // include the terminating NULL character
		char* stringCopy = new char[stringSize];
		memcpy(stringCopy, string, stringSize);

		char* remainder;
		char* token = strtok_s(stringCopy, delimiters, &remainder);
		while (token != NULL)
		{
			// skip empty tokens
			if (token[0] != 0)
				result.push_back(std::string(token));

			token = strtok_s(NULL, delimiters, &remainder);
		}

		delete[] stringCopy;
	}
};
