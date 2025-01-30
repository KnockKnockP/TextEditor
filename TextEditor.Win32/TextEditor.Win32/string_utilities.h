#ifndef UNIFIEDSTRING_H
#define UNIFIEDSTRING_H

#include <vector.h>
#include <Windows.h>

typedef enum _STRING_UTILITIES_ENCODING {
    STRING_UTILITIES_ENCODING_NONE = 0,
    ANSI = 1,
    WIDE = 2, //UTF-16 LE with BOM
    UTF8 = 3,
    UTF8_WITH_BOM = 4
} STRING_UTILITIES_ENCODING;

int STRING_UTILITIES_detect_encoding(const BYTE * const pBytes, const size_t size);
LPTSTR STRING_UTILITIES_encoding_enum_to_string(const int encoding_enum);

LPTSTR STRING_UTILITIES_w_to_t(LPCWSTR pWide_string);
LPWSTR STRING_UTILITIES_a_to_w(LPCSTR pANSI_string);
LPSTR STRING_UTILITIES_w_to_a(LPCWSTR pWide_string);
LPWSTR STRING_UTILITIES_UTF8_to_w(LPCSTR pUTF8_string);

LPWSTR STRING_UTILITIES_clone_w_to_w(LPCWSTR pOriginal);

size_t STRING_UTILITIES_characters(LPCTSTR pString);

typedef struct _WIDE_STRING {
    LPWSTR pWide_string;
    VECTOR_LPWSTR individual_lines;
} WIDE_STRING;

void WIDE_STRING_initialize(WIDE_STRING *pWide_string);

WIDE_STRING WIDE_STRING_create_empty(void);
WIDE_STRING WIDE_STRING_create_a(LPCSTR pANSI_string);
WIDE_STRING WIDE_STRING_create_w(LPCWSTR pWide_string);
WIDE_STRING WIDE_STRING_create(LPCTSTR pT_string);

LPCTSTR WIDE_STRING_get_t_string(WIDE_STRING *pWide_string);

void WIDE_STRING_append_wide_char(WIDE_STRING *pOriginal, const WCHAR additional);
void WIDE_STRING_append_wide_char_at(WIDE_STRING *pOriginal, const WCHAR additional, const size_t at);
void WIDE_STRING_append_string_a(WIDE_STRING *pOriginal, LPCSTR pAdditional);
void WIDE_STRING_append_string_a_at(WIDE_STRING *pOriginal, LPCSTR pAdditional, const size_t at);
void WIDE_STRING_append_string_w(WIDE_STRING *pOriginal, LPCWSTR pAdditional);
void WIDE_STRING_append_string_w_at(WIDE_STRING *pOriginal, LPCWSTR pAdditional, const size_t at);
void WIDE_STRING_append_string(WIDE_STRING *pOriginal, LPCTSTR pAdditional);
void WIDE_STRING_append_string_at(WIDE_STRING *pOriginal, LPCTSTR pAdditional, const size_t at);
void WIDE_STRING_append_WIDE_STRING(WIDE_STRING *pOriginal, const WIDE_STRING *pAdditional);
void WIDE_STRING_remove_last_character(WIDE_STRING *pWide_string);
void WIDE_STRING_remove_character_at(WIDE_STRING *pWide_string, const size_t at);

void WIDE_STRING_append_wide_char_at_line(WIDE_STRING *pOriginal, const WCHAR additional, const size_t line, const size_t at);
void WIDE_STRING_append_string_a_at_line(WIDE_STRING *pOriginal, LPCSTR pAdditional, const size_t line, const size_t at);
void WIDE_STRING_append_string_w_at_line(WIDE_STRING *pOriginal, LPCWSTR pAdditional, const size_t line, const size_t at);
void WIDE_STRING_append_string_at_line(WIDE_STRING *pOriginal, LPCTSTR pAdditional, const size_t line, const size_t at);
void WIDE_STRING_remove_character_at_line(WIDE_STRING *pOriginal, const size_t line, const size_t at);

void WIDE_STRING_consolidate_individual_lines(WIDE_STRING *pWide_string);
void WIDE_STRING_extract_file_name_from_path(WIDE_STRING *pWide_string);

void WIDE_STRING_destroy(WIDE_STRING *pWide_string);
#endif