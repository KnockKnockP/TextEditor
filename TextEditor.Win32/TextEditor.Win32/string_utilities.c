#include <leak_checker.h>

#include <string_utilities.h>
#include <memory_helper.h>
#include <windows_helper.h>

#ifndef STRING_UTILITIES_PUSH_UTF8
#define STRING_UTILITIES_PUSH_UTF8() character = (character << 6) | (pUTF8_string[i++] & 0b00111111)
#endif

static BOOL STRING_UTILITIES_CHECK_UTF8_BOUNDARY(const size_t size, const size_t i, const size_t cluster_size) {
    return size >= cluster_size && i < size;
}

static BOOL STRING_UTILITIES_CHECK_UTF8_CLUSTER_FOLLOW_UPS(const BYTE * const pBytes, const size_t i) {
    return (pBytes[i] & 0b11000000) == 0b10000000;
}

static BOOL STRING_UTILITIES_CHECK_UTF8_CLUSTER_1(const byte UTF8_byte) {
    return (UTF8_byte & 0b10000000) == 0b00000000;
}

static BOOL STRING_UTILITIES_CHECK_UTF8_CLUSTER_2(const BYTE * const pBytes, const size_t size, const size_t i) {
    return STRING_UTILITIES_CHECK_UTF8_BOUNDARY(size, i, 2) &&
           ((pBytes[i] & 0b11100000) == 0b11000000) &&
           STRING_UTILITIES_CHECK_UTF8_CLUSTER_FOLLOW_UPS(pBytes, i + 1);
}

static BOOL STRING_UTILITIES_CHECK_UTF8_CLUSTER_3(const BYTE * const pBytes, const size_t size, const size_t i) {
    return STRING_UTILITIES_CHECK_UTF8_BOUNDARY(size, i, 3) &&
           ((pBytes[i] & 0b11110000) == 0b11100000) &&
           STRING_UTILITIES_CHECK_UTF8_CLUSTER_FOLLOW_UPS(pBytes, i + 1) &&
           STRING_UTILITIES_CHECK_UTF8_CLUSTER_FOLLOW_UPS(pBytes, i + 2);
}

static BOOL STRING_UTILITIES_CHECK_UTF8_CLUSTER_4(const BYTE * const pBytes, const size_t size, const size_t i) {
    return STRING_UTILITIES_CHECK_UTF8_BOUNDARY(size, i, 4) &&
           ((pBytes[i] & 0b11111000) == 0b11110000) &&
           STRING_UTILITIES_CHECK_UTF8_CLUSTER_FOLLOW_UPS(pBytes, i + 1) &&
           STRING_UTILITIES_CHECK_UTF8_CLUSTER_FOLLOW_UPS(pBytes, i + 2) &&
           STRING_UTILITIES_CHECK_UTF8_CLUSTER_FOLLOW_UPS(pBytes, i + 3);
}

int STRING_UTILITIES_detect_encoding(const BYTE * const pBytes, const size_t size) {
    int code_page = UTF8;
    if (size >= 2 && pBytes[0] == 0xFF && pBytes[1] == 0xFE) {
        //WIDE means UTF-16 LE with BOM for now.
        code_page = WIDE;
    } else if (size >= 3 && pBytes[0] == 0xEF && pBytes[1] == 0xBB && pBytes[2] == 0xBF) {
        code_page = UTF8_WITH_BOM;
    } else {
        size_t i = 0;
        while (pBytes[i]) {
            //If no valid UTF-8 bytes are found, assume ANSI.
            if (STRING_UTILITIES_CHECK_UTF8_CLUSTER_1(pBytes[i])) {
                ++i;
            } else if (STRING_UTILITIES_CHECK_UTF8_CLUSTER_2(pBytes, size, i)) {
                i += 2;
            } else if (STRING_UTILITIES_CHECK_UTF8_CLUSTER_3(pBytes, size, i)) {
                i += 3;
            } else if (STRING_UTILITIES_CHECK_UTF8_CLUSTER_4(pBytes, size, i)) {
                i += 4;
            } else {
                code_page = ANSI;
                break;
            }
        }
    }
    return code_page;
}

