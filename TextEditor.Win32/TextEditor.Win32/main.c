#include <Windows.h>
#include <main_window.h>

#ifdef CRT_MAIN
int WinMainCRTStartup(void)
#else
int APIENTRY
#if UNICODE
wWinMain
#else
WinMain
#endif
(const HINSTANCE hInstance, const HINSTANCE hPrevInstance, const LPTSTR lpCmdLine, const int nShowCmd)
#endif
{
    MAIN_WINDOW_initialize();
    MAIN_WINDOW_show();

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    MAIN_WINDOW_destroy();

#ifdef CRT_MAIN
    ExitProcess(0);
#else
    return 0;
#endif
}