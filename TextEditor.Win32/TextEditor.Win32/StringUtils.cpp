#include <StringUtils.hpp>
#include <WindowsHelper.hpp>
#include <WindowsVersions.hpp>

std::string StringUtils::UnicodeToUTF8(const WPARAM codePrint) {
    std::string utf8{ "" };

    if (codePrint <= 0x7F) {
        utf8.push_back(static_cast<char>(codePrint));
    }
    else if (codePrint <= 0x7FF) {
        utf8.push_back(static_cast<char>(0xC0 | (codePrint >> 6)));
        utf8.push_back(static_cast<char>(0x80 | (codePrint & 0x3F)));
    } else if (codePrint <= 0xFFFF) {
        utf8.push_back(static_cast<char>(0xE0 | (codePrint >> 12)));
        utf8.push_back(static_cast<char>(0x80 | ((codePrint >> 6) & 0x3F)));
        utf8.push_back(static_cast<char>(0x80 | (codePrint & 0x3F)));
    } else {
        utf8.push_back(static_cast<char>(0xF0 | (codePrint >> 18)));
        utf8.push_back(static_cast<char>(0x80 | ((codePrint >> 12) & 0x3F)));
        utf8.push_back(static_cast<char>(0x80 | ((codePrint >> 6) & 0x3F)));
        utf8.push_back(static_cast<char>(0x80 | (codePrint & 0x3F)));
    }

    return utf8;
}

TStringContainer::TStringContainer(const int size) {
    string = new TCHAR[size];
    if (string == nullptr) {
        WindowsHelper::ErrorMessage("Failed to allocate string.");
        return;
    }

    this->size = size;
}

#if WINDOWS_VERSION <= _WIN32_WINNT_NT35
static void ToUCS2(const std::string &utf8, LPWSTR ucs2, size_t ucs2_size) {
    size_t utf8Index{ 0 }, ucs2Index{ 0 };

    while (utf8Index < utf8.length() && ucs2Index < ucs2_size - 1) {
        unsigned char c{ (unsigned char)utf8[utf8Index++] };
        unsigned int codepoint{ 0 };

        if (c <= 0x7F) {
            //ASCII
            codepoint = c;
        } else if (c <= 0xDF) {
            //2 bytes
            if (utf8Index >= utf8.length()) {
                //Replacement character for incomplete sequence.
                codepoint = 0xFFFD;
            } else {
                codepoint = ((c & 0x1F) << 6) | (utf8[utf8Index++] & 0x3F);
            }
        } else if (c <= 0xEF) {
            //3 bytes
            if (utf8Index + 1 >= utf8.length()) {
                codepoint = 0xFFFD;
            } else {
                codepoint = ((c & 0x0F) << 12) | ((utf8[utf8Index++] & 0x3F) << 6) | (utf8[utf8Index++] & 0x3F);
            }
        } else {
            //4 bytes or higher are not supported in UCS-2.
            codepoint = 0xFFFD;

            while (utf8Index < utf8.length() && (utf8[utf8Index] & 0xC0) == 0x80) {
                utf8Index++;
            }
        }

        if (codepoint > 0xFFFF) {
            codepoint = 0xFFFD;
        }

        ucs2[ucs2Index++] = static_cast<wchar_t>(codepoint);
    }

    while (ucs2Index < ucs2_size - 1 && utf8Index < utf8.length()) {
        ucs2[ucs2Index++] = 0xFFFD;
    }

    ucs2[ucs2Index] = L'\0';
}
#endif

TStringContainer::TStringContainer(const std::string &utf8) {
    size_t wideSize = utf8.length() * 2 + 1;

    LPWSTR wide{ new WCHAR[wideSize] };
    if (wide == nullptr) {
        WindowsHelper::ErrorMessage("Failed to allocate string.");
        return;
    }

#if WINDOWS_VERSION <= _WIN32_WINNT_NT35
    ToUCS2(utf8, wide, wideSize);
#else
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide, wideSize);
#endif

