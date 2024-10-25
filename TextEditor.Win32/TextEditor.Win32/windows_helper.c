#include <memory_helper.h>
#include <windows_helper.h>

#ifndef WINDOWS_HELPER_ERROR_CAPTION
#define WINDOWS_HELPER_ERROR_CAPTION "Error"
#endif

#ifndef WINDOWS_HELPER_EXPAND
#define WINDOWS_HELPER_EXPAND(macro) #macro
#endif

#ifndef WINDOWS_HELPER_EXTERN_SET
#define WINDOWS_HELPER_EXTERN_SET(function) function##_fetched function##_saved = NULL
#endif

#ifndef WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION
#define WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(function, library) \
function##_fetched WINDOWS_HELPER_get_##function(void) { \
    if (!function##_saved) { \
        function##_saved = (function##_fetched)WINDOWS_HELPER_get_function(TEXT(library), WINDOWS_HELPER_EXPAND(function)); \
    } \
    return function##_saved; \
}
#endif

WINDOWS_HELPER_EXTERN_SET(AddFontResourceEx);
WINDOWS_HELPER_EXTERN_SET(RemoveFontResourceEx);

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

WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(AddFontResourceEx, "gdi32.dll")
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(RemoveFontResourceEx, "gdi32.dll")

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