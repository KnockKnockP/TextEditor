#pragma once

#include <string>
#include <Windows.h>

class StringUtils {
public:
    static std::wstring ToUTF16(const std::string &string);
};