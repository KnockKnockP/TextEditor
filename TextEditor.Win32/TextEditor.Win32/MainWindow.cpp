#include <MainWindow.hpp>
#include <WindowsHelper.hpp>

MainWindow *MainWindow::singleton{ nullptr };

LRESULT CALLBACK MainWindow::Callback(HWND hwnd,
                                      UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam) {
    MainWindow *s{ MainWindow::singleton };

    switch (uMsg) {
        case WM_CREATE:
            s->textBox = TextBox(s->width,
                                 s->height,
                                 hwnd,
                                 TEXT(".\\KurintoMono-Rg.ttf"),
                                 TEXT("Kurinto Mono"));
            return 0;

        case WM_SIZE:
            s->width = LOWORD(lParam);
            s->height = HIWORD(lParam);

            SetWindowPos(s->textBox.GetHwnd(),
                         NULL,
                         0,
                         0,
                         s->width,
                         s->height,
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

MainWindow::MainWindow(void) {
    singleton = this;

    const ATOM registered = WindowsHelper::Register(Callback, TEXT("Text Editor"));
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
                        NULL,
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