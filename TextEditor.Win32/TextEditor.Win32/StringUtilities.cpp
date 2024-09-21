#include <StringUtilities.hpp>

namespace StringUtilities {
    uint8_t UTF8String::GetFirstUTF8ByteSize(const byte UTF8) noexcept {
        if ((UTF8 & 0b10000000) == 0b00000000) {
            return 1;
        } else if ((UTF8 & 0b11100000) == 0b11000000) {
            return 2;
        } else if ((UTF8 & 0b11110000) == 0b11100000) {
            return 3;
        } else if ((UTF8 & 0b11111000) == 0b11110000) {
            return 4;
        } else if ((UTF8 & 0b11111100) == 0b11111000) {
            return 5;
        } else if ((UTF8 & 0b11111110) == 0b11111100) {
            return 6;
        }

        return 0;
    }

    const inline std::vector<byte> UTF8String::GetBytesWithNullTerminator(void) const noexcept {
        std::vector<byte> bytesWithNullTerminator{ bytes };
        bytesWithNullTerminator.push_back(0);
        return bytesWithNullTerminator;
    }

    void UTF8String::PushCodePrint(const Unicode codePrint) noexcept {
        if (codePrint <= 0x7F) {
            bytes.push_back(static_cast<byte>(codePrint));
        } else if (codePrint <= 0x7FF) {
            bytes.push_back(static_cast<byte>(0xC0 | (codePrint >> 6)));
            bytes.push_back(static_cast<byte>(0x80 | (codePrint & 0x3F)));
        } else if (codePrint <= 0xFFFF) {
            bytes.push_back(static_cast<byte>(0xE0 | (codePrint >> 12)));
            bytes.push_back(static_cast<byte>(0x80 | ((codePrint >> 6) & 0x3F)));
            bytes.push_back(static_cast<byte>(0x80 | (codePrint & 0x3F)));
        } else {
            bytes.push_back(static_cast<byte>(0xF0 | (codePrint >> 18)));
            bytes.push_back(static_cast<byte>(0x80 | ((codePrint >> 12) & 0x3F)));
            bytes.push_back(static_cast<byte>(0x80 | ((codePrint >> 6) & 0x3F)));
            bytes.push_back(static_cast<byte>(0x80 | (codePrint & 0x3F)));
        }
    }

    void UTF8String::PushUntilNullTerminator(const byte *const pUTF8Bytes) noexcept {
        size_t i{ 0 };
        while (pUTF8Bytes[i]) {
            bytes.push_back(pUTF8Bytes[i++]);
        }
    }

    void UTF8String::Add(const Unicode codePrint) noexcept {
        PushCodePrint(codePrint);
    }

    void inline UTF8String::Add(const CHAR character) noexcept {
        bytes.push_back(character);
    }

    void UTF8String::Add(const WCHAR character) noexcept {
        const WCHAR characters[2] { character, 0 };
        PushUntilNullTerminator(UTF16ToUTF8(characters).get());
    }

    void UTF8String::Add(const LPCSTR pANSIString) noexcept {
        PushUntilNullTerminator(UTF16ToUTF8(ANSIToUTF16(pANSIString).get()).get());
    }

    void UTF8String::Add(const LPCWSTR pUTF16String) noexcept {
        PushUntilNullTerminator(UTF16ToUTF8(pUTF16String).get());
    }

    void UTF8String::Add(const std::string &string) noexcept {
        PushUntilNullTerminator(reinterpret_cast<const byte *>(string.c_str()));
    }

    void inline UTF8String::Add(const UTF8String &other) noexcept {
        bytes.insert(std::end(bytes), std::begin(other.bytes), std::end(other.bytes));
    }

    UTF8String::UTF8String(void) noexcept {}

    UTF8String::UTF8String(const UTF8String &UTF8String) noexcept {
        bytes = UTF8String.bytes;
    }

    UTF8String::UTF8String(const Unicode codePrint) noexcept {
        PushCodePrint(codePrint);
    }

    UTF8String::UTF8String(const CHAR character) noexcept {
        bytes.push_back(character);
    }

    UTF8String::UTF8String(const WCHAR character) noexcept {
        WCHAR pWideString[2] { character, 0 };

        PushUntilNullTerminator(UTF16ToUTF8(pWideString).get());
    }

    UTF8String::UTF8String(LPCSTR pANSIString) noexcept {
        PushUntilNullTerminator(UTF16ToUTF8(ANSIToUTF16(pANSIString).get()).get());
    }

    UTF8String::UTF8String(LPCWSTR pUTF16String) noexcept {
        PushUntilNullTerminator(UTF16ToUTF8(pUTF16String).get());
    }

