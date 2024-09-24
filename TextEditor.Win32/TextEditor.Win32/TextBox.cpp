#include <vector>
#include <TextBox.hpp>
#include <AtomWrapper.hpp>
#include <WindowsHelper.hpp>
#include <WindowsVersions.hpp>

HWND TextBox::hwnd{ nullptr };
StringUtilities::UTF8String TextBox::fontFile{}, TextBox::fontName{}, TextBox::text{};
#ifndef UNICODE
CHAR TextBox::multibyteBuffer[3] = { 0, 0, 0 };
#endif
HFONT TextBox::font{ nullptr };
size_t TextBox::fontWidth{ 16 }, TextBox::fontHeight{ 16 }, TextBox::wrappedLineCount{ 0 };
WORD TextBox::width{ 0 }, TextBox::height{ 0 };

LRESULT CALLBACK TextBox::Callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            if (!AddFontResourceEx(fontFile.GetWindowsTString().get(), FR_PRIVATE, nullptr)) {
                WindowsHelper::ErrorMessage(ST("Failed to add main window's text box's font file."));
            }

            font = CreateFont(fontHeight,
                              fontWidth,
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
                              fontName.GetWindowsTString().get());
            if (!font) {
                WindowsHelper::ErrorMessage(ST("Failed to create main window's text box's font."));
            }
#endif

            if (!CreateCaret(hwnd, nullptr, 2, fontHeight)) {
                WindowsHelper::ErrorMessage(ST("Failed to create main window's text box's caret."));
            }
            ShowCaret(hwnd);
            return 0;
        }

        case WM_SIZE:
            width = LOWORD(lParam);
            height = HIWORD(lParam);
            break;

        case WM_CHAR:
            if (wParam == VK_BACK) {
                text.RemoveLastCharacter();
            } else {
#ifndef UNICODE
                if (IsDBCSLeadByte(wParam)) {
                    if (!multibyteBuffer[0]) {
                        multibyteBuffer[0] = wParam;
                    } else {
                        multibyteBuffer[1] = wParam;
                        text += multibyteBuffer;

                        memset(multibyteBuffer, 0, sizeof(CHAR) * 3);
                    }
                } else {
#endif
                    WPARAM converted{ wParam };
                    if (converted == '\r') {
                        converted = '\n';
                    }
                    text += converted;
#ifndef UNICODE
                }
#endif
            }

            InvalidateRect(hwnd, nullptr, true);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT paintStruct{};
            HDC hdc = BeginPaint(hwnd, &paintStruct);
            if (!hdc) {
                WindowsHelper::ErrorMessage(ST("Failed to fetch main window's text box's paint data."));
                return 0;
            }

#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            if (!SelectObject(hdc, font)) {
                WindowsHelper::ErrorMessage(ST("Failed to select main window's text box's font."));
            }
#endif

            HBRUSH brush{ CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255)) };
            if (!brush) {
                WindowsHelper::ErrorMessage(ST("Failed to create background brush."));
                return 0;
            }

            if (!FillRect(hdc, &paintStruct.rcPaint, brush)) {
                WindowsHelper::ErrorMessage(ST("Failed to fill main window's text box's rectangle."));
                return 0;
            }

            DeleteObject(brush);

            const std::unique_ptr<const WCHAR[]> string{ text.GetUTF16String() };

            size_t i{ 0 }, newLines{ 0 }, startOfLastLine{ 0 };
            while (string[i]) {
                if (string[i++] == L'\n') {
                    ++newLines;
                    startOfLastLine = i;
                }
            }

            i = startOfLastLine;
            std::vector<WCHAR> charactersInLastLine{};
            while (string[i]) {
                charactersInLastLine.push_back(string[i++]);
            }
            charactersInLastLine.push_back(0);
            const StringUtilities::UTF8String lastLine{ charactersInLastLine.data() };

            RECT textSize{};
            DrawText(hdc, lastLine.GetWindowsTString().get(), -1, &textSize, DT_CALCRECT | DT_EXPANDTABS);

            SetCaretPos(textSize.right, newLines * fontHeight);

            if (!DrawText(hdc, text.GetWindowsTString().get(), -1, &paintStruct.rcPaint, DT_EXPANDTABS)) {
                WindowsHelper::ErrorMessage(ST("Failed to fill main window's text box's text."));
                return 0;
            }

            EndPaint(hwnd, &paintStruct);
            return 0;
        }

        case WM_DESTROY: {
#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            DeleteObject(font);
            RemoveFontResourceEx(fontFile.GetWindowsTString().get(), FR_PRIVATE, 0);
#endif
            HideCaret(hwnd);
            DestroyCaret();
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

TextBox::TextBox(void) {}

TextBox::TextBox(const UINT width, const UINT height, const HWND parent, StringUtilities::UTF8String fontFile, StringUtilities::UTF8String fontName) {
    this->fontFile = fontFile;
    this->fontName = fontName;

    const AtomWrapper textBoxClass(ST("TextBox"), Callback);
    hwnd = CreateWindow(textBoxClass.GetName().GetWindowsTString().get(),
                        nullptr,
                        WS_CHILD | WS_VISIBLE,
                        0,
                        0,
                        width,
                        height,
                        parent,
                        nullptr,
                        nullptr,
                        nullptr);
    if (!hwnd) {
        WindowsHelper::ErrorMessage(ST("Failed to create main window's text box."));
    }
}

HWND TextBox::GetHwnd(void) const {
    return hwnd;
}

void TextBox::Keystroke(const WPARAM wParam) const {
    if (wParam == VK_BACK) {
        text.RemoveLastCharacter();
    } else {
#ifndef UNICODE
        if (IsDBCSLeadByte(wParam)) {
            if (!multibyteBuffer[0]) {
                multibyteBuffer[0] = wParam;
            } else {
                multibyteBuffer[1] = wParam;
                text += multibyteBuffer;

                memset(multibyteBuffer, 0, sizeof(CHAR) * 3);
            }
        } else {
#endif
            text += wParam;
#ifndef UNICODE
        }
#endif
    }

    InvalidateRect(hwnd, nullptr, true);
}