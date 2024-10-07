#ifndef WINDOWS_HELPER_H
#define WINDOWS_HELPER_H

#include <Windows.h>
#include <string_utilities.h>

void WINDOWS_HELPER_error_a(LPCSTR pContents);
void WINDOWS_HELPER_error_w(LPCWSTR pContents);
void WINDOWS_HELPER_error_ws(WIDE_STRING *pContents);
void WINDOWS_HELPER_error(LPCTSTR pContents);
#endif