#include "MM3.h"

static INT howmuch = 0;
static INT maxy = 0;

typedef struct {
	FLOAT pictrans;
	INT minx;
	INT miny;
	INT maxx;
	INT maxy;
} WindowData;

typedef struct {
	HWND hwnd;
	INT startpos;
	INT bitir;
	INT kalinlik;
	BOOL isX;
} DropThreadData;

INT WINAPI rand_range(INT min, INT max)
{
	if (min >= max) return min;
	return min + (rand() % (max - min));
}

DWORD WINAPI DropThreadProc(LPVOID lpParam) {
	DropThreadData* data = (DropThreadData*)lpParam;
	HWND hwnd = data->hwnd;
	int startpos = data->startpos;
	int bitir = data->bitir;
	int kalinlik = data->kalinlik;
	BOOL isX = data->isX;
	free(data);

	WindowData* pData = (WindowData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	if (!pData) return 0;

	for (int a = 0; a < bitir; a++) {
		HDC hdc = GetDC(hwnd);
		if (hdc) {
			HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
			HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
			HPEN hOldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));

			int posX = startpos - pData->minx;
			int posY = -20;

			Ellipse(hdc, posX, posY, posX + kalinlik, posY + a);

			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
			DeleteObject(hBrush);
			ReleaseDC(hwnd, hdc);
		}
		Sleep(isX ? 5 : 2);
	}
	return 0;
}

VOID WINAPI sagaCiz(HWND hwnd) {
	WindowData* pData = (WindowData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	if (!pData) return;

	HDC hdc = GetDC(hwnd);
	int redVal = rand_range(100, 255);
	HBRUSH hBrush = CreateSolidBrush(RGB(redVal, 0, 0));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));

	int mysize = rand_range(3, 15);
	int genpos = 0;
	int ranint = rand_range(0, 11);

	if (ranint < 5) genpos = rand_range(-5, 65);
	else if (ranint < 8) genpos = rand_range(65, 120);
	else if (ranint < 10) genpos = rand_range(120, 250);
	else genpos = rand_range(250, 500);

	// クライアント領域内の相対位置
	int posX = (pData->maxx - pData->minx) - genpos;
	int posY = rand_range(0, (pData->maxy - pData->miny) - 50);
	int height = rand_range(3, 15);

	if (rand_range(0, 2) == 1)
		Ellipse(hdc, posX, posY, posX + mysize, posY + height);
	else
		Rectangle(hdc, posX, posY, posX + mysize, posY + height);

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hBrush);
	ReleaseDC(hwnd, hdc);
}

VOID WINAPI solaCiz(HWND hwnd) {
	WindowData* pData = (WindowData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	if (!pData) return;

	HDC hdc = GetDC(hwnd);
	int redVal = rand_range(100, 255);
	HBRUSH hBrush = CreateSolidBrush(RGB(redVal, 0, 0));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));

	int mysize = rand_range(3, 15);
	int genpos = 0;
	int ranint = rand_range(0, 11);

	if (ranint < 5) genpos = rand_range(-5, 65);
	else if (ranint < 8) genpos = rand_range(65, 120);
	else if (ranint < 10) genpos = rand_range(120, 250);
	else genpos = rand_range(250, 500);

	int posX = genpos;
	int posY = rand_range(0, (pData->maxy - pData->miny) - 50);
	int height = rand_range(3, 15);

	if (rand_range(0, 2) == 1)
		Ellipse(hdc, posX, posY, posX + mysize, posY + height);
	else
		Rectangle(hdc, posX, posY, posX + mysize, posY + height);

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hBrush);
	ReleaseDC(hwnd, hdc);
}

VOID WINAPI usteCiz(HWND hwnd) {
	WindowData* pData = (WindowData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	if (!pData) return;

	HDC hdc = GetDC(hwnd);
	int redVal = rand_range(100, 255);
	HBRUSH hBrush = CreateSolidBrush(RGB(redVal, 0, 0));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));

	int mysize = rand_range(3, 15);
	int genpos = 0;
	int ranint = rand_range(0, 11);

	if (ranint < 5) genpos = rand_range(-5, 20);
	else if (ranint < 8) genpos = rand_range(20, 40);
	else if (ranint < 10) genpos = rand_range(40, 60);
	else genpos = rand_range(60, 100);

	int width = pData->maxx - pData->minx;
	int posX = rand_range(20, width - 20);
	int posY = genpos;
	int height = rand_range(3, 15);

	if (rand_range(0, 2) == 1)
		Ellipse(hdc, posX, posY, posX + mysize, posY + height);
	else
		Rectangle(hdc, posX, posY, posX + mysize, posY + height);

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hBrush);
	ReleaseDC(hwnd, hdc);
}

