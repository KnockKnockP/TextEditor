#include <string_utilities.h>
#include <stdio.h>
#include <memory_helper.h>

LPWSTR STRING_UTILITIES_ANSI_to_wide(LPCSTR pANSI_string) {
    const int wide_characters = MultiByteToWideChar(CP_ACP, 0, pANSI_string, -1, NULL, 0);
    LPWSTR pUTF_16_String = malloc(sizeof(WCHAR) * wide_characters);
    if (!pUTF_16_String) {
        STRING_UTILITIES_THROW();
        return NULL;
    }

    if (!MultiByteToWideChar(CP_ACP, 0, pANSI_string, -1, pUTF_16_String, wide_characters)) {
        STRING_UTILITIES_THROW();
        return NULL;
    }
    return pUTF_16_String;
}

LPSTR STRING_UTILITIES_wide_to_ANSI(LPCWSTR pWide_string) {
    const int ANSI_characters = WideCharToMultiByte(CP_ACP, 0, pWide_string, -1, NULL, 0, NULL, NULL);
    CHAR *pANSI_string = malloc(sizeof(CHAR) * ANSI_characters);

    if (!WideCharToMultiByte(CP_ACP, 0, pWide_string, -1, pANSI_string, ANSI_characters, NULL, NULL)) {
        STRING_UTILITIES_THROW();
    }
    return pANSI_string;
}

void WIDE_STRING_initialize(WIDE_STRING *pWide_string) {
    pWide_string->pWide_string = NULL;
}

WIDE_STRING WIDE_STRING_create_empty(void) {
    WIDE_STRING wide_string = { 0 };
    WIDE_STRING_initialize(&wide_string);

    LPWSTR pString = malloc(sizeof(WCHAR));
    if (!pString) {
        return wide_string;
    }

    pString[0] = L'\0';
    wide_string.pWide_string = pString;
    return wide_string;
}

WIDE_STRING WIDE_STRING_create_a(LPCSTR pANSI_string) {
    WIDE_STRING wide_string = { 0 };
    WIDE_STRING_initialize(&wide_string);

    wide_string.pWide_string = STRING_UTILITIES_ANSI_to_wide(pANSI_string);
    return wide_string;
}

WIDE_STRING WIDE_STRING_create_w(LPCWSTR pWide_string) {
    WIDE_STRING wide_string = { 0 };
    WIDE_STRING_initialize(&wide_string);

    const size_t wide_characters = wcslen(pWide_string) + 1;
    LPWSTR pString = malloc(sizeof(WCHAR) * wide_characters);
    if (!pString) {
        STRING_UTILITIES_THROW();
        return wide_string;
    }

    wcscpy(pString, pWide_string);
    wide_string.pWide_string = pString;
    return wide_string;
}

WIDE_STRING WIDE_STRING_create(LPCTSTR pT_string) {
#ifdef UNICODE
    return WIDE_STRING_create_w(pT_string);
#else
    return WIDE_STRING_create_a(pT_string);
#endif
}

LPCTSTR WIDE_STRING_get_T_string(WIDE_STRING *pWide_string) {
#ifdef UNICODE
    const int characters = wcslen(pWide_string->pWide_string);
    LPWSTR temporary = malloc(sizeof(WCHAR) * (characters + 1));
    if (!temporary) {
        return NULL;
    }

    wcscpy(temporary, pWide_string->pWide_string);
    return temporary;
#else
    return STRING_UTILITIES_wide_to_ANSI(pWide_string->pWide_string);
#endif
}

void WIDE_STRING_append_wide_char(WIDE_STRING *pOriginal, const WCHAR additional) {
    const size_t characters = wcslen(pOriginal->pWide_string) + 2;
    LPWSTR pString = malloc(sizeof(WCHAR) * characters);
    if (!pString) {
        return;
    }

    wcscpy(pString, pOriginal->pWide_string);
    pString[characters - 2] = additional;
    pString[characters - 1] = L'\0';

    WIDE_STRING_destroy(pOriginal);
    pOriginal->pWide_string = pString;
}

void WIDE_STRING_append_string_a(WIDE_STRING *pOriginal, LPCSTR pAdditional) {
    WIDE_STRING additional_wide = WIDE_STRING_create_a(pAdditional);

    const size_t characters = wcslen(pOriginal->pWide_string) + wcslen(additional_wide.pWide_string) + 1;
    LPWSTR pString = malloc(sizeof(WCHAR) * characters);
    if (!pString) {
        return;
    }

    wcscpy(pString, pOriginal->pWide_string);
    wcscat(pString, additional_wide.pWide_string);

    WIDE_STRING_destroy(pOriginal);
    WIDE_STRING_destroy(&additional_wide);
    pOriginal->pWide_string = pString;
}

void WIDE_STRING_append_string_w(WIDE_STRING *pOriginal, LPCWSTR pAdditional) {
    WIDE_STRING additional_wide = WIDE_STRING_create_w(pAdditional);
    WIDE_STRING_append_WIDE_STRING(pOriginal, &additional_wide);
    WIDE_STRING_destroy(&additional_wide);
}

void WIDE_STRING_append_string(WIDE_STRING *pOriginal, LPCTSTR pAdditional) {
#ifdef UNICODE
    WIDE_STRING_append_string_w(pOriginal, pAdditional);
#else
    WIDE_STRING_append_string_a(pOriginal, pAdditional);
#endif
}

void WIDE_STRING_append_WIDE_STRING(WIDE_STRING *pOriginal, const WIDE_STRING *pAdditional) {
    const size_t characters = wcslen(pOriginal->pWide_string) + wcslen(pAdditional->pWide_string) + 1;
    LPWSTR pString = malloc(sizeof(WCHAR) * characters);
    if (!pString) {
        return;
    }

    wcscpy(pString, pOriginal->pWide_string);
    wcscat(pString, pAdditional->pWide_string);

    WIDE_STRING_destroy(pOriginal);
    pOriginal->pWide_string = pString;
}

void WIDE_STRING_remove_last_character(WIDE_STRING *pWide_string) {
    size_t wide_characters = wcslen(pWide_string->pWide_string);
    if (!wide_characters) {
        wide_characters = 1;
    }

    LPWSTR pString = malloc(sizeof(WCHAR) * wide_characters);
    if (!pString) {
        return;
    }

    wcsncpy(pString, pWide_string->pWide_string, wide_characters - 1);
    pString[wide_characters - 1] = L'\0';

    WIDE_STRING_destroy(pWide_string);
    pWide_string->pWide_string = pString;
}

void WIDE_STRING_destroy(WIDE_STRING *pWide_string) {
    MEMORY_HELPER_free((void **)&pWide_string->pWide_string);
}