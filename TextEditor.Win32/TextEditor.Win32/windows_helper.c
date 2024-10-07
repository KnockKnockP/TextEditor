#include <windows_helper.h>

#ifndef WINDOWS_HELPER_ERROR_CAPTION
#define WINDOWS_HELPER_ERROR_CAPTION "Error"
#endif

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
    MessageBox(NULL, WIDE_STRING_get_T_string(pContents), TEXT(WINDOWS_HELPER_ERROR_CAPTION), MB_OK | MB_ICONERROR);
}

void WINDOWS_HELPER_error(LPCTSTR pContents) {
    WIDE_STRING string = WIDE_STRING_create(pContents);
    WINDOWS_HELPER_clean_up(&string);
}