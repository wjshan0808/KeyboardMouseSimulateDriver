#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strsafe.h>
#include <string>
#pragma warning(disable:4996)

const char * TargetSeparator = "#";
typedef BOOL(*FindEditCallback)(HWND hAppWindow, HWND hChildToCheck, void * userparam);

struct EnumAppWndParam
{
	FindEditCallback cb;
	void * userparam;
	HWND target;
};

struct EnumChildWndParam
{
	FindEditCallback cb;
	void * userparam;
	HWND hAppWnd;
	HWND target;
};

struct TargetRule
{
public:
	TargetRule() {
		domain = NULL;
		title = NULL;
		editclass = NULL;
	}
	~TargetRule() {
		if (domain) {
			free(domain); domain = NULL;
		}
		if (title) {
			free(title); title = NULL;
		}
		if (editclass) {
			free(editclass); title = NULL;
		}
	}
public:
	char * domain;
	char * title;
	char * editclass;
};

static HWND FindEdit(FindEditCallback cb, void * userparam);
static BOOL CALLBACK AppWindowEnumProc(HWND, LPARAM);
static BOOL CALLBACK ChildWindowEnumProc(HWND, LPARAM);
static BOOL IsTargetEdit(HWND hAppWnd, HWND hChildToCheck, void * userparam);
static void PError();
static std::string AnsiToUTF8(const char * ansiString);
static bool ParseRule(const char * _sRule, TargetRule * rule);
static void AimEditCtrl(HWND hEditCtrl);
static void ActiveAppWindow(HWND hAppWindow);

int main(int argc, char * argv[])
{
	if (argc > 1 && argv[1]) {
		HWND hEdit = FindEdit(IsTargetEdit, (void *) (AnsiToUTF8( (const char *) argv[1]).c_str()));
		if (hEdit) {
			AimEditCtrl(hEdit);
		}
	}
	return 0;
}

static HWND FindEdit(FindEditCallback cb, void * userparam)
{
	EnumAppWndParam param; 
	memset(&param, 0, sizeof(param));

	param.cb = cb;
	param.userparam = userparam;
	
	EnumWindows(AppWindowEnumProc, (LPARAM)&param);

	return param.target;
}

static BOOL CALLBACK AppWindowEnumProc(HWND hAppWnd, LPARAM _param)
{
	EnumAppWndParam * appWndParam = (EnumAppWndParam *)_param;

	EnumChildWndParam param;
	memset(&param, 0, sizeof(param));

	param.cb = appWndParam->cb;
	param.hAppWnd = hAppWnd;
	param.userparam = appWndParam->userparam;

	EnumChildWindows(hAppWnd, ChildWindowEnumProc, (LPARAM)&param);

	if (param.target != NULL) {
		ActiveAppWindow(hAppWnd);
		appWndParam->target = param.target;
		return FALSE; // Found
	}

	return TRUE; // go on to enumeration
}

static BOOL CALLBACK ChildWindowEnumProc(HWND hChildWindow, LPARAM _param)
{
	EnumChildWndParam * childWndParam = (EnumChildWndParam *)_param;

	if (childWndParam->cb(childWndParam->hAppWnd, hChildWindow, childWndParam->userparam)) {
		childWndParam->target = hChildWindow;
		return FALSE; // Found.
	}
	
	return TRUE; // go on to enumeration
}

