//#define CONSOLE

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <Windows.h>
#include <CommCtrl.h>
#include <main_window.h>
#include <windows_helper.h>

#ifdef CONSOLE
#include <stdio.h>
#endif

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
#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#ifdef CONSOLE
    AllocConsole();
    freopen("CONOUT$", "wt", stdout);
#endif

    InitCommonControls();
    WINDOWS_HELPER_set_interface_type();
    WINDOWS_HELPER_interface_type = WINDOWS_HELPER_MULTIPLE_DOCUMENT_INTERFACE;
    //WINDOWS_HELPER_interface_type = WINDOWS_HELPER_SINGLE_DOCUMENT_INTERFACE;

    MAIN_WINDOW_initialize();
    MAIN_WINDOW_show();

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

#ifdef CRT_MAIN
    ExitProcess(0);
#else
    return 0;
#endif
}