LPTSTR STRING_UTILITIES_encoding_enum_to_string(const int encoding_enum) {
    if (encoding_enum == ANSI) {
        return TEXT("ANSI");
    }

    if (encoding_enum == WIDE) {
        return TEXT("UTF-16 LE BOM (Wide)");
    }

    if (encoding_enum == UTF8) {
        return TEXT("UTF-8");
    }

    if (encoding_enum == UTF8_WITH_BOM) {
        return TEXT("UTF-8 BOM");
    }

    return TEXT("Unknown Encoding");
}

LPTSTR STRING_UTILITIES_w_to_t(LPCWSTR pWide_string) {
#ifdef UNICODE
    return (LPTSTR)STRING_UTILITIES_clone_w_to_w(pWide_string);
#else
    return (LPTSTR)STRING_UTILITIES_w_to_a(pWide_string);
#endif
}

LPWSTR STRING_UTILITIES_a_to_w(LPCSTR pANSI_string) {
    const int wide_characters = MultiByteToWideChar(CP_ACP, 0, pANSI_string, -1, NULL, 0);
    LPWSTR pUTF_16_String = malloc(sizeof(WCHAR) * wide_characters);
    if (!pUTF_16_String) {
        WINDOWS_HELPER_THROW();
        return NULL;
    }

    if (!MultiByteToWideChar(CP_ACP, 0, pANSI_string, -1, pUTF_16_String, wide_characters)) {
        WINDOWS_HELPER_THROW();
        return NULL;
    }
    return pUTF_16_String;
}

LPSTR STRING_UTILITIES_w_to_a(LPCWSTR pWide_string) {
    const int ANSI_characters = WideCharToMultiByte(CP_ACP, 0, pWide_string, -1, NULL, 0, NULL, NULL);
    LPSTR pANSI_string = malloc(sizeof(CHAR) * ANSI_characters);
    if (!pANSI_string) {
        WINDOWS_HELPER_THROW();
        return NULL;
    }

    if (!WideCharToMultiByte(CP_ACP, 0, pWide_string, -1, pANSI_string, ANSI_characters, NULL, NULL)) {
        WINDOWS_HELPER_THROW();
        return NULL;
    }
    return pANSI_string;
}

LPWSTR STRING_UTILITIES_UTF8_to_w(LPCSTR pUTF8_string) {
    WIDE_STRING wide_string = WIDE_STRING_create_empty();
    const size_t size = strlen(pUTF8_string);
    int i = 0;

    while (pUTF8_string[i]) {
        uint32_t character = 0;

        if (STRING_UTILITIES_CHECK_UTF8_CLUSTER_1(pUTF8_string[i])) {
            character = pUTF8_string[i++];
        } else if (STRING_UTILITIES_CHECK_UTF8_CLUSTER_2((const BYTE * const)pUTF8_string, size, i)) {
            character = pUTF8_string[i++] & 0b00011111;
            STRING_UTILITIES_PUSH_UTF8();
        } else if (STRING_UTILITIES_CHECK_UTF8_CLUSTER_3((const BYTE * const)pUTF8_string, size, i)) {
            character = pUTF8_string[i++] & 0b00001111;
            STRING_UTILITIES_PUSH_UTF8();
            STRING_UTILITIES_PUSH_UTF8();
        } else if (STRING_UTILITIES_CHECK_UTF8_CLUSTER_4((const BYTE * const)pUTF8_string, size, i)) {
            character = pUTF8_string[i++] & 0b00000111;
            STRING_UTILITIES_PUSH_UTF8();
            STRING_UTILITIES_PUSH_UTF8();
            STRING_UTILITIES_PUSH_UTF8();
        }

        WCHAR converted[3] = { (WCHAR)character, 0, 0 };
        if (character >= 0x10000) {
            character -= 0x10000;
            converted[0] = 0xD800 + (WCHAR)(character >> 10);
            converted[1] = 0xDC00 + (character & 0b1111111111);
        }

        WIDE_STRING_append_string_w(&wide_string, converted);
    }

    LPWSTR cloned = STRING_UTILITIES_clone_w_to_w(wide_string.pWide_string);
    WIDE_STRING_destroy(&wide_string);
    return cloned;
}

