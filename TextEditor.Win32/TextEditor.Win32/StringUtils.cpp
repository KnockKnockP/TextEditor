#include <StringUtils.hpp>
#include <WindowsHelper.hpp>
#include <WindowsVersions.hpp>

#if WINDOWS_VERSION <= _WIN32_WINNT_NT35
static void ToUCS2(const std::vector<char> &UTF8, LPWSTR UCS2, size_t UCS2Size) {
    size_t UTF8Index{ 0 }, UCS2Index{ 0 };

    while (UTF8Index < UTF8.size() && UCS2Index < UCS2Size - 1) {
        unsigned char c{ (unsigned char)UTF8[UTF8Index++] };
        unsigned int codepoint{ 0 };

        if (c <= 0x7F) {
            //ASCII
            codepoint = c;
        } else if (c <= 0xDF) {
            //2 bytes
            if (UTF8Index >= UTF8.size()) {
                //Replacement character for incomplete sequence.
                codepoint = 0xFFFD;
            } else {
                codepoint = ((c & 0x1F) << 6) | (UTF8[UTF8Index++] & 0x3F);
            }
        } else if (c <= 0xEF) {
            //3 bytes
            if (UTF8Index + 1 >= UTF8.size()) {
                codepoint = 0xFFFD;
            } else {
                codepoint = ((c & 0x0F) << 12) | ((UTF8[UTF8Index++] & 0x3F) << 6) | (UTF8[UTF8Index++] & 0x3F);
            }
        } else {
            //4 bytes or higher are not supported in UCS-2.
            codepoint = 0xFFFD;

            while (UTF8Index < UTF8.size() && (UTF8[UTF8Index] & 0xC0) == 0x80) {
                UTF8Index++;
            }
        }

        if (codepoint > 0xFFFF) {
            codepoint = 0xFFFD;
        }

        UCS2[UCS2Index++] = static_cast<WCHAR>(codepoint);
    }

    while (UCS2Index < UCS2Size - 1 && UTF8Index < UTF8.size()) {
        UCS2[UCS2Index++] = 0xFFFD;
    }

    UCS2[UCS2Index] = L'\0';
}
#endif

void UnifiedString::PushAllBytes(const std::vector<char> bytes) {
    for (char byte : bytes) {
        this->bytes.push_back(byte);
    }
}

void UnifiedString::PushUnicode(const uint32_t unicode) {
    if (unicode <= 0x7F) {
        bytes.push_back(static_cast<char>(unicode));
    } else if (unicode <= 0x7FF) {
        bytes.push_back(static_cast<char>(0xC0 | (unicode >> 6)));
        bytes.push_back(static_cast<char>(0x80 | (unicode & 0x3F)));
    } else if (unicode <= 0xFFFF) {
        bytes.push_back(static_cast<char>(0xE0 | (unicode >> 12)));
        bytes.push_back(static_cast<char>(0x80 | ((unicode >> 6) & 0x3F)));
        bytes.push_back(static_cast<char>(0x80 | (unicode & 0x3F)));
    } else {
        bytes.push_back(static_cast<char>(0xF0 | (unicode >> 18)));
        bytes.push_back(static_cast<char>(0x80 | ((unicode >> 12) & 0x3F)));
        bytes.push_back(static_cast<char>(0x80 | ((unicode >> 6) & 0x3F)));
        bytes.push_back(static_cast<char>(0x80 | (unicode & 0x3F)));
    }
}

void UnifiedString::PushUnicode(const LPTSTR string) {
#if UNICODE
    size_t i{ 0 };
    while (string[i]) {
        PushUnicode(string[i++]);
    }
#else
    const size_t wideCharacters{ (size_t)MultiByteToWideChar(CP_UTF8, 0, string, -1, nullptr, 0) };
    LPWSTR wideString{ new WCHAR[wideCharacters] };

    MultiByteToWideChar(CP_UTF8, 0, string, -1, wideString, wideCharacters);

    for (size_t i{ 0 }; i < wideCharacters; ++i) {
        PushUnicode(wideString[i]);
    }

    delete[] wideString;
#endif
}

