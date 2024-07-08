#include <Windows.h>
#include <MainWindow.hpp>
#include <StringUtilities.hpp>

int APIENTRY WinMain(const HINSTANCE hInstance,
                     const HINSTANCE hPrevInstance,
                     const LPSTR lpCmdLine,
                     const int nShowCmd) {
    MessageBoxW(NULL, L"안녕 Hello", L"Hello 안녕", MB_OK);
    return 0;
    const MainWindow mainWindow;
    mainWindow.Show();

    MSG msg = {};
    while (GetMessage(&msg,
                      NULL,
                      0,
                      0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}