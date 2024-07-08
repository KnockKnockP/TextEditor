#include <StringUtils.hpp>

std::wstring StringUtils::ToUTF16(const std::string &utf8) {
    int size{ MultiByteToWideChar(CP_UTF8,
                                  0,
                                  utf8.c_str(),
                                  -1,
                                  NULL,
                                  0) };

    std::wstring utf16(size, 0);
    MultiByteToWideChar(CP_UTF8,
                        0,
                        utf8.c_str(),
                        -1,
                        &utf16[0],
                        size);
    return utf16;
}