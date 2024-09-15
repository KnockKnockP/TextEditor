#pragma once

#include <string>
#include <Windows.h>

namespace StringUtils {
    static std::string UnicodeToUTF8(const WPARAM codePrint);
}

class TStringContainer {
private:
    size_t size{ 0 };
    LPTSTR string{ NULL };

public:
    TStringContainer(const int size);
    TStringContainer(const std::string &utf8);

    LPTSTR GetString(void) const;

    ~TStringContainer(void);
};