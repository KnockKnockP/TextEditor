#include <leak_checker.h>

#include <Windows.h>
#include <CommCtrl.h>

#include <main_window.h>
#include <strings.h>
#include <windows_helper.h>

#ifdef CONSOLE
#include <stdio.h>
#endif

int APIENTRY
#if UNICODE
wWinMain
#else
WinMain
#endif
(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd) {
#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_MODE_DEBUG);
#endif

#ifdef CONSOLE
    AllocConsole();
    freopen("CONOUT$", "wt", stdout);
#endif

    InitCommonControls();
    CoInitialize(NULL);

    TextEditor::Win32Api::Initialize();
    TextEditor::Strings::Initialize();

    TextEditor::MainWindow main_window;
    if (!main_window.Initialize()) {
        CoUninitialize();
        return 0;
    }

    MSG message = { 0 };
    while (GetMessage(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    CoUninitialize();

    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);
    return 0;
}
