#include <StringUtils.hpp>
#include <codecvt>

std::wstring StringUtils::ToUTF16(const std::string &string) {
    std::wstring_convert<std::codecvt_utf8_utf16<WCHAR>> converter;
    return converter.from_bytes(string);
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