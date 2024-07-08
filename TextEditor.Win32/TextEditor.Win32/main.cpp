#include <Windows.h>
#include <MainWindow.hpp>

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd) {
    const MainWindow mainWindow(NULL);
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