#include <Windows.h>
#include <MainWindow.hpp>

int APIENTRY
#if UNICODE
wWinMain
#else
WinMain
#endif
(const HINSTANCE hInstance, const HINSTANCE hPrevInstance, const LPTSTR lpCmdLine, const int nShowCmd) {
    const StringUtilities::UTF8String a{ TEXT("안녕") };
    MessageBox(NULL, a.GetWindowsTString().get(), TEXT("Hello"), MB_OK);

    const MainWindow mainWindow;
    mainWindow.Show();

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}