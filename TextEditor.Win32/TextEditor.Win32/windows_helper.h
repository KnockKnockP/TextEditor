#ifndef WINDOWS_HELPER_H
#define WINDOWS_HELPER_H

#include <stdint.h>
#include <signal.h>
#include <Windows.h>
#include <string_utilities.h>

#ifndef WINDOWS_HELPER_THROW
#define WINDOWS_HELPER_THROW() raise(SIGABRT)
//#define WINDOWS_HELPER_THROW() for (;;) {}
#endif

#ifndef WINDOWS_HELPER_GET_FUNCTION_DECLARATION
#define WINDOWS_HELPER_GET_FUNCTION_DECLARATION(function) \
extern function##_fetched function##_saved; \
function##_fetched WINDOWS_HELPER_get_##function(void)
#endif

typedef int (WINAPI *AddFontResourceEx_fetched)(LPCTSTR name, DWORD fl, PVOID res);
typedef BOOL (WINAPI *RemoveFontResourceEx_fetched)(LPCTSTR name, DWORD fl, PVOID pdv);
typedef HIMC (WINAPI *ImmGetContext_fetched)(HWND unnamedParam1);
typedef LONG (WINAPI *ImmGetCompositionString_fetched)(HIMC unnamedParam1, DWORD unnamedParam2, LPVOID lpBuf, DWORD dwBufLen);
typedef BOOL (WINAPI *ImmReleaseContext_fetched)(HWND unnamedParam1, HIMC unnamedParam2);

FARPROC WINDOWS_HELPER_get_function(LPCTSTR pLibrary_name, LPCSTR pFunction_name);

WINDOWS_HELPER_GET_FUNCTION_DECLARATION(AddFontResourceEx);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(RemoveFontResourceEx);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(ImmGetContext);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(ImmGetCompositionString);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(ImmReleaseContext);

BOOL WINDOWS_HELPER_file_exists(LPCTSTR file);

void WINDOWS_HELPER_error_a(LPCSTR pContents);
void WINDOWS_HELPER_error_w(LPCWSTR pContents);
void WINDOWS_HELPER_error_ws(WIDE_STRING *pContents);
void WINDOWS_HELPER_error(LPCTSTR pContents);
#endif