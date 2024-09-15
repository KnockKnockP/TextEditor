#include <MainWindow.hpp>
#include <StringUtils.hpp>
#include <AtomWrapper.hpp>
#include <WindowsHelper.hpp>

HWND MainWindow::hwnd{ NULL };
UINT MainWindow::width{ 0 }, MainWindow::height{ 0 };
TextBox MainWindow::textBox;

LRESULT CALLBACK MainWindow::Callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            textBox = TextBox(width, height, hwnd, "unifont-15.1.05.otf", "Unifont");
            return 0;

        case WM_CHAR:
            textBox.Keystroke(wParam);
            break;

        case WM_SIZE:
            width = LOWORD(lParam);
            height = HIWORD(lParam);

            SetWindowPos(textBox.GetHwnd(), NULL, 0, 0, width, height, (SWP_NOREPOSITION | SWP_NOZORDER));
            return 0;

        case WM_CLOSE: {
            const TStringContainer tText{ "정말로 종료하시겠습니까?" }, tCaption{ "Are you sure you want to quit?" };

            if (MessageBox(hwnd, tText.GetString(), tCaption.GetString(), MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
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
    const AtomWrapper mainWindowClass("Text Editor", Callback);
    const TStringContainer tMainWindowClassName{ mainWindowClass.GetName() }, tWindowTitle{ "Text Editor / 문서 편집기" };
    hwnd = CreateWindow(tMainWindowClassName.GetString(),
                        tWindowTitle.GetString(),
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