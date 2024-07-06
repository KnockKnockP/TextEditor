#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>
#include <MainWindow.hpp>

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine,
                     _In_ int nShowCmd) {
    MainWindow mainWindow;
    mainWindow.Show();

    MSG msg{};
    while (GetMessage(&msg,
                      NULL,
                      0,
                      0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}