LPWSTR STRING_UTILITIES_clone_w_to_w(LPCWSTR pOriginal) {
    const size_t characters = wcslen(pOriginal);
    LPWSTR pClone = malloc(sizeof(WCHAR) * (characters + 1));
    if (!pClone) {
        WINDOWS_HELPER_THROW();
        return NULL;
    }

    wcscpy(pClone, pOriginal);
    return pClone;
}

size_t STRING_UTILITIES_characters(LPCTSTR pString) {
#ifdef UNICODE
    return wcslen(pString);
#else
    return strlen(pString);
#endif
}

static void WIDE_STRING_update_individual_lines(WIDE_STRING *pWide_string) {
    for (size_t i = 0; i < pWide_string->individual_lines.size; ++i) {
        MEMORY_HELPER_free((void **)&pWide_string->individual_lines.pArray[i]);
    }
    VECTOR_CLEAR_LPWSTR(&pWide_string->individual_lines);

    const size_t characters = wcslen(pWide_string->pWide_string);
    size_t line_characters = 0;
    LPWSTR line = malloc(sizeof(WCHAR));
    if (!line) {
        return;
    }

    for (size_t i = 0; i <= characters; ++i) {
        if (pWide_string->pWide_string[i] == L'\n' || pWide_string->pWide_string[i] == L'\0') {
            line[line_characters] = L'\0';
            VECTOR_PUSH_LPWSTR(&pWide_string->individual_lines, line);

            if (pWide_string->pWide_string[i] == L'\0') {
                break;
            }

            line = malloc(sizeof(WCHAR));
            if (!line) {
                return;
            }
            line_characters = 0;
        } else {
            LPWSTR temporary = realloc(line, sizeof(WCHAR) * (++line_characters + 1));
            if (!temporary) {
                MEMORY_HELPER_free((void **)&temporary);
                return;
            }
            line = temporary;

            line[line_characters - 1] = pWide_string->pWide_string[i];
        }
    }
}

void WIDE_STRING_initialize(WIDE_STRING *pWide_string) {
    pWide_string->pWide_string = NULL;
    pWide_string->individual_lines = VECTOR_CREATE_LPWSTR();
}

WIDE_STRING WIDE_STRING_create_empty(void) {
    WIDE_STRING wide_string = { 0 };
    WIDE_STRING_initialize(&wide_string);

    LPWSTR pString = malloc(sizeof(WCHAR));
    if (!pString) {
        WINDOWS_HELPER_THROW();
        return wide_string;
    }

    pString[0] = L'\0';
    wide_string.pWide_string = pString;
    WIDE_STRING_update_individual_lines(&wide_string);
    return wide_string;
}

WIDE_STRING WIDE_STRING_create_a(LPCSTR pANSI_string) {
    WIDE_STRING wide_string = { 0 };
    WIDE_STRING_initialize(&wide_string);

    wide_string.pWide_string = STRING_UTILITIES_a_to_w(pANSI_string);
    WIDE_STRING_update_individual_lines(&wide_string);
    return wide_string;
}

WIDE_STRING WIDE_STRING_create_w(LPCWSTR pWide_string) {
    WIDE_STRING wide_string = { 0 };
    WIDE_STRING_initialize(&wide_string);

    const size_t wide_characters = wcslen(pWide_string) + 1;
    LPWSTR pString = malloc(sizeof(WCHAR) * wide_characters);
    if (!pString) {
        WINDOWS_HELPER_THROW();
        return wide_string;
    }

    wcscpy(pString, pWide_string);
    wide_string.pWide_string = pString;
    WIDE_STRING_update_individual_lines(&wide_string);
    return wide_string;
}

WIDE_STRING WIDE_STRING_create(LPCTSTR pT_string) {
#ifdef UNICODE
    return WIDE_STRING_create_w(pT_string);
#else
    return WIDE_STRING_create_a(pT_string);
#endif
}

