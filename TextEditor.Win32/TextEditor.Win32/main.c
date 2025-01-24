//#define CONSOLE
#include <leak_checker.h>

#include <Windows.h>
#include <CommCtrl.h>
#include <strings.h>
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
(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
#endif
{
#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_MODE_DEBUG);
#endif

#ifdef CONSOLE
    AllocConsole();
    freopen("CONOUT$", "wt", stdout);
#endif

    InitCommonControls();
    CoInitialize(NULL);
    WINDOWS_HELPER_set_types();
    WINDOWS_HELPER_get_AddFontResourceEx();
    WINDOWS_HELPER_get_RemoveFontResourceEx();
    WINDOWS_HELPER_get_ImmGetContext();
    WINDOWS_HELPER_get_ImmGetCompositionString();
    WINDOWS_HELPER_get_ImmReleaseContext();
    WINDOWS_HELPER_get_DwmEnableBlurBehindWindow();
    WINDOWS_HELPER_get_DwmGetColorizationColor();
    STRINGS_initialize();
 
    WINDOWS_HELPER_document_type = MDI;
    //WINDOWS_HELPER_document_type = SDI;

    MAIN_WINDOW_initialize();
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();

#ifdef CRT_MAIN
    ExitProcess(0);
#else
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);
    return 0;
#endif
}