#pragma once

#include <string>
#include <Windows.h>

class TStringContainer {
private:
    LPTSTR string{ NULL };

public:
    TStringContainer(const int size);
    TStringContainer(const std::string &utf8);
    LPTSTR GetString(void) const;
    ~TStringContainer(void);
};