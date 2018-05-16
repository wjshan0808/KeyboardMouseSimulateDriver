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


static void InputChar(unsigned char uChar, unsigned int nWaitMs);
static void InputString(const std::string & str, unsigned int nWaitMs);
static void PrintHelp(const char * appname);


typedef bool (_stdcall * _InitializeWinIo)();

typedef void (_stdcall * _ShutdownWinIo)();

typedef bool (_stdcall * _InstallWinIoDriver)(PSTR pszWinIoDriverPath, bool IsDemandLoaded);

typedef bool (_stdcall * _RemoveWinIoDriver)();

typedef bool (_stdcall * _GetPortVal)(WORD wPortAddr, PDWORD pdwPortVal, BYTE bSize);

typedef bool (_stdcall * _SetPortVal)(WORD wPortAddr, DWORD dwPortVal, BYTE bSize);

_InitializeWinIo InitializeWinIo = NULL;
_ShutdownWinIo ShutdownWinIo = NULL;
_InstallWinIoDriver InstallWinIoDriver = NULL;
_RemoveWinIoDriver RemoveWinIoDriver = NULL;
_GetPortVal GetPortVal = NULL;
_SetPortVal SetPortVal = NULL;

int _tmain(int argc, TCHAR** argv)
{
	char szPath[MAX_PATH] = "";
	GetModuleFileNameA(NULL, szPath, MAX_PATH);
	strrchr(szPath, '\\')[1] = '\0';

	std::string sDllPath = szPath;
	sDllPath += "WinIo32.dll";

	std::string sDrvPath = szPath;
	sDrvPath += "WinIo64.sys";

	HMODULE hLib = LoadLibraryA(sDllPath.c_str());
	if (hLib == NULL) {
		printf("Failed to load winio32.dll");
		return -1;
	}

	InitializeWinIo = (_InitializeWinIo)GetProcAddress(hLib, "InitializeWinIo");
	ShutdownWinIo = (_ShutdownWinIo)GetProcAddress(hLib, "ShutdownWinIo");
	InstallWinIoDriver = (_InstallWinIoDriver)GetProcAddress(hLib, "InstallWinIoDriver");
	RemoveWinIoDriver = (_RemoveWinIoDriver)GetProcAddress(hLib, "RemoveWinIoDriver");
	GetPortVal = (_GetPortVal)GetProcAddress(hLib, "GetPortVal");
	SetPortVal = (_SetPortVal)GetProcAddress(hLib, "SetPortVal");


	static unsigned int nWaitMilliSeconds = 25;
	static std::string sInput = "";

	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{ _T("wait"),    ARG_REQ,   0, _T('w') },
			{ _T("install"), ARG_NULL,  0, _T('i') },
			{ _T("remove"),  ARG_NULL,  0, _T('r') },
			{ _T("help"),    ARG_NULL,  0, _T('h') },
			{ ARG_NULL , ARG_NULL , ARG_NULL , ARG_NULL }
		};

		int option_index = 0;
		c = getopt_long(argc, argv, _T("w:hir"), long_options, &option_index);

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

		case _T('w'):
			nWaitMilliSeconds = std::atoi(optarg);
			break;

		case _T('i'):
			if (!InstallWinIoDriver((char *)sDrvPath.c_str(), false)) {
				printf("Failed to install driver, winio64.sys");
				return -1;
			}
			else {
				return 0;
			}

		case _T('r'):
			if (!RemoveWinIoDriver()) {
				printf("Failed to remove driver, winio64.sys");
				return -1;
			}
			else {
				return 0;
			}

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
	if (!InitializeWinIo()) {
		printf("Failed to initialize winio");
		return -1;
	}


	InputString(sInput, nWaitMilliSeconds);

	::Sleep(1000);

	ShutdownWinIo();

	FreeLibrary(hLib);

	return 0;
}

enum {
	KBC_KEY_CMD = 0x64,
	KBC_KEY_DATA = 0x60,
};

static void KBCWait4IBE()
{
	DWORD dwVal = 0;
	do{
		GetPortVal(KBC_KEY_CMD, &dwVal, 1);
	} while ((dwVal & 0x2) > 0);
}

static void MykeyDown(BYTE vkCode)
{
	KBCWait4IBE();
	SetPortVal(KBC_KEY_CMD, (DWORD)0xD2, 1);

	// if not commented, `shift' key is keeping pressed, why?
	// ref: http://wiki.osdev.org/%228042%22_PS/2_Controller
#if 0
	KBCWait4IBE();
	SetPortVal(KBC_KEY_DATA, (DWORD)0xe2, 1);

	KBCWait4IBE();
	SetPortVal(KBC_KEY_CMD, (DWORD)0xD2, 1);
#endif

	KBCWait4IBE();
	SetPortVal(KBC_KEY_DATA, (DWORD)vkCode, 1);
}

static void MykeyUp(BYTE vkCode)
{
	KBCWait4IBE();
	SetPortVal(KBC_KEY_CMD, (DWORD)0xD2, 1);

#if 0 
	KBCWait4IBE();
	SetPortVal(KBC_KEY_DATA, (DWORD)0xE0, 1);

	KBCWait4IBE();
	SetPortVal(KBC_KEY_CMD, (DWORD)0xD2, 1);
#endif

	KBCWait4IBE();
	SetPortVal(KBC_KEY_DATA, (DWORD)(vkCode | 0x80), 1);
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