/*
*
*/
static BOOL IsTargetEdit(HWND hAppWnd, HWND hChildToCheck, void * userparam)
{
	const char * sRule = (const char *)userparam;
	TargetRule rule;
	if (!ParseRule(sRule, &rule)) {
		return FALSE;
	}

	char szLog[MAX_PATH] = "";
	sprintf(szLog, "AppWind: %p; Child: %p\n", hAppWnd, hChildToCheck);
	OutputDebugStringA(szLog);

	std::string sTitle;
	do {
		wchar_t wszTitle[MAX_PATH] = L"";
		GetWindowTextW(hAppWnd, wszTitle, MAX_PATH);
		char szTitle[MAX_PATH] = "";
		WideCharToMultiByte(CP_UTF8, 0, wszTitle, -1, szTitle, MAX_PATH, NULL, NULL);
		sTitle = szTitle;
	} while (0);

	std::string sEditClass;
	do {
		char szEditClass[MAX_PATH] = "";
		GetClassNameA(hChildToCheck, szEditClass, MAX_PATH);
		sEditClass = szEditClass;
	} while (0);

	char * domaintitle = rule.title;
	char * editclass = rule.editclass;

#if 0
	if (!stricmp(rule.domain, "icbc")) {
		 /*char * */domaintitle = "\xE4\xB8\xAD\xE5\x9B\xBD\xE5\xB7\xA5\xE5\x95\x86\xE9\x93\xB6\xE8\xA1\x8C\xE6\x96\xB0\xE4\xB8\x80\xE4\xBB\xA3\xE7\xBD\x91\xE4\xB8\x8A\xE9\x93\xB6\xE8\xA1\x8C";
		 /*char * */editclass = "ATL:1277F280";
	}
	else if (!stricmp(rule.domain, "95559")) {
		/*char * */domaintitle = "\xE4\xBA\xA4\xE9\x80\x9A\xE9\x93\xB6\xE8\xA1\x8C\xE4\xB8\xAA\xE4\xBA\xBA\xE7\xBD\x91\xE9\x93\xB6";
		/*char * */editclass = "ATL:0F8A2B98";

	}
	else if (!stricmp(rule.domain, "boc")) {
		/*char * */domaintitle = "\xE4\xB8\xAD\xE5\x9B\xBD\xE9\x93\xB6\xE8\xA1\x8C";
		/*char * */editclass = "ATL:07765390";

	}
	else if (!stricmp(rule.domain, "95599")) {
		/*char * */domaintitle = "\xE4\xB8\xAA\xE4\xBA\xBA\xE7\xBD\x91\xE4\xB8\x8A\xE9\x93\xB6\xE8\xA1\x8C\xE7\x94\xA8\xE6\x88\xB7\xE5\x90\x8D\xE7\x99\xBB\xE5\xBD\x95";
		/*char * */editclass = "AfxOleControl80su";

	}
	else if (!stricmp(rule.domain, "cmbchina")) {
		/*char * */domaintitle = "pbsz.ebank.cmbchina.com";
		/*char * */editclass = "ATL:07741200";

	}
	else {

	}
#endif

	if (domaintitle && sTitle.find(domaintitle) != sTitle.npos
		&& editclass && sEditClass.find(editclass) != sEditClass.npos) {
		return TRUE;
	}

	return FALSE;
}

static bool ParseRule(const char * _sRule, TargetRule * rule)
{
	char * sRule = strdup(_sRule);

	char * tok = strtok(sRule, TargetSeparator);
	int index = 0;
	while (tok != NULL) {
		switch (index) {
		case 0: {
			rule->domain = strdup(tok);
		}break;
		case 1: {
			rule->title = strdup(tok);
		}break;
		case 2: {
			rule->editclass = strdup(tok);
		}break;
		}
		index++;
		tok = strtok(NULL, TargetSeparator);
	}

	free(sRule);

	if (index == 3) 
		return true;

	return false;
}

void PError()
{
	// Retrieve the system error message for the last-error code
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process
	OutputDebugStringA((const char *)lpMsgBuf);

	LocalFree(lpMsgBuf);
}

static void AimEditCtrl(HWND hEditCtrl)
{
	PostMessage(hEditCtrl, WM_LBUTTONDOWN, MK_LBUTTON, 0x00050005);
	PostMessage(hEditCtrl, WM_LBUTTONUP,   MK_LBUTTON, 0x00050005);
}

static std::string AnsiToUTF8(const char * ansiString)
{
	wchar_t wszTemp[MAX_PATH] = L"";
	MultiByteToWideChar(CP_ACP, 0, ansiString, -1, wszTemp, MAX_PATH);
	char szTemp[MAX_PATH] = "";
	WideCharToMultiByte(CP_UTF8, 0, wszTemp, -1, szTemp, MAX_PATH, NULL, NULL);
	return szTemp;
}

static void ActiveAppWindow(HWND hAppWnd)
{
	if (IsIconic(hAppWnd)) {
		ShowWindow(hAppWnd, SW_RESTORE);
	}
	else {
		ShowWindow(hAppWnd, SW_SHOWDEFAULT);
	}
	SetForegroundWindow(hAppWnd);
	SetActiveWindow(hAppWnd);
	BringWindowToTop(hAppWnd);
}