VOID WINAPI altaCiz(HWND hwnd) {
	WindowData* pData = (WindowData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	if (!pData) return;

	HDC hdc = GetDC(hwnd);
	int redVal = rand_range(100, 255);
	HBRUSH hBrush = CreateSolidBrush(RGB(redVal, 0, 0));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));

	int mysize = rand_range(3, 15);
	int genpos = 0;
	int ranint = rand_range(0, 11);

	if (ranint < 5) genpos = rand_range(-5, 20);
	else if (ranint < 8) genpos = rand_range(20, 40);
	else if (ranint < 10) genpos = rand_range(40, 60);
	else genpos = rand_range(60, 100);

	int width = pData->maxx - pData->minx;
	int posX = rand_range(20, width - 20);
	int posY = (pData->maxy - pData->miny) - genpos;
	int height = rand_range(3, 15);

	if (rand_range(0, 2) == 1)
		Ellipse(hdc, posX, posY, posX + mysize, posY + height);
	else
		Rectangle(hdc, posX, posY, posX + mysize, posY + height);

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hBrush);
	ReleaseDC(hwnd, hdc);
}

VOID WINAPI verticalDrop(HWND hwnd) {
	WindowData* pData = (WindowData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	if (!pData) return;

	DropThreadData* data = (DropThreadData*)malloc(sizeof(DropThreadData));
	data->hwnd = hwnd;
	data->startpos = rand_range(pData->minx + 1, pData->maxx - 5);
	data->bitir = rand_range(5, (pData->maxy - pData->miny) - 20);
	data->kalinlik = rand_range(5, 25);
	data->isX = FALSE;

	QueueUserWorkItem(DropThreadProc, data, WT_EXECUTEDEFAULT);
}

HWND WINAPI CreateMainForm(HINSTANCE hInstance)
{
	HWND hwnd = CreateWindowExA(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		"GdifuncsMainForm",
		"gdifuncs",
		WS_POPUP,
		scrPoint.x, scrPoint.y, scrSize.cx, scrSize.cy,
		NULL, NULL, hInstance, NULL
	);

	if (hwnd) {
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}
	return hwnd;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	WindowData* pData = (WindowData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_CREATE: {
		pData = (WindowData*)malloc(sizeof(WindowData));
		pData->pictrans = 0.99f;

		pData->minx = GetSystemMetrics(SM_XVIRTUALSCREEN);
		pData->miny = GetSystemMetrics(SM_YVIRTUALSCREEN);
		pData->maxx = pData->minx + GetSystemMetrics(SM_CXVIRTUALSCREEN);
		pData->maxy = pData->miny + GetSystemMetrics(SM_CYVIRTUALSCREEN) + 20;
		maxy = pData->maxy;

		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)pData);

		LPCREATESTRUCTA pcs = (LPCREATESTRUCTA)lp;

		if (howmuch == 3) {
			howmuch = 4;
			pData->pictrans = 0.99f;
		}
		else if (howmuch == 2) {
			howmuch = 3;
			pData->pictrans = 0.85f;
			CreateMainForm(pcs->hInstance);
		}
		else if (howmuch == 1) {
			howmuch = 2;
			pData->pictrans = 0.75f;
			CreateMainForm(pcs->hInstance);
		}
		else if (howmuch == 0) {
			howmuch = 1;
			pData->pictrans = 0.60f;
			CreateMainForm(pcs->hInstance);

			PlaySoundA(
				MAKEINTRESOURCEA(IDR_WAVE1),
				pcs->hInstance,
				SND_RESOURCE | SND_ASYNC | SND_LOOP
			);

			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PinkSavage, NULL, 0, NULL);

			SetTimer(hwnd, 2, 10, NULL);
		}

		SetLayeredWindowAttributes(hwnd, RGB(128, 0, 0), (BYTE)(pData->pictrans * 255), LWA_COLORKEY | LWA_ALPHA);

		SetTimer(hwnd, 1, 400, NULL);
		break;
	}

	case WM_TIMER:
		if (wp == 1) {
			int i = rand_range(0, 10);
			if (i < 3) sagaCiz(hwnd);
			else if (i >= 3 && i < 5) solaCiz(hwnd);
			else if (i == 5 || i == 8) usteCiz(hwnd);
			else if (i == 6 || i == 7) altaCiz(hwnd);
			else if (i == 9) verticalDrop(hwnd);
		}
		else if (wp == 2) {
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		else if (wp == 3) {
			STARTUPINFOA si = { sizeof(si) };
			PROCESS_INFORMATION pi;
			char cmd[] = "cmd.exe /c taskkill /f /im tobi0a0c.exe";
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;

			if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
			ExitProcess(0);
		}
		break;

	case WM_CLOSE:
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, 1);
		KillTimer(hwnd, 2);
		KillTimer(hwnd, 3);
		if (pData) free(pData);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProcA(hwnd, msg, wp, lp);
	}
	return 0;
}

INT WINAPI MainForm(VOID)
{
	timeBeginPeriod(1);
	srand((UINT)time(NULL));

	HINSTANCE hInstance = GetModuleHandleA(NULL);

	WNDCLASSEXA wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hbrBackground = CreateSolidBrush(RGB(128, 0, 0));
	wc.lpszClassName = "GdifuncsMainForm";

	if (!RegisterClassExA(&wc)) return 0;

	CreateMainForm(hInstance);

	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	timeEndPeriod(1);
	return (INT)msg.wParam;
}