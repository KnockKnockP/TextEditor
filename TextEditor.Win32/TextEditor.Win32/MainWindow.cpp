#include <MainWindow.hpp>
#include <StringUtils.hpp>
#include <AtomWrapper.hpp>
#include <WindowsHelper.hpp>

HWND MainWindow::hwnd{ nullptr };
UINT MainWindow::width{ 0 }, MainWindow::height{ 0 };
TextBox MainWindow::textBox;

std::vector<char> params;

LRESULT CALLBACK MainWindow::Callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            textBox = TextBox(width, height, hwnd, UT("unifont-15.1.05.otf"), UT("Unifont"));
            return 0;

        case WM_CHAR:
            textBox.Keystroke(wParam);
            break;

        case WM_SIZE:
            width = LOWORD(lParam);
            height = HIWORD(lParam);

            SetWindowPos(textBox.GetHwnd(), nullptr, 0, 0, width, height, (SWP_NOREPOSITION | SWP_NOZORDER));
            return 0;

        case WM_CLOSE: {
            if (MessageBox(hwnd,
                           UnifiedString{ UT("정말로 종료하시겠습니까?") }.GetWindowsString(),
                           UnifiedString{ UT("Are you sure you want to quit?") }.GetWindowsString(),
                           MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
                DestroyWindow(hwnd);
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

MainWindow::MainWindow(void) {
    const AtomWrapper mainWindowClass(UT("Text Editor"), Callback);
    hwnd = CreateWindow(mainWindowClass.GetName().GetWindowsString(),
                        UnifiedString{ UT("Text Editor / 문서 편집기") }.GetWindowsString(),
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr);
    if (!hwnd) {
        WindowsHelper::ErrorMessage(UT("Failed to create main window."));
    }
}

void MainWindow::Show(void) const {
    ShowWindow(hwnd, SW_SHOW);
}