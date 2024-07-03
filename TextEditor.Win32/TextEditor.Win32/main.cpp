#include <Windows.h>

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT paintStruct{};
            HDC hdc = BeginPaint(hwnd, &paintStruct);

            FillRect(hdc, &paintStruct.rcPaint, (HBRUSH)COLOR_WINDOW + 1);
            EndPaint(hwnd, &paintStruct);
            return 0;
        }

        case WM_CLOSE:
            if (MessageBox(hwnd, TEXT("Would you like to quit?"), TEXT("정말로 종료하시겠습니까?"), MB_OKCANCEL) == IDOK) {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PTSTR pCmdLine, int nCmdShow) {
    const TCHAR CLASS_NAME[] = TEXT("Text Editor");

    WNDCLASS wndclass{};
    wndclass.lpfnWndProc = WindowProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = CLASS_NAME;
    RegisterClass(&wndclass);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, TEXT("Text Editor / 문서 편집기"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
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