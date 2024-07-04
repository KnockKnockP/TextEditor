#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>

UINT x = 0, y = 0, width = 0, height = 0;

HWND textbox = NULL;

static LRESULT CALLBACK TextBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT paintStruct{};
            HDC hdc = BeginPaint(hwnd, &paintStruct);

            FillRect(hdc, &paintStruct.rcPaint, (HBRUSH)CreateSolidBrush(RGB(255, 255, 255)));
            DrawText(hdc, TEXT("TEST / 테스트"), -1, &paintStruct.rcPaint, DT_CENTER);

            EndPaint(hwnd, &paintStruct);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            const TCHAR CLASS_NAME[] = TEXT("TextBox");

            WNDCLASS wndclass{};
            wndclass.lpfnWndProc = TextBoxProc;
            wndclass.hInstance = (HINSTANCE)(GetWindowLong(hwnd, GWL_HINSTANCE));
            wndclass.lpszClassName = CLASS_NAME;
            RegisterClass(&wndclass);

            textbox = CreateWindow(CLASS_NAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, width, height, hwnd, NULL, NULL, NULL);
            return 0;
        }

        case WM_MOVE:
            x = LOWORD(lParam);
            y = HIWORD(lParam);
            return 0;

        case WM_SIZE:
            width = LOWORD(lParam);
            height = HIWORD(lParam);

            SetWindowPos(textbox, NULL, 0, 0, width, height, (SWP_NOREPOSITION | SWP_NOZORDER));
            return 0;

        case WM_CLOSE:
            if (MessageBox(hwnd, TEXT("Are you sure you want to quit?"), TEXT("정말로 종료하시겠습니까?"), MB_OKCANCEL) == IDOK) {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR pCmdLine,
                     int nCmdShow) {
    const TCHAR CLASS_NAME[] = TEXT("Text Editor");

    WNDCLASS wndclass{};
    wndclass.lpfnWndProc = MainWindowProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = CLASS_NAME;
    RegisterClass(&wndclass);

    HWND hwnd = CreateWindow(CLASS_NAME,
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
    if (hwnd == NULL) {
        return -1;
    }
    ShowWindow(hwnd, nCmdShow);

    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}