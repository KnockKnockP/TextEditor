#include <MainWindow.hpp>
#include <WindowsHelper.hpp>

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
                              TEXT("unifont-15.1.05.otf"),
                              TEXT("Unifont"));
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
            if (MessageBox(hwnd,
                TEXT("Are you sure you want to quit?"),
                TEXT("정말로 종료하시겠습니까?"),
                MB_OKCANCEL) == IDOK) {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd,
                         uMsg,
                         wParam,
                         lParam);
}

MainWindow::MainWindow(const HINSTANCE hInstance) {
    const ATOM registered = WindowsHelper::Register(Callback,
                                                    TEXT("Text Editor"),
                                                    hInstance);
    if (!registered) {
        MessageBox(NULL,
                   TEXT("Failed to register main window's class."),
                   TEXT("Error"),
                   (MB_OK | MB_ICONERROR));
    }

    hwnd = CreateWindow(MAKEINTATOM(registered),
                        TEXT("Text Editor / 문서 편집기"),
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);
    if (!hwnd) {
        MessageBox(NULL,
                   TEXT("Failed to create main window."),
                   TEXT("Error"),
                   (MB_OK | MB_ICONERROR));
    }
}

void MainWindow::Show(void) const {
    ShowWindow(hwnd, SW_SHOW);
}