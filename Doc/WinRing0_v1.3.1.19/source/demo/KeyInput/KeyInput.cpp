#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include <SDKDDKVer.h>

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <string>

#define STATIC_GETOPT
#include "getopt.h"

// For WbemLocator interface
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
// For _bstr_t
#include <comutil.h>
#pragma comment(lib, "comsuppw.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RUN_TIME_DYNAMIC_LINKING

#ifdef RUN_TIME_DYNAMIC_LINKING
#include "../../dll/OlsApiInit.h"
#else // for Load-Time Dynamic Linking
#include "../../dll/OlsApi.h"
#ifdef _M_X64
#pragma comment(lib, "../../../release/WinRing0x64.lib")
#else if
#pragma comment(lib, "../../../release/WinRing0.lib")
#endif
#endif

static void InputChar(unsigned char uChar, unsigned int nWaitMs);
static void InputString(const std::string & str, unsigned int nWaitMs);
static void PrintHelp(const char * appname);

int _tmain(int argc, TCHAR** argv)
{
	static unsigned int nWaitMilliSeconds = 25;
	static std::string sInput = "";

	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{ _T("wait"),    ARG_REQ,  0, _T('w') },
			{ _T("help"),    ARG_NULL,  0, _T('h') },
			{ ARG_NULL , ARG_NULL , ARG_NULL , ARG_NULL }
		};

		int option_index = 0;
		c = getopt_long(argc, argv, _T("w:h"), long_options, &option_index);

		// Check for end of operation or error
		if (c == -1)
			break;

		// Handle options
		switch (c)
		{
		case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options[option_index].flag != 0)
				break;
			if (!strcmp(long_options[option_index].name, "wait")) {
				nWaitMilliSeconds = std::atoi(optarg);
			}
			break;

		case _T('w'):
			nWaitMilliSeconds = std::atoi(optarg);
			nWaitMilliSeconds = min(200, nWaitMilliSeconds);
			break;

		case _T('h'):
			PrintHelp(argv[0]);
			return 0;

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			abort();
		}
	}
	
	if (optind < argc)
	{
		sInput = argv[optind];
	}

	// End of command line parsing.
	// Start to Input characters.

#ifdef RUN_TIME_DYNAMIC_LINKING
	HMODULE m_hOpenLibSys = NULL;
	if (InitOpenLibSys(&m_hOpenLibSys) != TRUE)
	{
		fprintf(stderr, ("DLL Load Error!!"));
		return -1;
	}
#else if
	InitializeOls();
#endif

	InputString(sInput, nWaitMilliSeconds);

	::Sleep(1000);
	
#ifdef RUN_TIME_DYNAMIC_LINKING
	//DeinitOpenLibSys(&m_hOpenLibSys);
#else
	//DeinitializeOls();
#endif

	return 0;
}

enum {
	KBC_KEY_CMD = 0x64,
	KBC_KEY_DATA = 0x60,
};

static void KBCWait4IBE()
{
	BYTE dwVal = 0;
	do{
		dwVal = ReadIoPortByte(KBC_KEY_CMD);
	} while ((dwVal & 0x2) > 0);
}

static void MykeyDown(BYTE vkCode)
{
	KBCWait4IBE();
	WriteIoPortByte(KBC_KEY_CMD, 0xD2);

	// if not commented, `shift' key is keeping pressed, why?
	// ref: http://wiki.osdev.org/%228042%22_PS/2_Controller
#if 0
	KBCWait4IBE();
	WriteIoPortByte(KBC_KEY_DATA, 0xe2);

	KBCWait4IBE();
	WriteIoPortByte(KBC_KEY_CMD, 0xD2);
#endif
	KBCWait4IBE();
	WriteIoPortByte(KBC_KEY_DATA, vkCode);
}

static void MykeyUp(BYTE vkCode)
{
	KBCWait4IBE();
	WriteIoPortByte(KBC_KEY_CMD, 0xD2);

#if 0 
	KBCWait4IBE();
	WriteIoPortByte(KBC_KEY_DATA, 0xE0);

	KBCWait4IBE();
	WriteIoPortByte(KBC_KEY_CMD, 0xD2);
#endif

	KBCWait4IBE();
	WriteIoPortByte(KBC_KEY_DATA, vkCode | 0x80);
}

static void InputChar(unsigned char uChar, unsigned int nWaitMilliseconds)
{
	//char sLog[100] = "";
	//sprintf(sLog, "KeyInput: [%c]\n", (char )uChar);
	//OutputDebugStringA(sLog);

	// ref: https://msdn.microsoft.com/en-us/library/windows/desktop/ms646329(v=vs.85).aspx
	USHORT usVkey = VkKeyScanA(uChar);
	unsigned char shift = MapVirtualKey(VK_LSHIFT, 0); 
	unsigned char captial = MapVirtualKey(VK_CAPITAL, 0); 
	unsigned char scanCode = MapVirtualKey((unsigned char)(0x00ff & usVkey), 0);

	if (usVkey & 0x0100) { // `shift' key pressed
		MykeyDown(shift);
		Sleep(nWaitMilliseconds);

		MykeyDown(scanCode);
		Sleep(nWaitMilliseconds);

		MykeyUp(scanCode);
		Sleep(nWaitMilliseconds);

		MykeyUp(shift);
		Sleep(nWaitMilliseconds);
	}
	else {
		MykeyDown(scanCode);
		Sleep(nWaitMilliseconds);

		MykeyUp(scanCode);
		Sleep(nWaitMilliseconds);
	}
}

static void InputString(const std::string & str, unsigned int nWaitMs)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		unsigned char uchar = (unsigned char)*it;
		InputChar(uchar, nWaitMs);
	}
}

static void PrintHelp(const char * appname)
{
	char filename[100] = "";
	_splitpath(appname, NULL, NULL, filename, NULL);
	
	fprintf(stdout, "Usage: \n"
		"\t %s [-w|--wait <ms>] [-h|--help] <stringtoinput>\n"
		"\t -w --wait \t milliseconds to wait between characters, default 15ms.\n"
		"\t -h --help \t print this help.\n"
		, filename);
}
