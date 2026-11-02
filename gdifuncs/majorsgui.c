#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include "gifdec.h"

#include "MM3.h"

// --- グローバル変数エリアに追加 ---
HFONT g_hFontLabel = NULL;
HBRUSH g_hKeyBrush = NULL;
HBRUSH g_hDarkRedBrush = NULL;
HBRUSH g_hBorderBrush = NULL;

HINSTANCE g_hInstance = NULL;
HWND g_hWndMain = NULL;

BOOL g_bDragging = FALSE;
POINT g_ptClicked;
int g_alrdyfed = 0;
int g_barHeight = 3;

typedef struct {
	gd_GIF* gif;
	HBITMAP hBitmap;
	void* pBits;
	uint8_t* frame_buf;
} GifPlayer;

GifPlayer g_gif = { 0 };

static void ConvertGifRgbToBGRA(gd_GIF* gif, const uint8_t* srcRgb, unsigned char* pDstBits) {
	if (!gif || !srcRgb || !pDstBits) return;

	int totalPixels = gif->width * gif->height;

	int trans_idx = -1;
	if (gif->gce.tindex < 256) {
		trans_idx = (int)gif->gce.tindex;
	}

	uint8_t trans_r = 0, trans_g = 0, trans_b = 0;
	if (trans_idx >= 0) {
		gd_Palette* palette = gif->palette ? gif->palette : &gif->gct;
		if (palette && palette->colors) {
			trans_r = palette->colors[trans_idx * 3 + 0];
			trans_g = palette->colors[trans_idx * 3 + 1];
			trans_b = palette->colors[trans_idx * 3 + 2];
		}
		else {
			trans_idx = -1;
		}
	}

	for (int i = 0; i < totalPixels; i++) {
		uint8_t r = srcRgb[i * 3 + 0];
		uint8_t g = srcRgb[i * 3 + 1];
		uint8_t b = srcRgb[i * 3 + 2];

		// GIFが「ここ透明ね」と言っている場所、または純粋なマゼンタ色は、
		// ウィンドウのカラーキー（マゼンタ）に置き換えて完全に透過させる
		if ((trans_idx >= 0 && r == trans_r && g == trans_g && b == trans_b) || (r == 255 && g == 0 && b == 255)) {
			pDstBits[i * 4 + 0] = 255; // B (255)
			pDstBits[i * 4 + 1] = 0;   // G (0)
			pDstBits[i * 4 + 2] = 255; // R (255) -> マゼンタ = 透過色
			pDstBits[i * 4 + 3] = 255;
		}
		else {
			pDstBits[i * 4 + 0] = b; // B
			pDstBits[i * 4 + 1] = g; // G
			pDstBits[i * 4 + 2] = r; // R
			pDstBits[i * 4 + 3] = 255; // A
		}
	}
}
BOOL LoadGifWithGifDec(const char* filename, HDC hdcRef) {
	gd_GIF* gif = gd_open_gif(filename);
	if (!gif) {
		OutputDebugStringA("[ERROR] GIFファイルが開けませんでした\n");
		return FALSE;
	}

	g_gif.gif = gif;
	g_gif.frame_buf = (uint8_t*)malloc(gif->width * gif->height * 3);

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = gif->width;
	bmi.bmiHeader.biHeight = -gif->height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	g_gif.hBitmap = CreateDIBSection(hdcRef, &bmi, DIB_RGB_COLORS, &g_gif.pBits, NULL, 0);
	if (!g_gif.hBitmap || !g_gif.pBits) {
		gd_close_gif(gif);
		g_gif.gif = NULL;
		return FALSE;
	}

	if (gd_get_frame(gif) > 0) {
		gd_render_frame(gif, g_gif.frame_buf);
		ConvertGifRgbToBGRA(gif, g_gif.frame_buf, (unsigned char*)g_gif.pBits);
	}

	return TRUE;
}

