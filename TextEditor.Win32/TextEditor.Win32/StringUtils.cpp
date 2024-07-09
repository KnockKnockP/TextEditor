#include <StringUtils.hpp>

TStringContainer::TStringContainer(const int size) {
    string = new TCHAR[size];
}

//Calude ahh generated function.
static void ToUCS2(const std::string &utf8,
                   LPWSTR ucs2,
                   size_t ucs2_size) {
    size_t utf8Index{ 0 }, ucs2Index{ 0 };

    while ((utf8Index < utf8.length()) && (ucs2Index < (ucs2_size - 1))) {
        unsigned char c{ (unsigned char)(utf8[utf8Index++]) };
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
            if ((utf8Index + 1) >= utf8.length()) {
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

    while ((ucs2Index < (ucs2_size - 1)) && (utf8Index < utf8.length())) {
        ucs2[ucs2Index++] = 0xFFFD;
    }

    ucs2[ucs2Index] = L'\0';
}

TStringContainer::TStringContainer(const std::string &utf8) {
    size_t ucs2Size = ((utf8.length() * 2) + 1);
    LPWSTR ucs2{ new WCHAR[ucs2Size] };
    ToUCS2(utf8, ucs2, ucs2Size);

#ifdef UNICODE
    const size_t size{ (wcslen(ucs2) + 1) };
    string = new TCHAR[size];
    wcscpy(string, ucs2);
#else
    int ansiSize{ WideCharToMultiByte(CP_ACP,
                                      0,
                                      ucs2,
                                      -1,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL) };

    LPSTR ansi{ new CHAR[ansiSize] };
    WideCharToMultiByte(CP_ACP,
                        0,
                        ucs2,
                        -1,
                        ansi,
                        ansiSize,
                        NULL,
                        NULL);

    string = new CHAR[ansiSize];
    memcpy(string, ansi, ansiSize);

    delete[] ansi;
#endif

    delete[] ucs2;
}

LPTSTR TStringContainer::GetString(void) const {
    return string;
}

TStringContainer::~TStringContainer(void) {
    delete[] string;
    string = NULL;
}