    UTF8String::UTF8String(const std::string &string, const Encoding encoding) {
        switch (encoding) {
            case None:
                //throw std::invalid_argument{ "Encoding is None." };
                break;

            case ANSI:
                PushUntilNullTerminator(UTF16ToUTF8(ANSIToUTF16(string.c_str()).get()).get());
                break;

            case UTF_8:
            default:
                PushUntilNullTerminator(reinterpret_cast<const byte *>(string.c_str()));
                break;
        }
    }

    const UTF8String UTF8String::operator+(const Unicode codePrint) const noexcept {
        UTF8String string{ *this };
        string.Add(codePrint);
        return string;
    }

    const UTF8String UTF8String::operator+(const CHAR character) const noexcept {
        UTF8String string{ *this };
        string.Add(character);
        return string;
    }

    const UTF8String UTF8String::operator+(const WCHAR character) const noexcept {
        UTF8String string{ *this };
        string.Add(character);
        return string;
    }

    const UTF8String UTF8String::operator+(const LPCSTR pANSIString) const noexcept {
        UTF8String string{ *this };
        string.Add(pANSIString);
        return string;
    }

    const UTF8String UTF8String::operator+(const LPCWSTR pUTF16String) const noexcept {
        UTF8String string{ *this };
        string.Add(pUTF16String);
        return string;
    }

    const UTF8String UTF8String::operator+(const std::string &string) const noexcept {
        UTF8String newString{ *this };
        newString.Add(string);
        return newString;
    }

    const UTF8String UTF8String::operator+(const UTF8String &other) const noexcept {
        UTF8String string{ *this };
        string.Add(other);
        return string;
    }

    const UTF8String UTF8String::operator+=(const Unicode codePrint) noexcept {
        Add(codePrint);
        return UTF8String{ *this };
    }

    const UTF8String UTF8String::operator+=(const CHAR character) noexcept {
        Add(character);
        return UTF8String{ *this };
    }

    const UTF8String UTF8String::operator+=(const WCHAR character) noexcept {
        Add(character);
        return UTF8String{ *this };
    }

    const UTF8String UTF8String::operator+=(const LPCSTR pANSIString) noexcept {
        Add(pANSIString);
        return UTF8String{ *this };
    }

    const UTF8String UTF8String::operator+=(const LPCWSTR pUTF16String) noexcept {
        Add(pUTF16String);
        return UTF8String{ *this };
    }

    const UTF8String UTF8String::operator+=(const std::string &string) noexcept {
        Add(string);
        return UTF8String{ *this };
    }

    const UTF8String UTF8String::operator+=(const UTF8String &other) noexcept {
        Add(other);
        return UTF8String{ *this };
    }

    Unicode UTF8String::operator[](const size_t index) {
        if (index >= bytes.size()) {
            //throw std::out_of_range{ "Index " + std::to_string(index) + " is out of range." };
            return 0;
        }

        Unicode unicode{ 0 };
        size_t character{ 0 };
        for (size_t i{ 0 }; i < bytes.size(); ++i) {
            const uint8_t UTF8Bytes{ GetFirstUTF8ByteSize(bytes[i]) };
            if (character == index) {
                if (bytes.size() <= i + UTF8Bytes - 1) {
                    break;
                }

                if (UTF8Bytes == 1) {
                    unicode = bytes[i];
                } else if (UTF8Bytes == 2) {
                    const byte byte0{ static_cast<byte>(bytes[i] & 0b00011111) },
                        byte1{ static_cast<byte>(bytes[i + 1] & 0b00111111) };
                    unicode = byte0;
                    unicode <<= 6;

                    unicode |= byte1;
                } else if (UTF8Bytes == 3) {
                    const byte byte0{ static_cast<byte>(bytes[i] & 0b00001111) },
                        byte1{ static_cast<byte>(bytes[i + 1] & 0b00111111) },
                        byte2{ static_cast<byte>(bytes[i + 2] & 0b00111111) };
                    unicode = byte0;
                    unicode <<= 6;

                    unicode |= byte1;
                    unicode <<= 6;

                    unicode |= byte2;
                } else if (UTF8Bytes == 4) {
                    const byte byte0{ static_cast<byte>(bytes[i] & 0b00000111) },
                        byte1{ static_cast<byte>(bytes[i + 1] & 0b00111111) },
                        byte2{ static_cast<byte>(bytes[i + 2] & 0b00111111) },
                        byte3{ static_cast<byte>(bytes[i + 3] & 0b00111111) };
                    unicode = byte0;
                    unicode <<= 6;

                    unicode |= byte1;
                    unicode <<= 6;

                    unicode |= byte2;
                    unicode <<= 6;

                    unicode |= byte3;
                }

                return unicode;
            }

            if (UTF8Bytes) {
                ++character;
            }
            i += static_cast<size_t>(UTF8Bytes) - 1;
        }

        return 0;
    }

