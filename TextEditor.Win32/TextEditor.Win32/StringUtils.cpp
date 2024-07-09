#include <StringUtils.hpp>
#include <mbstring.h>

TStringContainer::TStringContainer(const int size) {
    string = new TCHAR[size];
}

TStringContainer::TStringContainer(const std::string &utf8) {
    int utf16Size{ MultiByteToWideChar(CP_UTF8,
                                       0,
                                       utf8.c_str(),
                                       -1,
                                       NULL,
                                       0) };

    LPWSTR utf16{ new WCHAR[utf16Size] };
    MultiByteToWideChar(CP_UTF8,
                        0,
                        utf8.c_str(),
                        -1,
                        utf16,
                        utf16Size);
#ifdef UNICODE
    const size_t size{ (wcslen(utf16) + 1) };
    string = new TCHAR[size];
    wcscpy(string, utf16);
#else
    int ansiSize{ WideCharToMultiByte(CP_ACP,
                                      0,
                                      utf16,
                                      -1,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL) };

    LPSTR ansi{ new CHAR[ansiSize] };
    WideCharToMultiByte(CP_ACP,
                        0,
                        utf16,
                        -1,
                        ansi,
                        ansiSize,
                        NULL,
                        NULL);

    string = new CHAR[ansiSize];
    memcpy(string, ansi, ansiSize);

    delete[] ansi;
#endif

    delete[] utf16;
}

LPTSTR TStringContainer::GetString(void) const {
    return string;
}

TStringContainer::~TStringContainer(void) {
    delete[] string;
    string = NULL;
}