#ifndef UNIFIEDSTRING_HPP
#define UNIFIEDSTRING_HPP
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <Windows.h>

#define STRINGUTILITIES_UTF8STRING_THROW() throw std::runtime_error{ "GetLastError: " + std::to_string(GetLastError()) }

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

        static constexpr uint8_t GetFirstUTF8ByteSize(const byte UTF8) noexcept;

        const std::vector<byte> GetBytesWithNullTerminator(void) const noexcept;
        void PushCodePrint(const Unicode codePrint) noexcept;
        void PushUntilNullTerminator(const byte* const UTF8Bytes) noexcept;

    public:
        UTF8String(void) noexcept;
        UTF8String(const UTF8String& UTF8String) noexcept;
        UTF8String(const Unicode codePrint) noexcept;
        UTF8String(const CHAR character) noexcept;
        UTF8String(const WCHAR character) noexcept;
        UTF8String(LPCSTR pANSIString) noexcept;
        UTF8String(LPCWSTR pUTF16String) noexcept;
        UTF8String(const std::string& string, const Encoding encoding = Encoding::UTF_8);

        const UTF8String operator+(const Unicode codePrint) const noexcept;
        const UTF8String operator+(const CHAR character) const noexcept;
        const UTF8String operator+(const WCHAR character) const noexcept;
        const UTF8String operator+(const LPCSTR pANSIString) const noexcept;
        const UTF8String operator+(const LPCWSTR pUTF16String) const noexcept;
        const UTF8String operator+(const std::string& string) const noexcept; //Assumes UTF-8.
        const UTF8String operator+(const UTF8String& other) const noexcept;

        const UTF8String operator+=(const Unicode codePrint) noexcept;
        const UTF8String operator+=(const CHAR character) noexcept;
        const UTF8String operator+=(const WCHAR character) noexcept;
        const UTF8String operator+=(const LPCSTR pANSIString) noexcept;
        const UTF8String operator+=(const LPCWSTR pUTF16String) noexcept;
        const UTF8String operator+=(const std::string& string) noexcept; //Assumes UTF-8.
        const UTF8String operator+=(const UTF8String& other) noexcept;

        Unicode operator[](const size_t index);

        std::shared_ptr<const CHAR[]> GetWindowsANSIString(void) const noexcept;
        std::shared_ptr<const WCHAR[]> GetWindowsUTF16String(void) const noexcept;
        const std::shared_ptr<const TCHAR[]> GetWindowsTString(void) const noexcept;

        void RemoveLastCharacter(void) noexcept;

        void Add(const Unicode codePrint) noexcept;
        void Add(const CHAR character) noexcept;
        void Add(const WCHAR character) noexcept;
        void Add(const LPCSTR pANSIString) noexcept;
        void Add(const LPCWSTR pUTF16String) noexcept;
        void Add(const std::string& string) noexcept; //Assumes UTF-8.
        void Add(const UTF8String& other) noexcept;
    };

    const UTF8String operator+(const Unicode codePrint, const UTF8String &string);
    const UTF8String operator+(const LPCWSTR pUTF16String, const UTF8String &string);

    inline std::unique_ptr<const WCHAR[]> ANSIToUTF16(LPCSTR pANSIString);
    inline std::unique_ptr<const CHAR[]> UTF16ToANSI(LPCWSTR pUTF16String);
    inline std::unique_ptr<const byte[]> UTF16ToUTF8(LPCWSTR pUTF16String);
    inline std::unique_ptr<const WCHAR[]> UTF8ToUTF16(LPCSTR pUTF8String);
}
#endif