LPCTSTR WIDE_STRING_get_t_string(WIDE_STRING *pWide_string) {
#ifdef UNICODE
    const size_t characters = wcslen(pWide_string->pWide_string);
    LPWSTR temporary = malloc(sizeof(WCHAR) * (characters + 1));
    if (!temporary) {
        WINDOWS_HELPER_THROW();
        return NULL;
    }

    wcscpy(temporary, pWide_string->pWide_string);
    return temporary;
#else
    return STRING_UTILITIES_w_to_a(pWide_string->pWide_string);
#endif
}

void WIDE_STRING_append_wide_char(WIDE_STRING *pOriginal, const WCHAR additional) {
    const size_t characters = wcslen(pOriginal->pWide_string) + 2;
    LPWSTR pString = malloc(sizeof(WCHAR) * characters);
    if (!pString) {
        WINDOWS_HELPER_THROW();
        return;
    }

    wcscpy(pString, pOriginal->pWide_string);
    pString[characters - 2] = additional;
    pString[characters - 1] = L'\0';

    MEMORY_HELPER_free((void **)&pOriginal->pWide_string);
    pOriginal->pWide_string = pString;
    WIDE_STRING_update_individual_lines(pOriginal);
}

void WIDE_STRING_append_wide_char_at(WIDE_STRING *pOriginal, const WCHAR additional, const size_t at) {
    const size_t original_length = wcslen(pOriginal->pWide_string);
    LPWSTR pNew = malloc(sizeof(WCHAR) * (original_length + 2));
    if (!pNew) {
        WINDOWS_HELPER_THROW();
        return;
    }

    for (size_t i = 0; i < at; ++i) {
        pNew[i] = pOriginal->pWide_string[i];
    }
    pNew[at] = additional;

    for (size_t i = at; i < original_length; ++i) {
        pNew[i + 1] = pOriginal->pWide_string[i];
    }
    pNew[original_length + 1] = L'\0';

    MEMORY_HELPER_free((void **)&pOriginal->pWide_string);
    pOriginal->pWide_string = pNew;
    WIDE_STRING_update_individual_lines(pOriginal);
}

void WIDE_STRING_append_string_a(WIDE_STRING *pOriginal, LPCSTR pAdditional) {
    WIDE_STRING additional_wide = WIDE_STRING_create_a(pAdditional);

    const size_t characters = wcslen(pOriginal->pWide_string) + wcslen(additional_wide.pWide_string) + 1;
    LPWSTR pString = malloc(sizeof(WCHAR) * characters);
    if (!pString) {
        WINDOWS_HELPER_THROW();
        return;
    }

    wcscpy(pString, pOriginal->pWide_string);
    wcscat(pString, additional_wide.pWide_string);

    WIDE_STRING_destroy(&additional_wide);
    MEMORY_HELPER_free((void **)&pOriginal->pWide_string);
    pOriginal->pWide_string = pString;
    WIDE_STRING_update_individual_lines(pOriginal);
}

void WIDE_STRING_append_string_a_at(WIDE_STRING *pOriginal, LPCSTR pAdditional, const size_t at) {
    LPCWSTR pAdditional_wide = STRING_UTILITIES_a_to_w(pAdditional);
    WIDE_STRING_append_string_w_at(pOriginal, pAdditional_wide, at);
}

void WIDE_STRING_append_string_w(WIDE_STRING *pOriginal, LPCWSTR pAdditional) {
    WIDE_STRING additional_wide = WIDE_STRING_create_w(pAdditional);
    WIDE_STRING_append_WIDE_STRING(pOriginal, &additional_wide);
    WIDE_STRING_destroy(&additional_wide);
}

