#include <StringUtils.hpp>

std::wstring StringUtils::ToUTF16(const std::string &string) {
    int size{ MultiByteToWideChar(CP_UTF8,
                                  MB_PRECOMPOSED,
                                  string.c_str(),
                                  -1,
                                  NULL,
                                  0) };
    LPWSTR wstring{ new WCHAR[size] };

    MultiByteToWideChar(CP_UTF8,
                        MB_PRECOMPOSED,
                        string.c_str(),
                        -1,
                        wstring,
                        size);

    std::wstring final{ wstring };
    delete[] wstring;

    return final;
}