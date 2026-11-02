#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#include "gifdec.h"
#include <Windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "resource.h"

static gd_GIF *g_gif = NULL;
static uint8_t *g_frame_buffer = NULL;
static HBITMAP g_hBitmap = NULL;
static HDC g_hMemDC = NULL;
static UINT_PTR g_timerId = 0;

void LoadGifWithGifDec(const char *path, HDC hdc) {
	g_gif = gd_open_gif(path);
	if (!g_gif) return;

	g_frame_buffer = (uint8_t *)malloc(g_gif->width * g_gif->height * 3);
	if (g_frame_buffer) {
		g_gif->canvas = g_frame_buffer;
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char TempGifPath[MAX_PATH]; 
	switch (uMsg) {
	case WM_CREATE: {
		HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

		HRSRC hResInfo = FindResourceA(hInstance, MAKEINTRESOURCEA(IDR_GIF1), "GIF");
		if (!hResInfo) {
			return -1;
		}

		HGLOBAL hResData = LoadResource(hInstance, hResInfo);
		if (!hResData) {
			return -1;
		}

		DWORD gifSize = SizeofResource(hInstance, hResInfo);
		void* pGifData = LockResource(hResData);
		if (!pGifData || gifSize == 0) {
			return -1;
		}

		char TempPath[MAX_PATH];
		GetTempPathA(MAX_PATH, TempPath);
		GetTempFileNameA(TempPath, "GIF", 0, TempGifPath);

		HANDLE hFile = CreateFileA(TempGifPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return -1;
		}

		DWORD written = 0;
		WriteFile(hFile, pGifData, gifSize, &written, NULL);
		CloseHandle(hFile);

		if (written == 0) {
			return -1;
		}

		g_gif = gd_open_gif(TempGifPath);
		DeleteFileA(TempGifPath);

		if (!g_gif) {
			return -1;
		}

		HDC hdcScreen = GetDC(hWnd);
		g_hMemDC = CreateCompatibleDC(hdcScreen);

		BITMAPINFO bmi = { 0 };
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = g_gif->width;
		bmi.bmiHeader.biHeight = -g_gif->height;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 24;
		bmi.bmiHeader.biCompression = BI_RGB;

		void *pBits = NULL;
		g_hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
		ReleaseDC(hWnd, hdcScreen);

		if (!g_hBitmap || !pBits) {
			gd_close_gif(g_gif);
			return -1;
		}

		SelectObject(g_hMemDC, g_hBitmap);
		g_gif->canvas = (uint8_t *)pBits;

		int delay = gd_get_frame(g_gif);
		if (delay > 0) {
			g_timerId = SetTimer(hWnd, 1, delay * 10, NULL);
		}

		ShowCursor(FALSE);
		return 0;
	}
	case WM_CLOSE:
		return 0;

	case WM_KEYDOWN:
		return 0;

	case WM_TIMER:
		if (wParam == g_timerId && g_gif) {
			int delay = gd_get_frame(g_gif);
			if (delay < 0) {
				gd_rewind(g_gif);
				delay = gd_get_frame(g_gif);
			}

			if (g_gif->canvas) {
				size_t total_pixels = (size_t)g_gif->width * g_gif->height;
				uint8_t *p = g_gif->canvas;
				for (size_t i = 0; i < total_pixels; i++) {
					uint8_t r = p[0];
					p[0] = p[2];
					p[2] = r;
					p += 3;
				}
			}

			if (delay > 0) {
				KillTimer(hWnd, g_timerId);
				g_timerId = SetTimer(hWnd, 1, delay * 10, NULL);
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		return 0;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		if (g_hBitmap && g_hMemDC && g_gif) {
			RECT rect;
			GetClientRect(hWnd, &rect);
			int clientWidth = rect.right - rect.left;
			int clientHeight = rect.bottom - rect.top;

			StretchBlt(
				hdc, 0, 0, clientWidth, clientHeight,
				g_hMemDC, 0, 0, g_gif->width, g_gif->height,
				SRCCOPY
			);
		}

		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		if (g_timerId) KillTimer(hWnd, g_timerId);
		if (g_hBitmap) DeleteObject(g_hBitmap);
		if (g_hMemDC) DeleteDC(g_hMemDC);
		if (g_gif) gd_close_gif(g_gif);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSA wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = "logonui";
	RegisterClassA(&wc);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	HWND hwnd = CreateWindowExA(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,"logonui", "logonui",
		WS_POPUP | WS_VISIBLE,
		0, 0, screenWidth, screenHeight,
		NULL, NULL, hInstance, NULL
	);

	if (!hwnd) return 1;

	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return (int)msg.wParam;
}