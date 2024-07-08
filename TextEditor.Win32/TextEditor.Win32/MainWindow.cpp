#include <MainWindow.hpp>
#include <WindowsHelper.hpp>
#include <StringUtils.hpp>

HWND MainWindow::hwnd{ NULL };
UINT MainWindow::width{ 0 }, MainWindow::height{ 0 };
TextBox MainWindow::textBox;

LRESULT CALLBACK MainWindow::Callback(const HWND hwnd,
                                      const UINT uMsg,
                                      const WPARAM wParam,
                                      const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            textBox = TextBox(width,
                              height,
                              hwnd,
                              "unifont-15.1.05.otf",
                              "Unifont");
            return 0;

        case WM_SIZE:
            width = LOWORD(lParam);
            height = HIWORD(lParam);

            SetWindowPos(textBox.GetHwnd(),
                         NULL,
                         0,
                         0,
                         width,
                         height,
                         (SWP_NOREPOSITION | SWP_NOZORDER));
            return 0;

        case WM_CLOSE:
            if (MessageBoxW(hwnd,
                            L"정말로 종료하시겠습니까?",
                            L"Are you sure you want to quit?",
                            MB_OKCANCEL) == IDOK) {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd,
                          uMsg,
                          wParam,
                          lParam);
}

MainWindow::MainWindow(void) {
    WindowsHelper::ErrorMessage(u8"Test 테스트");

    const ATOM registered{ WindowsHelper::Register(Callback, "Text Editor") };
    if (!registered) {
        WindowsHelper::ErrorMessage("Failed to register main window's class.");
    }

    hwnd = CreateWindowW((LPWSTR)MAKEINTATOM(registered),
                         L"Text Editor / 문서 편집기",
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         NULL,
                         NULL);
    if (!hwnd) {
        WindowsHelper::ErrorMessage("Failed to create main window.");
    }
}

void MainWindow::Show(void) const {
    ShowWindow(hwnd, SW_SHOW);
}