void WIDE_STRING_append_string_w_at(WIDE_STRING *pOriginal, LPCWSTR pAdditional, const size_t at) {
    const size_t original_characters = wcslen(pOriginal->pWide_string), additional_characters = wcslen(pAdditional);

    LPWSTR pNew = malloc(sizeof(WCHAR) * (original_characters + additional_characters + 1));
    if (!pNew) {
        WINDOWS_HELPER_THROW();
        return;
    }

    size_t i = 0;
    for (; i < at; ++i) {
        pNew[i] = pOriginal->pWide_string[i];
    }

    for (size_t j = 0; j < additional_characters; ++i, ++j) {
        pNew[i] = pAdditional[j];
    }

    for (size_t j = at; j < original_characters; ++i, ++j) {
        pNew[i] = pOriginal->pWide_string[j];
    }

    pNew[i] = L'\0';

    MEMORY_HELPER_free((void **)&pOriginal->pWide_string);
    pOriginal->pWide_string = pNew;
    WIDE_STRING_update_individual_lines(pOriginal);
}

void WIDE_STRING_append_string(WIDE_STRING *pOriginal, LPCTSTR pAdditional) {
#ifdef UNICODE
    WIDE_STRING_append_string_w(pOriginal, pAdditional);
#else
    WIDE_STRING_append_string_a(pOriginal, pAdditional);
#endif
}

void WIDE_STRING_append_string_at(WIDE_STRING *pOriginal, LPCTSTR pAdditional, const size_t at) {
#ifdef UNICODE
    WIDE_STRING_append_string_w_at(pOriginal, pAdditional, at);
#else
    WIDE_STRING_append_string_a_at(pOriginal, pAdditional, at);
#endif
}

void WIDE_STRING_append_WIDE_STRING(WIDE_STRING *pOriginal, const WIDE_STRING *pAdditional) {
    const size_t characters = wcslen(pOriginal->pWide_string) + wcslen(pAdditional->pWide_string) + 1;
    LPWSTR pString = malloc(sizeof(WCHAR) * characters);
    if (!pString) {
        WINDOWS_HELPER_THROW();
        return;
    }

    wcscpy(pString, pOriginal->pWide_string);
    wcscat(pString, pAdditional->pWide_string);

    MEMORY_HELPER_free((void **)&pOriginal->pWide_string);
    pOriginal->pWide_string = pString;
    WIDE_STRING_update_individual_lines(pOriginal);
}

void WIDE_STRING_remove_last_character(WIDE_STRING *pWide_string) {
    size_t wide_characters = wcslen(pWide_string->pWide_string);
    if (!wide_characters) {
        wide_characters = 1;
    }

    LPWSTR pString = malloc(sizeof(WCHAR) * wide_characters);
    if (!pString) {
        WINDOWS_HELPER_THROW();
        return;
    }

    wcsncpy(pString, pWide_string->pWide_string, wide_characters - 1);
    pString[wide_characters - 1] = L'\0';

    MEMORY_HELPER_free((void **)&pWide_string->pWide_string);
    pWide_string->pWide_string = pString;
    WIDE_STRING_update_individual_lines(pWide_string);
}

void WIDE_STRING_remove_character_at(WIDE_STRING *pWide_string, const size_t at) {
    size_t wide_characters = wcslen(pWide_string->pWide_string);
    LPWSTR pNew = malloc(sizeof(WCHAR) * wide_characters);
    if (!pNew) {
        WINDOWS_HELPER_THROW();
        return;
    }

    for (size_t i = 0; i < at; ++i) {
        pNew[i] = pWide_string->pWide_string[i];
    }

    for (size_t i = at + 1; i < wide_characters; ++i) {
        pNew[i - 1] = pWide_string->pWide_string[i];
    }
    pNew[wide_characters - 1] = L'\0';

    MEMORY_HELPER_free((void **)&pWide_string->pWide_string);
    pWide_string->pWide_string = pNew;
    WIDE_STRING_update_individual_lines(pWide_string);
}

static void WIDE_STRING_at_to_at_line_clean_up(WIDE_STRING *pOriginal, WIDE_STRING *wide_line, const size_t line, LPWSTR pClone) {
    MEMORY_HELPER_free((void **)&pOriginal->individual_lines.pArray[line]);
    WIDE_STRING_destroy(wide_line);

    pOriginal->individual_lines.pArray[line] = pClone;
    WIDE_STRING_consolidate_individual_lines(pOriginal);
}

