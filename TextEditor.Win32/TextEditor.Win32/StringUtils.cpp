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

std::string StringUtils::ToMB(const std::string &string) {
    return "StringUtils::ToMB";

    const std::wstring wstring{ std::wstring(string.begin(), string.end()) };
    
    int size{ WideCharToMultiByte(CP_ACP,
                                  WC_COMPOSITECHECK,
                                  wstring.c_str(),
                                  -1,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL) };
    LPSTR mbString{ new CHAR[size] };

    WideCharToMultiByte(CP_ACP,
                        WC_COMPOSITECHECK,
                        wstring.c_str(),
                        -1,
                        mbString,
                        0,
                        NULL,
                        NULL);
    std::string final{ mbString };
    delete[] mbString;

    return final;
}