void AdvanceGifFrame(void) {
	if (!g_gif.gif || !g_gif.pBits || !g_gif.frame_buf) return;

	gd_GIF* gif = g_gif.gif;

	int ret = gd_get_frame(gif);
	if (ret <= 0) {
		gd_rewind(gif);
		ret = gd_get_frame(gif);
	}

	if (ret > 0) {
		gd_render_frame(gif, g_gif.frame_buf);
		ConvertGifRgbToBGRA(gif, g_gif.frame_buf, (unsigned char*)g_gif.pBits);
	}
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	char TempGifPath[MAX_PATH];
	switch (message) {
	case WM_CREATE: {

		SetLayeredWindowAttributes(hWnd, RGB(255, 0, 255), 0, LWA_COLORKEY);

		g_hKeyBrush = CreateSolidBrush(RGB(255, 0, 255));
		g_hDarkRedBrush = CreateSolidBrush(RGB(192, 0, 0));
		g_hBorderBrush = CreateSolidBrush(RGB(255, 0, 1));
		g_hFontLabel = CreateFontA(21, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Microsoft Sans Serif");

		HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		HRSRC hResInfo = FindResourceA(hInstance, MAKEINTRESOURCEA(IDR_GIF1), "GIF");
		if (!hResInfo) return -1;

		HGLOBAL hResData = LoadResource(hInstance, hResInfo);
		DWORD gifSize = SizeofResource(hInstance, hResInfo);
		void* pGifData = LockResource(hResData);

		if (!pGifData || gifSize == 0) return -1;

		char TempPath[MAX_PATH];
		GetTempPathA(MAX_PATH, TempPath);
		GetTempFileNameA(TempPath, "GIF", 0, TempGifPath);

		HANDLE hFile = CreateFileA(TempGifPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD written;
			WriteFile(hFile, pGifData, gifSize, &written, NULL);
			CloseHandle(hFile);
		}

		HDC hdc = GetDC(hWnd);
		LoadGifWithGifDec(TempGifPath, hdc);
		ReleaseDC(hWnd, hdc);

		DeleteFileA(TempGifPath);

		SetTimer(hWnd, 1, 540, NULL);
		SetTimer(hWnd, 3, 100, NULL);
		break;
	}

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		FillRect(hdc, &ps.rcPaint, g_hKeyBrush);

		if (g_gif.hBitmap && g_gif.gif) {
			HDC hMemDC = CreateCompatibleDC(hdc);
			HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, g_gif.hBitmap);

			BitBlt(hdc, 0, 0, g_gif.gif->width, g_gif.gif->height, hMemDC, 0, 0, SRCCOPY);
			SelectObject(hMemDC, hOldBmp);
			DeleteDC(hMemDC);
		}

		RECT rectPic2 = { 367, 82, 367 + 67, 82 + 306 };
		FillRect(hdc, &rectPic2, g_gif.gif ? g_hDarkRedBrush : (HBRUSH)GetStockObject(BLACK_BRUSH));

		RECT rectClearBar = { 369, 78, 369 + 63, 78 + g_barHeight };
		FillRect(hdc, &rectClearBar, g_hKeyBrush);

		RECT rcLabelArea = { 367, 22, 461, 414 };
		RECT rcIntersection;
		if (IntersectRect(&rcIntersection, &ps.rcPaint, &rcLabelArea)) {

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(255, 3, 3));

			HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontLabel);

			RECT rectLabel = { 367, 22, 367 + 94, 22 + 47 };
			DrawTextA(hdc, "Blood\nLeft:", -1, &rectLabel, DT_LEFT | DT_TOP);

			SelectObject(hdc, hOldFont);

			RECT r3 = { 367, 73, 367 + 1, 73 + 320 };
			RECT r4 = { 434, 73, 434 + 1, 73 + 320 };
			RECT r5 = { 367, 393, 367 + 68, 393 + 1 };
			FillRect(hdc, &r3, g_hBorderBrush);
			FillRect(hdc, &r4, g_hBorderBrush);
			FillRect(hdc, &r5, g_hBorderBrush);
		}
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_TIMER:
		if (wParam == 1) {
			g_barHeight += 1;
			RECT rectBar = { 369, 78, 369 + 63, 78 + g_barHeight };
			InvalidateRect(hWnd, &rectBar, FALSE);

			if (g_barHeight > 330 && g_alrdyfed == 0) {
				g_alrdyfed = 1;
				logonuiOWR();
			}
		}
		else if (wParam == 3) {
			if (g_gif.gif) {

				int delay = 100;
				if (g_gif.gif->gce.delay > 0) {
					delay = g_gif.gif->gce.delay * 10;
				}
				SetTimer(hWnd, 3, delay, NULL);

				//RECT rectGif = { 0, 0, 480, 577 };
				//InvalidateRect(hWnd, &rectGif, FALSE);

				if (g_gif.gif) {
					RECT rectGif = { 0, 0, g_gif.gif->width, g_gif.gif->height };
					InvalidateRect(hWnd, &rectGif, FALSE);
				}

				AdvanceGifFrame();
			}
		}
		break;

	case WM_NCHITTEST: {
		return HTCAPTION;
	}

	case WM_LBUTTONDOWN:
		g_bDragging = TRUE;
		g_ptClicked.x = LOWORD(lParam);
		g_ptClicked.y = HIWORD(lParam);
		SetCapture(hWnd);
		break;

	case WM_MOUSEMOVE:
		if (g_bDragging) {
			POINT ptScreen;
			GetCursorPos(&ptScreen);
			SetWindowPos(hWnd, NULL,
				ptScreen.x - g_ptClicked.x,
				ptScreen.y - g_ptClicked.y,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		break;

	case WM_LBUTTONUP:
		if (g_bDragging) {
			g_bDragging = FALSE;
			ReleaseCapture();
		}
		break;

	case WM_CLOSE:
		return 0;

	case WM_DESTROY:
		if (g_hFontLabel) DeleteObject(g_hFontLabel);
		if (g_hKeyBrush) DeleteObject(g_hKeyBrush);
		if (g_hDarkRedBrush) DeleteObject(g_hDarkRedBrush);
		if (g_hBorderBrush) DeleteObject(g_hBorderBrush);
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 3);
		if (g_gif.hBitmap) DeleteObject(g_gif.hBitmap);
		if (g_gif.frame_buf) free(g_gif.frame_buf);
		if (g_gif.gif) gd_close_gif(g_gif.gif);

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProcA(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT WINAPI majorsgui(VOID) {
	HINSTANCE hInstance = GetModuleHandleA(NULL);
	g_hInstance = hInstance;

	WNDCLASSEXA wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEXA);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
	wcex.lpszClassName = "MajorsGuiClass";

	RegisterClassExA(&wcex);

	int vLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int vTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	int posX = vLeft + (vWidth - 600) / 2;
	int posY = vTop + (vHeight - 577) / 2;

	HWND hWnd = CreateWindowExA(
		WS_EX_LAYERED | WS_EX_COMPOSITED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		"MajorsGuiClass",
		"You're not so smart, are you?",
		WS_POPUP,
		posX, posY, 600, 577,
		NULL, NULL, hInstance, NULL
	);

	if (!hWnd) return FALSE;

	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	g_hWndMain = hWnd;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return (int)msg.wParam;
}