void WIDE_STRING_append_wide_char_at_line(WIDE_STRING *pOriginal, const WCHAR additional, const size_t line, const size_t at) {
    WIDE_STRING wide_line = WIDE_STRING_create_w(pOriginal->individual_lines.pArray[line]);
    WIDE_STRING_append_wide_char_at(&wide_line, additional, at);

    LPWSTR pClone = STRING_UTILITIES_clone_w_to_w(wide_line.pWide_string);
    WIDE_STRING_at_to_at_line_clean_up(pOriginal, &wide_line, line, pClone);
}

void WIDE_STRING_append_string_a_at_line(WIDE_STRING *pOriginal, LPCSTR pAdditional, const size_t line, const size_t at) {
    WIDE_STRING wide_line = WIDE_STRING_create_w(pOriginal->individual_lines.pArray[line]);
    WIDE_STRING_append_string_a_at(&wide_line, pAdditional, at);

    LPWSTR pClone = STRING_UTILITIES_clone_w_to_w(wide_line.pWide_string);
    WIDE_STRING_at_to_at_line_clean_up(pOriginal, &wide_line, line, pClone);
}

void WIDE_STRING_append_string_w_at_line(WIDE_STRING *pOriginal, LPCWSTR pAdditional, const size_t line, const size_t at) {
    WIDE_STRING wide_line = WIDE_STRING_create_w(pOriginal->individual_lines.pArray[line]);
    WIDE_STRING_append_string_w_at(&wide_line, pAdditional, at);

    LPWSTR pClone = STRING_UTILITIES_clone_w_to_w(wide_line.pWide_string);
    WIDE_STRING_at_to_at_line_clean_up(pOriginal, &wide_line, line, pClone);
}

void WIDE_STRING_append_string_at_line(WIDE_STRING *pOriginal, LPCTSTR pAdditional, const size_t line, const size_t at) {
#ifdef UNICODE
    WIDE_STRING_append_string_w_at_line(pOriginal, pAdditional, line, at);
#else
    WIDE_STRING_append_string_a_at_line(pOriginal, pAdditional, line, at);
#endif
}

void WIDE_STRING_remove_character_at_line(WIDE_STRING *pOriginal, const size_t line, const size_t at) {
    WIDE_STRING wide_line = WIDE_STRING_create_w(pOriginal->individual_lines.pArray[line]);
    WIDE_STRING_remove_character_at(&wide_line, at);

    LPWSTR pClone = STRING_UTILITIES_clone_w_to_w(wide_line.pWide_string);
    WIDE_STRING_at_to_at_line_clean_up(pOriginal, &wide_line, line, pClone);
}

void WIDE_STRING_consolidate_individual_lines(WIDE_STRING *pWide_string) {
    size_t characters = 0;
    for (size_t i = 0; i < pWide_string->individual_lines.size; ++i) {
        characters += wcslen(pWide_string->individual_lines.pArray[i]) + 1;
    }

    LPWSTR pNew = malloc(sizeof(WCHAR) * characters);
    if (!pNew) {
        WINDOWS_HELPER_THROW();
        return;
    }
    pNew[0] = L'\0';

    for (size_t i = 0; i < pWide_string->individual_lines.size; ++i) {
        wcscat(pNew, pWide_string->individual_lines.pArray[i]);

        if (i != pWide_string->individual_lines.size - 1) {
            wcscat(pNew, L"\n");
        }
    }

    WIDE_STRING_destroy(pWide_string);
    pWide_string->pWide_string = pNew;
    WIDE_STRING_update_individual_lines(pWide_string);
}

void WIDE_STRING_destroy(WIDE_STRING *pWide_string) {
    MEMORY_HELPER_free((void **)&pWide_string->pWide_string);

    for (size_t i = 0; i < pWide_string->individual_lines.size; ++i) {
        MEMORY_HELPER_free((void **)&pWide_string->individual_lines.pArray[i]);
    }
    VECTOR_DESTROY_LPWSTR(&pWide_string->individual_lines);
}