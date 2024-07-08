#include <TextBox.hpp>
#include <WindowsHelper.hpp>
#include <StringUtils.hpp>

HWND TextBox::hwnd{ NULL };
std::string TextBox::fontFile{ "" }, TextBox::fontName{""};
HFONT TextBox::font{ NULL };

LRESULT CALLBACK TextBox::Callback(const HWND hwnd,
                                   const UINT uMsg,
                                   const WPARAM wParam,
                                   const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
#if _WIN32_WINNT > _WIN32_WINNT_NT4
            const std::wstring wFontFile{ StringUtils::ToUTF16(fontFile)};

            if (!AddFontResourceEx(wFontFile.c_str(),
                                   FR_PRIVATE,
                                   NULL)) {
                WindowsHelper::ErrorMessage("Failed to add main window's text box's font file.");
            }

            const std::wstring wFontName{ StringUtils::ToUTF16(fontName) };
            font = CreateFont(0,
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
                              ANTIALIASED_QUALITY,
                              FF_DONTCARE,
                              wFontName.c_str());
            if (!font) {
                WindowsHelper::ErrorMessage("Failed to create main window's text box's font.");
            }
#endif
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT paintStruct{};
            HDC hdc = BeginPaint(hwnd, &paintStruct);
            if (!hdc) {
                WindowsHelper::ErrorMessage("Failed to fetch main window's text box's paint data.");
                return 0;
            }

#if _WIN32_WINNT > _WIN32_WINNT_NT4
            if (!SelectObject(hdc, font)) {
                WindowsHelper::ErrorMessage("Failed to select main window's text box's font.");
            }
#endif

            if (!FillRect(hdc,
                          &paintStruct.rcPaint,
                          (HBRUSH)CreateSolidBrush(RGB(255, 255, 255)))) {
                WindowsHelper::ErrorMessage("Failed to fill main window's text box's rectangle.");
            }

            const std::wstring text{ StringUtils::ToUTF16("Test / 테스트 / テスト") };
            if (!DrawText(hdc,
                          text.c_str(),
                          -1,
                          &paintStruct.rcPaint,
                          DT_CENTER)) {
                WindowsHelper::ErrorMessage("Failed to fill main window's text box's text.");
            }

            EndPaint(hwnd, &paintStruct);
            return 0;
        }

        case WM_DESTROY: {
#if _WIN32_WINNT > _WIN32_WINNT_NT4
            DeleteObject(font);

            const std::wstring wFontFile{ StringUtils::ToUTF16(fontFile) };
            RemoveFontResourceEx(wFontFile.c_str(),
                                 FR_PRIVATE,
                                 0);
#endif
            return 0;
        }
    }

    return DefWindowProc(hwnd,
                         uMsg,
                         wParam,
                         lParam);
}

TextBox::TextBox(void) {}

TextBox::TextBox(const UINT width,
                 const UINT height,
                 const HWND parent,
                 const std::string &fontFile,
                 const std::string &fontName) {
    this->fontFile = fontFile;
    this->fontName = fontName;

    ATOM registered{ WindowsHelper::Register(Callback, "TextBox") };
    if (!registered) {
        WindowsHelper::ErrorMessage("Failed to register main window's text box's class.");
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
        WindowsHelper::ErrorMessage("Failed to create main window's text box.");
    }
}

HWND TextBox::GetHwnd(void) const {
    return hwnd;
}