#ifdef UNICODE
    size = wcslen(wide) + 1;

    string = new TCHAR[size];
    if (string == nullptr) {
        WindowsHelper::ErrorMessage("Failed to allocate string.");
        return;
    }

    wcscpy(string, wide);
#else
    int ansiSize{ WideCharToMultiByte(CP_ACP, 0, wide, -1, NULL, 0, NULL, NULL) };

    LPSTR ansi{ new CHAR[ansiSize] };
    if (ansi == nullptr) {
        WindowsHelper::ErrorMessage("Failed to allocate string.");
        return;
    }

    WideCharToMultiByte(CP_ACP, 0, wide, -1, ansi, ansiSize, NULL, NULL);

    string = new CHAR[ansiSize];
    if (string == nullptr) {
        WindowsHelper::ErrorMessage("Failed to allocate string.");
        return;
    }

    memcpy(string, ansi, ansiSize);

    delete[] ansi;
#endif

    delete[] wide;
}

void TStringContainer::operator+=(const TCHAR character) {
    if (size == 0) {
        //This should never happen but just in case.
        size = 1;
    }

    LPTSTR newString = new TCHAR[size + 1];
    if (newString == nullptr) {
        WindowsHelper::ErrorMessage("Failed to allocate string.");
        return;
    }

#if UNICODE
    wcscpy(newString, string);
#else
    memcpy(newString, string, size * sizeof(TCHAR));
#endif

    newString[size - 1] = character;
    newString[size++] = TEXT('\0');

    delete[] string;
    string = newString;
}

void TStringContainer::RemoveLastCharacter(void) {
    if (size <= 1) {
        return;
    }

    LPTSTR newString = new TCHAR[size - 1];
    if (newString == nullptr) {
        WindowsHelper::ErrorMessage("Failed to allocate string.");
        return;
    }

    memcpy(newString, string, sizeof(TCHAR) * (size - 1));
    newString[--size - 1] = TEXT('\0');

    delete[] string;
    string = newString;
}

LPTSTR TStringContainer::GetString(void) const {
    return string;
}

TStringContainer::~TStringContainer(void) {
    delete[] string;
    string = NULL;
}

/*
LRESULT CALLBACK TextBox::Callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        // ... existing cases ...

        case WM_IME_COMPOSITION:
            if (lParam & GCS_RESULTSTR) {
                HIMC hIMC = ImmGetContext(hwnd);
                if (hIMC) {
                    LONG bufLen = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
                    if (bufLen > 0) {
                        TCHAR* buf = new TCHAR[(bufLen / sizeof(TCHAR)) + 1];
                        ImmGetCompositionString(hIMC, GCS_RESULTSTR, buf, bufLen);
                        buf[bufLen / sizeof(TCHAR)] = TEXT('\0');

                        // Append the completed IME string to our text
                        for (int i = 0; buf[i] != TEXT('\0'); ++i) {
                            text += buf[i];
                        }

                        delete[] buf;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    ImmReleaseContext(hwnd, hIMC);
                }
            }
            return 0;

        case WM_IME_CHAR:
            // We handle the composition in WM_IME_COMPOSITION, so we can ignore this
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
*/

/*
void TextBox::Keystroke(const WPARAM wParam) const {
    if (wParam == VK_BACK) {
        text.RemoveLastCharacter();
        InvalidateRect(hwnd, NULL, TRUE);
    } else if (wParam < 256) {  // Only handle ASCII characters here
        const TCHAR character{ (TCHAR)wParam };
        text += character;
        InvalidateRect(hwnd, NULL, TRUE);
    }
    // Non-ASCII characters (including Korean) will be handled by WM_IME_COMPOSITION
}
*/

/*
#include <imm.h>
#pragma comment(lib, "imm32.lib")
*/

/*
TextBox::TextBox(const UINT width, const UINT height, const HWND parent, const std::string &fontFile, const std::string &fontName) {
    // ... existing code ...

    if (hwnd) {
        // Enable IME for this window
        ImmAssociateContext(hwnd, NULL);
    }
}
*/