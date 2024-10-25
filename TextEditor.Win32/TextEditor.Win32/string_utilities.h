#ifndef UNIFIEDSTRING_H
#define UNIFIEDSTRING_H

#include <stdint.h>
#include <signal.h>
#include <Windows.h>
#include <vector.h>

#ifndef STRING_UTILITIES_THROW
#define STRING_UTILITIES_THROW() raise(SIGABRT);
#endif

LPTSTR STRING_UTILITIES_wide_to_T(LPCWSTR pWide_string);
LPWSTR STRING_UTILITIES_ANSI_to_wide(LPCSTR pANSI_string);
LPSTR STRING_UTILITIES_wide_to_ANSI(LPCWSTR pWide_string);

LPWSTR STRING_UTILITIES_clone_wide_to_wide(LPCWSTR pOriginal);

typedef struct _WIDE_STRING {
    LPWSTR pWide_string;
    VECTOR_LPWSTR individual_lines;
} WIDE_STRING;

void WIDE_STRING_initialize(WIDE_STRING *pWide_string);

WIDE_STRING WIDE_STRING_create_empty(void);
WIDE_STRING WIDE_STRING_create_a(LPCSTR pANSI_string);
WIDE_STRING WIDE_STRING_create_w(LPCWSTR pWide_string);
WIDE_STRING WIDE_STRING_create(LPCTSTR pT_string);

LPCTSTR WIDE_STRING_get_T_string(WIDE_STRING *pWide_string);

void WIDE_STRING_append_wide_char(WIDE_STRING *pOriginal, const WCHAR additional);
void WIDE_STRING_append_wide_char_at(WIDE_STRING *pOriginal, const WCHAR additional, const size_t at);
void WIDE_STRING_append_string_a(WIDE_STRING *pOriginal, LPCSTR pAdditional);
void WIDE_STRING_append_string_a_at(WIDE_STRING *pOriginal, LPCSTR pAdditional, const size_t at);
void WIDE_STRING_append_string_w(WIDE_STRING *pOriginal, LPCWSTR pAdditional);
void WIDE_STRING_append_string(WIDE_STRING *pOriginal, LPCTSTR pAdditional);
void WIDE_STRING_append_WIDE_STRING(WIDE_STRING *pOriginal, const WIDE_STRING *pAdditional);
void WIDE_STRING_remove_last_character(WIDE_STRING *pWide_string);
void WIDE_STRING_remove_character_at(WIDE_STRING *pWide_string, const size_t at);

void WIDE_STRING_append_wide_char_at_line(WIDE_STRING *pOriginal, const WCHAR additional, const size_t line, const size_t at);
void WIDE_STRING_append_string_a_at_line(WIDE_STRING *pOriginal, LPCSTR pAdditional, const size_t line, const size_t at);
void WIDE_STRING_remove_character_at_line(WIDE_STRING *pOriginal, const size_t line, const size_t at);

void WIDE_STRING_consolidate_individual_lines(WIDE_STRING *pWide_string);

void WIDE_STRING_destroy(WIDE_STRING *pWide_string);
#endif