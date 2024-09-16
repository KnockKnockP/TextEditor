#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP
#pragma once

#ifndef UT
#define UT (LPTSTR)TEXT
#endif

#include <vector>
#include <cstdint>
#include <Windows.h>

class UnifiedString {
private:
    size_t characters{ 0 };
    std::vector<char> bytes{};
    LPTSTR windowsString{ nullptr };

    void PushAllBytes(const std::vector<char> bytes);
    void PushUnicode(const uint32_t unicode);
    void PushUnicode(const LPTSTR string); //MB or UTF-8

public:
    UnifiedString(void);
    UnifiedString(const std::vector<char> &bytes);
    UnifiedString(const uint32_t unicode);
    UnifiedString(const char character);
    UnifiedString(const LPTSTR string); //MB or UTF-8
    UnifiedString(const UnifiedString &other);

    UnifiedString operator+(const UnifiedString other);
    UnifiedString operator+(const LPTSTR other); //MB or UTF-8

    void operator+=(const uint32_t other);
    void operator+=(const LPSTR other);

    void RemoveLastCharacter(void);
    const std::vector<char> GetBytes(void) const;
    LPTSTR GetWindowsString(void);

    ~UnifiedString(void);
};

UnifiedString operator+(const LPTSTR string, const UnifiedString other);
#endif