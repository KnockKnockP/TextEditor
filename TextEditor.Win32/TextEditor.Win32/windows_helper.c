#include <memory_helper.h>
#include <windows_helper.h>

#ifndef WINDOWS_HELPER_ERROR_CAPTION
#define WINDOWS_HELPER_ERROR_CAPTION "Error"
#endif

AddFontResourceEx_fetched AddFontResourceEx_saved = NULL;
RemoveFontResourceEx_fetched RemoveFontResourceEx_saved = NULL;

FARPROC WINDOWS_HELPER_get_function(LPCTSTR pLibrary_name, LPCSTR pFunction_name) {
    const HMODULE library = LoadLibrary(pLibrary_name);
    if (!library) {
        return NULL;
    }

    FARPROC function = GetProcAddress(library, pFunction_name);
    if (!function) {
        return NULL;
    }

    return function;
}

AddFontResourceEx_fetched WINDOWS_HELPER_get_AddFontResourceEx(void) {
    if (!AddFontResourceEx_saved) {
        AddFontResourceEx_saved = (AddFontResourceEx_fetched)WINDOWS_HELPER_get_function(TEXT("gdi32.dll"),
#ifdef UNICODE
            "AddFontResourceExW"
#else
            "AddFontResourceExA"
#endif
        );
    }

    return AddFontResourceEx_saved;
}

RemoveFontResourceEx_fetched WINDOWS_HELPER_get_RemoveFontResourceEx(void) {
    if (!RemoveFontResourceEx_saved) {
        RemoveFontResourceEx_saved = (RemoveFontResourceEx_fetched)WINDOWS_HELPER_get_function(TEXT("gdi32.dll"),
#ifdef UNICODE
            "RemoveFontResourceExW"
#else
            "RemoveFontResourceExA"
#endif
        );
    }

    return RemoveFontResourceEx_saved;
}

static void WINDOWS_HELPER_clean_up(WIDE_STRING *pContents) {
    WINDOWS_HELPER_error_ws(pContents);
    WIDE_STRING_destroy(pContents);
}

void WINDOWS_HELPER_error_a(LPCSTR pContents) {
    WIDE_STRING string = WIDE_STRING_create_a(pContents);
    WINDOWS_HELPER_clean_up(&string);
}

void WINDOWS_HELPER_error_w(LPCWSTR pContents) {
    WIDE_STRING string = WIDE_STRING_create_w(pContents);
    WINDOWS_HELPER_clean_up(&string);
}

void WINDOWS_HELPER_error_ws(WIDE_STRING *pContents) {
    LPCTSTR pContents_t = WIDE_STRING_get_T_string(pContents);
    MessageBox(NULL, pContents_t, TEXT(WINDOWS_HELPER_ERROR_CAPTION), MB_OK | MB_ICONERROR);
    MEMORY_HELPER_free((void **)&pContents_t);
}

void WINDOWS_HELPER_error(LPCTSTR pContents) {
    WIDE_STRING string = WIDE_STRING_create(pContents);
    WINDOWS_HELPER_clean_up(&string);
}