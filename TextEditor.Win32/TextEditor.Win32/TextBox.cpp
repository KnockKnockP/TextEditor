#include <TextBox.hpp>
#include <WindowsHelper.hpp>

TextBox *TextBox::singleton{ nullptr };

LRESULT CALLBACK TextBox::Callback(HWND hwnd,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam) {
    TextBox *s{ (TextBox *)(singleton) };

    switch (uMsg) {
        case WM_CREATE:
            if (!AddFontResourceEx(s->fontFile,
                                   FR_PRIVATE,
                                   NULL)) {
                MessageBox(NULL,
                           TEXT("Failed to add main window's text box's font file."),
                           TEXT("Error"),
                           (MB_OK | MB_ICONERROR));
            }

            s->font = CreateFont(0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY,
                                 FF_DONTCARE,
                                 s->fontName);
            if (!s->font) {
                MessageBox(NULL,
                           TEXT("Failed to create main window's text box's font."),
                           TEXT("Error"),
                           (MB_OK | MB_ICONERROR));
            }
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT paintStruct{};
            HDC hdc = BeginPaint(hwnd, &paintStruct);
            if (!hdc) {
                MessageBox(NULL,
                           TEXT("Failed to fetch main window's text box's paint data."),
                           TEXT("Error"),
                           (MB_OK | MB_ICONERROR));
                return 0;
            }

            if (!SelectObject(hdc, s->font)) {
                MessageBox(NULL,
                           TEXT("Failed to select main window's text box's font."),
                           TEXT("Error"),
                           (MB_OK | MB_ICONERROR));
            }

            if (!FillRect(hdc,
                          &paintStruct.rcPaint,
                          (HBRUSH)CreateSolidBrush(RGB(255, 255, 255)))) {
                MessageBox(NULL,
                           TEXT("Failed to fill main window's text box's rectangle."),
                           TEXT("Error"),
                           (MB_OK | MB_ICONERROR));
            }

            if (!DrawText(hdc,
                          TEXT("TEST / 테스트 / テスト"),
                          -1,
                          &paintStruct.rcPaint,
                          DT_CENTER)) {
                MessageBox(NULL,
                           TEXT("Failed to fill main window's text box's text."),
                           TEXT("Error"),
                           (MB_OK | MB_ICONERROR));
            }

            EndPaint(hwnd, &paintStruct);
            return 0;
        }

        case WM_DESTROY:
            DeleteObject(s->font);
            RemoveFontResource(s->fontFile);
            return 0;
    }

    return DefWindowProc(hwnd,
                         uMsg,
                         wParam,
                         lParam);
}

TextBox::TextBox(void) {}

TextBox::TextBox(UINT width,
                 UINT height,
                 HWND parent,
                 LPCTSTR fontFile,
                 LPCTSTR fontName) {
    singleton = this;

    this->fontFile = fontFile;
    this->fontName = fontName;

    ATOM registered = WindowsHelper::Register(Callback, TEXT("TextBox"));
    if (!registered) {
        MessageBox(NULL,
                   TEXT("Failed to register main window's text box's class."),
                   TEXT("Error"),
                   (MB_OK | MB_ICONERROR));
    }

    hwnd = CreateWindow(MAKEINTATOM(registered),
                        NULL,
                        (WS_CHILD | WS_VISIBLE),
                        0,
                        0,
                        width,
                        height,
                        parent,
                        NULL,
                        NULL,
                        NULL);
    if (!hwnd) {
        MessageBox(NULL,
                   TEXT("Failed to create main window's text box."),
                   TEXT("Error"),
                   (MB_OK | MB_ICONERROR));
    }
}

HWND TextBox::GetHwnd(void) const {
    return hwnd;
}