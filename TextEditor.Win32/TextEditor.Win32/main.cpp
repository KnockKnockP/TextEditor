#include <Windows.h>
#include <MainWindow.hpp>

int APIENTRY
#if UNICODE
wWinMain
#else
WinMain
#endif
(const HINSTANCE hInstance, const HINSTANCE hPrevInstance, const LPTSTR lpCmdLine, const int nShowCmd) {
    const MainWindow mainWindow;
    mainWindow.Show();

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}