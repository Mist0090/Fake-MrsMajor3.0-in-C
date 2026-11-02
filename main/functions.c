#include <Windows.h>
#include <strsafe.h>
#include <ShlObj.h>

BOOL WINAPI ExtractFile(INT resourceId, CONST CHAR* resourceType, CONST CHAR* outFileDir, CONST CHAR* outFileName)
{
	// 1. リソースの特定
	HRSRC hResInfo = FindResourceA(NULL, MAKEINTRESOURCE(resourceId), resourceType);
	if (!hResInfo)
		return FALSE;

	// 2. サイズ取得
	DWORD resSize = SizeofResource(NULL, hResInfo);
	if (resSize == 0)
		return FALSE;

	// 3. ロード
	HGLOBAL hResData = LoadResource(NULL, hResInfo);
	if (!hResData)
		return FALSE;

	// 4. ロック（ポインタ取得）
	LPVOID pResBuffer = LockResource(hResData);
	if (!pResBuffer)
		return FALSE;

	// ★改善点1: 親フォルダを階層ごと自動作成する（存在しないフォルダ対策）
	// すでに存在している場合は ERROR_ALREADY_EXISTS が返りますが無視してOKです
	SHCreateDirectoryExA(NULL, outFileDir, NULL);

	// ★改善点2: 安全なパス結合（バッファオーバーラン対策）
	CHAR outFilePath[MAX_PATH];

	// ディレクトリパスをコピー
	if (FAILED(StringCchCopyA(outFilePath, MAX_PATH, outFileDir)))
		return FALSE;

	// 末尾が「\」で終わっていない場合のみ「\」を追加
	size_t len = lstrlenA(outFilePath);
	if (len > 0 && outFilePath[len - 1] != '\\') {
		if (FAILED(StringCchCatA(outFilePath, MAX_PATH, "\\")))
			return FALSE;
	}

	// ファイル名を結合
	if (FAILED(StringCchCatA(outFilePath, MAX_PATH, outFileName)))
		return FALSE;

	// 5. ファイル作成
	HANDLE hFile = CreateFileA(
		outFilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	// 6. 書き込み
	DWORD bytesWritten = 0;
	BOOL bSuccess = WriteFile(hFile, pResBuffer, resSize, &bytesWritten, NULL);

	CloseHandle(hFile);

	return (bSuccess && bytesWritten == resSize);
}

VOID WINAPI SetRegistryString(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, CONST CHAR* valueData)
{
	HKEY hKey;
	if (RegCreateKeyExA(hKeyRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, NULL) == ERROR_SUCCESS) {

		RegSetValueExA(hKey, valueName, 0, REG_SZ, (CONST BYTE*)valueData, (DWORD)(strlen(valueData) + 1));
		RegCloseKey(hKey);
	}
}

VOID WINAPI SetRegistryExString(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, CONST CHAR* valueData)
{
	HKEY hKey;
	if (RegCreateKeyExA(hKeyRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, NULL) == ERROR_SUCCESS) {

		RegSetValueExA(hKey, valueName, 0, REG_EXPAND_SZ, (CONST BYTE*)valueData, (DWORD)(strlen(valueData) + 1));
		RegCloseKey(hKey);
	}
}

VOID WINAPI SetRegistryDword(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, DWORD valueData)
{
	HKEY hKey;
	if (RegCreateKeyExA(hKeyRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, NULL) == ERROR_SUCCESS) {

		RegSetValueExA(hKey, valueName, 0, REG_DWORD, (CONST BYTE*)&valueData, sizeof(DWORD));
		RegCloseKey(hKey);
	}
}

BOOL WINAPI SystemRestart(VOID)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		return FALSE;
	}

	LookupPrivilegeValueA(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges->Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges->Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS) 
	{
		CloseHandle(hToken);
		return FALSE;
	}
	CloseHandle(hToken);

	BOOL bSuccess = ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_OTHER);

	return bSuccess;
}
