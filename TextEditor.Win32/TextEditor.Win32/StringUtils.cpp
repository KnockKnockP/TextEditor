#include <StringUtils.hpp>

TStringContainer::TStringContainer(const int size) {
    string = new TCHAR[size];
}

TStringContainer::TStringContainer(const LPWSTR wString) {
#if UNICODE
    size_t size{ (wcslen(wString) + 1) };
    string = new TCHAR[size];
    wcscpy_s(string, size, wString);
#endif
}

LPTSTR TStringContainer::GetString(void) const {
    return string;
}

TStringContainer::~TStringContainer(void) {
    delete[] string;
    string = NULL;
}

WStringContainer::WStringContainer(const int size) {
    string = new WCHAR[size];
}

LPWSTR WStringContainer::GetString(void) const {
    return string;
}

WStringContainer::~WStringContainer(void) {
    delete[] string;
    string = NULL;
}

TStringContainer StringUtils::ToUTF16(const std::string &utf8) {
    int utf16Size{ MultiByteToWideChar(CP_UTF8,
                                       0,
                                       utf8.c_str(),
                                       -1,
                                       NULL,
                                       0) };

    WStringContainer utf16{ utf16Size };
    MultiByteToWideChar(CP_UTF8,
                        0,
                        utf8.c_str(),
                        -1,
                        utf16.GetString(),
                        utf16Size);
#ifdef UNICODE
    return TStringContainer(utf16.GetString());
#else
    int ansiSize{ WideCharToMultiByte(CP_ACP,
                                      0,
                                      utf16.GetString(),
                                      -1,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL) };

    TStringContainer ansi{ ansiSize };
    WideCharToMultiByte(CP_ACP,
                        0,
                        utf16.GetString(),
                        -1,
                        ansi.GetString(),
                        ansiSize,
                        NULL,
                        NULL);
    return ansi;
#endif
}