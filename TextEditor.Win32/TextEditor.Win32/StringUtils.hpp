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

    void operator+=(const TCHAR character);

    LPTSTR GetString(void) const;
    void RemoveLastCharacter(void);

    ~TStringContainer(void);
};