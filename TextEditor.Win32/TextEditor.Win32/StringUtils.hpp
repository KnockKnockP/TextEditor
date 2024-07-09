#pragma once

#include <string>
#include <Windows.h>

class TStringContainer {
private:
    LPTSTR string{ NULL };

public:
    TStringContainer(const int size);
    TStringContainer(const LPWSTR wString);
    LPTSTR GetString(void) const;
    ~TStringContainer(void);
};

class WStringContainer {
private:
    LPWSTR string{ NULL };

public:
    WStringContainer(const int size);
    LPWSTR GetString(void) const;
    ~WStringContainer(void);
};

class StringUtils {
public:
    static TStringContainer ToUTF16(const std::string &utf8);
};