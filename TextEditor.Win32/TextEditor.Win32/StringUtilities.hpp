#ifndef UNIFIEDSTRING_HPP
#define UNIFIEDSTRING_HPP
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <Windows.h>

#ifndef ST
#define ST(TEXT) L##TEXT
#endif

#ifndef STRINGUTILITIES_UTF8STRING_THROW
//#define STRINGUTILITIES_UTF8STRING_THROW() throw std::runtime_error{ "GetLastError: " + std::to_string(GetLastError()) }
#define STRINGUTILITIES_UTF8STRING_THROW()
#endif

typedef uint8_t byte;
typedef uint32_t Unicode;

namespace StringUtilities {
    enum Encoding {
        None,
        ANSI,
        UTF_8
    };

    //This does not handle when bytes are bigger than size_t or int.
    class UTF8String {
    private:
        std::vector<byte> bytes{};

        static uint8_t GetFirstUTF8ByteSize(const byte UTF8) noexcept;

        const std::vector<byte> GetBytesWithNullTerminator(void) const noexcept;
        void PushCodePrint(const Unicode codePrint) noexcept;
        void PushUntilNullTerminator(const byte *const UTF8Bytes) noexcept;

    public:
        UTF8String(void) noexcept;
        UTF8String(const UTF8String &UTF8String) noexcept;
        UTF8String(const Unicode codePrint) noexcept;
        UTF8String(const CHAR character) noexcept;
        UTF8String(const WCHAR character) noexcept;
        UTF8String(LPCSTR pANSIString) noexcept;
        UTF8String(LPCWSTR pUTF16String) noexcept;
        UTF8String(const std::string &string, const Encoding encoding = Encoding::UTF_8);

        const UTF8String operator+(const Unicode codePrint) const noexcept;
        const UTF8String operator+(const CHAR character) const noexcept;
        const UTF8String operator+(const WCHAR character) const noexcept;
        const UTF8String operator+(const LPCSTR pANSIString) const noexcept;
        const UTF8String operator+(const LPCWSTR pUTF16String) const noexcept;
        const UTF8String operator+(const std::string &string) const noexcept; //Assumes UTF-8.
        const UTF8String operator+(const UTF8String &other) const noexcept;

        const UTF8String operator+=(const Unicode codePrint) noexcept;
        const UTF8String operator+=(const CHAR character) noexcept;
        const UTF8String operator+=(const WCHAR character) noexcept;
        const UTF8String operator+=(const LPCSTR pANSIString) noexcept;
        const UTF8String operator+=(const LPCWSTR pUTF16String) noexcept;
        const UTF8String operator+=(const std::string &string) noexcept; //Assumes UTF-8.
        const UTF8String operator+=(const UTF8String &other) noexcept;

        Unicode operator[](const size_t index);

        std::unique_ptr<const CHAR[]> GetANSIString(void) const noexcept;
        std::unique_ptr<const WCHAR[]> GetUTF16String(void) const noexcept;
        std::unique_ptr<const TCHAR[]> GetWindowsTString(void) const noexcept;

        void RemoveLastCharacter(void) noexcept;

        void Add(const Unicode codePrint) noexcept;
        void Add(const CHAR character) noexcept;
        void Add(const WCHAR character) noexcept;
        void Add(const LPCSTR pANSIString) noexcept;
        void Add(const LPCWSTR pUTF16String) noexcept;
        void Add(const std::string &string) noexcept; //Assumes UTF-8.
        void Add(const UTF8String &other) noexcept;
    };

    const UTF8String operator+(const Unicode codePrint, const UTF8String &string);
    const UTF8String operator+(const LPCSTR pANSIString, const UTF8String &string);
    const UTF8String operator+(const LPCWSTR pUTF16String, const UTF8String &string);

    inline std::unique_ptr<const WCHAR[]> ANSIToUTF16(const CHAR* const pANSIString);
    inline std::unique_ptr<const CHAR[]> UTF16ToANSI(const WCHAR* const pUTF16String);
    inline std::unique_ptr<const byte[]> UTF16ToUTF8(const WCHAR* const pUTF16String);
    inline std::unique_ptr<const WCHAR[]> UTF8ToUTF16(const CHAR* const pUTF8String);
}
#endif