UnifiedString::UnifiedString(void) {}

UnifiedString::UnifiedString(const std::vector<char> &bytes) {
    PushAllBytes(bytes);
}

UnifiedString::UnifiedString(const char character) {
    PushUnicode(character);
}

UnifiedString::UnifiedString(const uint32_t unicode) {
    PushUnicode(unicode);
}

UnifiedString::UnifiedString(const LPTSTR string) {
    PushUnicode(string);
}

UnifiedString::UnifiedString(const UnifiedString &other) {
    PushAllBytes(other.GetBytes());
}

UnifiedString UnifiedString::operator+(const UnifiedString other) {
    PushAllBytes(other.bytes);
    return UnifiedString(bytes);
}

UnifiedString UnifiedString::operator+(const LPTSTR other) {
    PushUnicode(other);
    return UnifiedString(bytes);
}

void UnifiedString::operator+=(const uint32_t other) {
    PushUnicode(other);
}

void UnifiedString::operator+=(const LPSTR other) {
    const size_t wideCharacters{ (size_t)MultiByteToWideChar(CP_ACP, 0, other, -1, nullptr, 0) };
    if (!wideCharacters) {
        return;
    }

    LPWSTR wideString{ new WCHAR[wideCharacters] };

    MultiByteToWideChar(CP_ACP, 0, other, -1, wideString, wideCharacters);

    for (size_t i{ 0 }; i < wideCharacters - 1; ++i) {
        PushUnicode(wideString[i]);
    }

    delete[] wideString;
}

void UnifiedString::RemoveLastCharacter(void) {
    if (bytes.size() > 0) {
        bytes.pop_back();
    }
}

const std::vector<char> UnifiedString::GetBytes(void) const {
    return bytes;
}

LPTSTR UnifiedString::GetWindowsString(void) {
    //Convert UTF-8 bytes to wide string.
#if WINDOWS_VERSION <= _WIN32_WINNT_NT35
    const size_t UCS2Size{ bytes.size() * 2 + 1 };
    LPWSTR UCS2String{ new WCHAR[UCS2Size] };
    memset(UCS2String, 0, sizeof(WCHAR) * UCS2Size);

    ToUCS2(bytes, UCS2String, UCS2Size);
#endif

    const size_t wideCharacters{
#if WINDOWS_VERSION <= _WIN32_WINNT_NT35
        wcslen(UCS2String)
#else
        (size_t)MultiByteToWideChar(CP_UTF8, 0, bytes.data(), bytes.size(), nullptr, 0)
#endif
    };

    LPWSTR wideString{ new WCHAR[wideCharacters + 1] };

#if WINDOWS_VERSION <= _WIN32_WINNT_NT35
    wcscpy(wideString, UCS2String);
    delete[] UCS2String;
#else
    MultiByteToWideChar(CP_UTF8, 0, bytes.data(), bytes.size(), wideString, wideCharacters);
#endif
    wideString[wideCharacters] = L'\0';

    void *string{ wideString };

#ifndef UNICODE
    //Convert wide string to ANSI string.
    const int ansiCharacters{ WideCharToMultiByte(CP_ACP, 0, wideString, -1, nullptr, 0, NULL, NULL) };
    LPSTR ansiString{ new CHAR[ansiCharacters] };

    WideCharToMultiByte(CP_ACP, 0, wideString, -1, ansiString, ansiCharacters, NULL, NULL);
    string = ansiString;
    delete[] wideString;
#endif

    if (windowsString) {
        delete[] windowsString;
    }
    windowsString = (TCHAR *)string;
    
    return windowsString;
}

UnifiedString::~UnifiedString(void) {
    if (windowsString) {
        delete[] windowsString;
        windowsString = nullptr;
    }
}

UnifiedString operator+(const LPTSTR string, const UnifiedString other) {
    return UnifiedString{ UnifiedString{ string } + other };
}