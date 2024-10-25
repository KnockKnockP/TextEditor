#ifndef WINDOWS_HELPER_H
#define WINDOWS_HELPER_H

#include <Windows.h>
#include <string_utilities.h>

#ifndef WINDOWS_HELPER_EXTERN_FUNCTION
#define WINDOWS_HELPER_EXTERN_FUNCTION(function) extern function##_fetched function##_saved
#endif

#ifndef WINDOWS_HELPER_GET_FUNCTION_DECLARATION
#define WINDOWS_HELPER_GET_FUNCTION_DECLARATION(function) function##_fetched WINDOWS_HELPER_get_##function(void)
#endif

typedef int (WINAPI *AddFontResourceEx_fetched)(LPCTSTR name, DWORD fl, PVOID res);
typedef int (WINAPI *RemoveFontResourceEx_fetched)(LPCTSTR name, DWORD fl, PVOID pdv);

WINDOWS_HELPER_EXTERN_FUNCTION(AddFontResourceEx);
WINDOWS_HELPER_EXTERN_FUNCTION(RemoveFontResourceEx);

FARPROC WINDOWS_HELPER_get_function(LPCTSTR pLibrary_name, LPCSTR pFunction_name);

WINDOWS_HELPER_GET_FUNCTION_DECLARATION(AddFontResourceEx);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(RemoveFontResourceEx);

void WINDOWS_HELPER_error_a(LPCSTR pContents);
void WINDOWS_HELPER_error_w(LPCWSTR pContents);
void WINDOWS_HELPER_error_ws(WIDE_STRING *pContents);
void WINDOWS_HELPER_error(LPCTSTR pContents);
#endif