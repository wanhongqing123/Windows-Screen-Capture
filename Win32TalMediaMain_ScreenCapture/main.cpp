
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <DuplicatorCapture.h>
#include <versionhelpers.h>
#include <dwmapi.h>

#include <signal.h>
typedef  HRESULT(WINAPI* fn_SetProcessDpiAwareness)(int);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    //re
	switch (msg) {
	case WM_SIZE:
		break;
	case WM_DPICHANGED:
	{
		DWORD dw = wParam;
		int i = HIWORD(dw);
		int j = LOWORD(dw);
	    printf("%d  %d\n", i,j);
	}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


static void updateFramebufferTransparency(HWND handle)
{
	BOOL enabled;

	if (!IsWindowsVistaOrGreater())
		return;

	if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled)
	{
		HRGN region = CreateRectRgn(0, 0, -1, -1);
		DWM_BLURBEHIND bb = { 0 };
		bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
		bb.hRgnBlur = region;
		bb.fEnable = TRUE;

		if (SUCCEEDED(DwmEnableBlurBehindWindow(handle, &bb)))
		{
			// Decorated windows don't repaint the transparent background
			// leaving a trail behind animations
			// HACK: Making the window layered with a transparency color key
			//       seems to fix this.  Normally, when specifying
			//       a transparency color key to be used when composing the
			//       layered window, all pixels painted by the window in this
			//       color will be transparent.  That doesn't seem to be the
			//       case anymore, at least when used with blur behind window
			//       plus negative region.
			LONG exStyle = GetWindowLongW(handle, GWL_EXSTYLE);
			exStyle |= WS_EX_LAYERED;
			SetWindowLongW(handle, GWL_EXSTYLE, exStyle);

			// Using a color key not equal to black to fix the trailing
			// issue.  When set to black, something is making the hit test
			// not resize with the window frame.
			SetLayeredWindowAttributes(handle,
				RGB(255, 0, 255), 255, LWA_COLORKEY);
		}

		DeleteObject(region);
	}
	else
	{
		LONG exStyle = GetWindowLongW(handle, GWL_EXSTYLE);
		exStyle &= ~WS_EX_LAYERED;
		SetWindowLongW(handle, GWL_EXSTYLE, exStyle);
		RedrawWindow(handle, NULL, NULL,
			RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	}
}


static int i = 0;
static int ii = 0;
static bool l = false;
LRESULT __stdcall HookProc(int code, WPARAM wParam, LPARAM lParam) {
	if (l == false) {
		l = true;
	}
	else {
		i++;
	}
	ii++;
	auto test = CallNextHookEx(NULL, code, wParam, lParam);
	i--;
	if (i == 0) {
		printf("afdasdfasdfasdfasdfasdf========%d\n", ii);
		ii = 0;
	}
	return test;
}

int main(int argc, char** argv){
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1) {
		SDL_Log("SDL Init Success \n");
		return 0;
	}

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), /*CS_CLASSDC*/CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, WndProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example",
		NULL };
	RegisterClassEx(&wc);
	
	HWND hwnd = CreateWindow(wc.lpszClassName, L"DirectX11Capture", WS_OVERLAPPEDWINDOW,
		100, 100, 1280, 720, NULL, NULL, wc.hInstance, NULL);

	SDL_Rect rect;
	int num = SDL_GetNumVideoDisplays();
	for (int i = 0; i < num; ++i) {
		SDL_GetDisplayBounds(0, &rect);
	}
	DuplicatorCapture* pCapture = new DuplicatorCapture(hwnd, true);
	bool bInit = pCapture->Initialize();
	if (!bInit) {
		printf("DuplicatorCapture Init Failed\n ");
		SDL_Delay(3000);
		goto exitfalg;
	}

	pCapture->SetCaptureOutParam(rect.w, rect.h);
	rect.x = 0;
	rect.y = 0;
	rect.w = rect.w;
	rect.h = rect.h;
	pCapture->SetCaptureRect(rect);
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (true){
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Uint32 start = SDL_GetTicks();
		if (pCapture->CaptureOneFrame()) {
			int width = 0;
			int height = 0;
			int stride = 0;
			uint8_t* ptr = NULL;
			pCapture->RenderToWindow();
		}
		Sleep(30);
	}
exitfalg:
	pCapture->UnInitialize();
	delete pCapture;
	DestroyWindow(hwnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
    return(0);
}