    std::unique_ptr<const CHAR[]> UTF8String::GetANSIString(void) const noexcept {
        return UTF16ToANSI(UTF8ToUTF16(reinterpret_cast<LPCSTR>(GetBytesWithNullTerminator().data())).get());
    }

    std::unique_ptr<const WCHAR[]> UTF8String::GetUTF16String(void) const noexcept {
        return UTF8ToUTF16(reinterpret_cast<LPCSTR>(GetBytesWithNullTerminator().data()));
    }

    std::unique_ptr<const TCHAR[]> UTF8String::GetWindowsTString(void) const noexcept {
#if UNICODE
        return GetUTF16String();
#else
        return GetANSIString();
#endif
    }

    void UTF8String::RemoveLastCharacter(void) noexcept {
        size_t lastCharacterPosition{ 0 };
        for (size_t i{ 0 }; i < bytes.size(); ++i) {
            const uint8_t size{ GetFirstUTF8ByteSize(bytes[i]) };
            if (size) {
                lastCharacterPosition = i;
                i += static_cast<size_t>(size) - 1;
            }
        }

        const size_t bytesToRemove{ bytes.size() - lastCharacterPosition };
        for (size_t i{ 0 }; i < bytesToRemove; ++i) {
            bytes.pop_back();
        }
    }

    const UTF8String operator+(const Unicode codePrint, const UTF8String &string) {
        UTF8String newString{ static_cast<Unicode>(codePrint) };
        newString.Add(string);
        return newString;
    }

    const UTF8String operator+(const LPCSTR pANSIString, const UTF8String &string){
        UTF8String newString{ pANSIString };
        newString.Add(string);
        return newString;
    }

    const UTF8String operator+(const LPCWSTR pUTF16String, const UTF8String &string) {
        UTF8String newString{ pUTF16String };
        newString.Add(string);
        return newString;
    }

    inline std::unique_ptr<const WCHAR[]> ANSIToUTF16(const CHAR *const pANSIString) {
        const int UTF16Characters{ MultiByteToWideChar(CP_ACP, 0, pANSIString, -1, nullptr, 0)};
        std::unique_ptr<WCHAR[]> pUTF16String{std::make_unique<WCHAR[]>(UTF16Characters)};

        if (!MultiByteToWideChar(CP_ACP, 0, pANSIString, -1, pUTF16String.get(), UTF16Characters)) {
            STRINGUTILITIES_UTF8STRING_THROW();
        }
        return pUTF16String;
    }

    inline std::unique_ptr<const CHAR[]> UTF16ToANSI(const WCHAR *const pUTF16String) {
        const int ANSICharacters{ WideCharToMultiByte(CP_ACP, 0, pUTF16String, -1, nullptr, 0, nullptr, nullptr)};
        std::unique_ptr<CHAR[]> pANSIString{std::make_unique<CHAR[]>(ANSICharacters)};

        if (!WideCharToMultiByte(CP_ACP, 0, pUTF16String, -1, pANSIString.get(), ANSICharacters, nullptr, nullptr)) {
            STRINGUTILITIES_UTF8STRING_THROW();
        }
        return pANSIString;
    }

    inline std::unique_ptr<const byte[]> UTF16ToUTF8(const WCHAR *const pUTF16String) {
        const int UTF8Size{ WideCharToMultiByte(CP_UTF8, 0, pUTF16String, -1, nullptr, 0, nullptr, nullptr)};
        std::unique_ptr<byte[]> pUTF8String{std::make_unique<byte[]>(UTF8Size)};

        if (!WideCharToMultiByte(CP_UTF8, 0, pUTF16String, - 1, reinterpret_cast<CHAR *>(pUTF8String.get()), UTF8Size, nullptr, nullptr)) {
            STRINGUTILITIES_UTF8STRING_THROW();
        }
        return pUTF8String;
    }

    inline std::unique_ptr<const WCHAR[]> UTF8ToUTF16(const CHAR *const pUTF8String) {
        const int UTF16Characters{ MultiByteToWideChar(CP_UTF8, 0, pUTF8String, -1, nullptr, 0)};
        std::unique_ptr<WCHAR[]> pUTF16String{std::make_unique<WCHAR[]>(UTF16Characters)};

        if (!MultiByteToWideChar(CP_UTF8, 0, pUTF8String, -1, pUTF16String.get(), UTF16Characters)) {
            STRINGUTILITIES_UTF8STRING_THROW();
        }
        return pUTF16String;
    }
}