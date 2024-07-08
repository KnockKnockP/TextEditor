#pragma once

#include <string>
#include <Windows.h>

#ifdef UNICODE
#define TT(quote) std::string(quote)
#else
#define TT(quote) StringUtilities::utf8_decode(quote)
#endif

//Source code = UTF-8
//Windows NT UNICODE = WCHAR
//Windows 9x = CHAR

class StringUtilities {
public:
	static std::string utf8_encode(const std::wstring &wstr) {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    static std::string utf8_decode(const std::string &str) {
        if (str.empty()) return std::string();
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return std::string(wstrTo.begin(), wstrTo.end());
    }
};