#ifndef WINDOWS_HELPER_H
#define WINDOWS_HELPER_H

#include <Windows.h>
#include <string_utilities.h>

typedef int (WINAPI *AddFontResourceEx_fetched)(LPCTSTR name, DWORD fl, PVOID res);
typedef int (WINAPI *RemoveFontResourceEx_fetched)(LPCTSTR name, DWORD fl, PVOID pdv);

extern AddFontResourceEx_fetched AddFontResourceEx_saved;
extern RemoveFontResourceEx_fetched RemoveFontResourceEx_saved;

FARPROC WINDOWS_HELPER_get_function(LPCTSTR pLibrary_name, LPCSTR pFunction_name);

AddFontResourceEx_fetched WINDOWS_HELPER_get_AddFontResourceEx(void);
RemoveFontResourceEx_fetched WINDOWS_HELPER_get_RemoveFontResourceEx(void);

void WINDOWS_HELPER_error_a(LPCSTR pContents);
void WINDOWS_HELPER_error_w(LPCWSTR pContents);
void WINDOWS_HELPER_error_ws(WIDE_STRING *pContents);
void WINDOWS_HELPER_error(LPCTSTR pContents);
#endif