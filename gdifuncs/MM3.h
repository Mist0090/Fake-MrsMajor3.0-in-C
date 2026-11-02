#pragma once

#define COBJMACROS
#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#include <olectl.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <Shlwapi.h>
#include <strsafe.h>
#include <tlhelp32.h>

#define MAX_STR 512

#include "resource.h"
#include "nt.h"

#define CAT(dest, src) StringCchCatA(dest, _countof(dest), src)

extern SIZE scrSize;
extern POINT scrPoint;

INT WINAPI protection64(VOID);
INT WINAPI majorsgui(VOID);
INT WINAPI MainForm(VOID);
INT WINAPI PinkSavage(VOID);
VOID WINAPI logonuiOWR(VOID);