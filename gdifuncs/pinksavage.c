#include "MM3.h"

FLOAT cur = .05f;
INT akis = 0;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_CREATE:
		SetTimer(hwnd, 1, 250, NULL);
		break;

	case WM_TIMER:
		if (wp == 1) {
			SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			if (cur > 0.60f) {
				akis = 1;
				cur = 0.59f;
			}
			if (cur < 0.05f) {
				akis = 0;
			}

			if (akis == 1) {
				cur -= 0.01f;
			}
			else {
				cur += 0.01f;
			}

			BYTE alpha = (BYTE)(cur * 255.f);
			SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
		}
		break;

	case WM_CLOSE:
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, 1);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

INT WINAPI PinkSavage(VOID)
{
	HINSTANCE hIns = GetModuleHandleA(NULL);

	WNDCLASSEXA wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hIns;
	wc.lpszClassName = "pinksavage";
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = (HICON)NULL;
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(255, 0, 0));
	wc.lpszMenuName = NULL;

	RegisterClassExA(&wc);

	HWND hwnd = CreateWindowExA(
		WS_EX_LAYERED | WS_EX_TRANSPARENT| WS_EX_TOOLWINDOW,
		"pinksavage", "pinksavage", WS_MAXIMIZE | WS_POPUP,
		scrPoint.x, scrPoint.y, scrSize.cx, scrSize.cy,
		NULL, NULL, hIns, NULL
	);

	if (!hwnd)
		return FALSE;

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}