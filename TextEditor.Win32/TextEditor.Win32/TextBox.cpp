#include <TextBox.hpp>
#include <AtomWrapper.hpp>
#include <WindowsHelper.hpp>
#include <WindowsVersions.hpp>

HWND TextBox::hwnd{ NULL };
std::string TextBox::fontFile{ "" }, TextBox::fontName{""};
HFONT TextBox::font{ NULL };
std::string TextBox::text{ "" };

LRESULT CALLBACK TextBox::Callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            const TStringContainer tFontFile{ fontFile };
            if (!AddFontResourceEx(tFontFile.GetString(),
                                   FR_PRIVATE,
                                   NULL)) {
                WindowsHelper::ErrorMessage("Failed to add main window's text box's font file.");
            }

            const TStringContainer tFontName{ fontName };
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
                              tFontName.GetString());
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

#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            if (!SelectObject(hdc, font)) {
                WindowsHelper::ErrorMessage("Failed to select main window's text box's font.");
            }
#endif

            HBRUSH brush{ CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255)) };
            if (!brush) {
                WindowsHelper::ErrorMessage("Failed to create background brush.");
                return 0;
            }

            if (!FillRect(hdc, &paintStruct.rcPaint, brush)) {
                WindowsHelper::ErrorMessage("Failed to fill main window's text box's rectangle.");
                return 0;
            }

            DeleteObject(brush);

            const TStringContainer tText{ text };
            if (!DrawText(hdc, tText.GetString(), -1, &paintStruct.rcPaint, DT_CENTER)) {
                WindowsHelper::ErrorMessage("Failed to fill main window's text box's text.");
                return 0;
            }

            EndPaint(hwnd, &paintStruct);
            return 0;
        }

        case WM_DESTROY: {
#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            DeleteObject(font);

            const TStringContainer tFontFile{ fontFile };
            RemoveFontResourceEx(tFontFile.GetString(), FR_PRIVATE, 0);
#endif
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

TextBox::TextBox(void) {}

TextBox::TextBox(const UINT width, const UINT height, const HWND parent, const std::string &fontFile, const std::string &fontName) {
    this->fontFile = fontFile;
    this->fontName = fontName;

    const AtomWrapper textBoxClass("TextBox", Callback);
    const TStringContainer tTextBoxClassName{ textBoxClass.GetName() };
    hwnd = CreateWindow(tTextBoxClassName.GetString(),
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

void TextBox::Keystroke(const WPARAM wParam) const {
    if (wParam == VK_BACK) {
        if (text.size() > 0) {
            text.pop_back();
        }
    } else {
        text += StringUtils::UnicodeToUTF8(wParam);
    }

    InvalidateRect(hwnd, 0, TRUE);
}