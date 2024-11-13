//#define CONSOLE
#include <leak_checker.h>

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
    WINDOWS_HELPER_get_DwmExtendFrameIntoClientArea();
    WINDOWS_HELPER_get_SetLayeredWindowAttributes();
    WINDOWS_HELPER_get_DwmGetColorizationColor();
 
    //WINDOWS_HELPER_document_type = MDI;
    WINDOWS_HELPER_document_type = SDI;

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
    WINDOWS_HELPER_TOUCH(hInstance);
    WINDOWS_HELPER_TOUCH(hPrevInstance);
    WINDOWS_HELPER_TOUCH(lpCmdLine);
    WINDOWS_HELPER_TOUCH(nShowCmd);
    return 0;
#endif
}