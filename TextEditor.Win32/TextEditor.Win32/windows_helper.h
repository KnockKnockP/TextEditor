#ifndef WINDOWS_HELPER_H
#define WINDOWS_HELPER_H

#include <stdint.h>
#include <signal.h>
#include <dwmapi.h>
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

#ifndef WINDOWS_HELPER_TRANSPARENT_RGB
#define WINDOWS_HELPER_TRANSPARENT_RGB RGB(200, 201, 202)
#endif

typedef enum _WINDOWS_HELPER_DOCUMENT_INTERFACE {
    WINDOWS_HELPER_DOCUMENT_INTERFACE_NONE,
    MDI, //Windows 3.0 ~ 3.2, NT 3.1 ~ NT 3.51
    SDI, //Windows 1.0 ~ 2.12, 95 ~ ME, NT 4.0 ~ 2000
    TDI //Windows XP ~ Windows 11
} WINDOWS_HELPER_DOCUMENT_INTERFACE;

typedef enum _WINDOWS_HELPER_STYLE {
    WINDOWS_HELPER_DOCUMENT_STYLE_NONE,
    CLASSIC, //Windows 1.0 ~ ME, NT 3.1 ~ XP
    AERO, //Windows Vista ~ 7,
    METRO //Also known as second worst design known to man
} WINDOWS_HELPER_STYLE;

typedef int (WINAPI *AddFontResourceEx_fetched)(LPCTSTR name, DWORD fl, PVOID res);
typedef BOOL (WINAPI *RemoveFontResourceEx_fetched)(LPCTSTR name, DWORD fl, PVOID pdv);
typedef HIMC (WINAPI *ImmGetContext_fetched)(HWND unnamedParam1);
typedef LONG (WINAPI *ImmGetCompositionString_fetched)(HIMC unnamedParam1, DWORD unnamedParam2, LPVOID lpBuf, DWORD dwBufLen);
typedef BOOL (WINAPI *ImmReleaseContext_fetched)(HWND unnamedParam1, HIMC unnamedParam2);
typedef HRESULT (WINAPI *DwmEnableBlurBehindWindow_fetched)(HWND hWnd, const DWM_BLURBEHIND *pBlurBehind);
typedef HRESULT (WINAPI *DwmGetColorizationColor_fetched)(DWORD *pcrColorization, BOOL *pfOpaqueBlend);

extern BYTE WINDOWS_HELPER_document_type, WINDOWS_HELPER_style;

FARPROC WINDOWS_HELPER_get_function(LPCTSTR pLibrary_name, LPCSTR pFunction_name);

WINDOWS_HELPER_GET_FUNCTION_DECLARATION(AddFontResourceEx);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(RemoveFontResourceEx);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(ImmGetContext);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(ImmGetCompositionString);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(ImmReleaseContext);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(DwmEnableBlurBehindWindow);
WINDOWS_HELPER_GET_FUNCTION_DECLARATION(DwmGetColorizationColor);

void WINDOWS_HELPER_set_types(void);

BOOL WINDOWS_HELPER_file_exists(LPCTSTR file);
HANDLE WINDOWS_HELPER_file_dialog(const HWND owner, const BOOL is_open);

void WINDOWS_HELPER_warning(LPCTSTR pContents);

void WINDOWS_HELPER_error_a(LPCSTR pContents);
void WINDOWS_HELPER_error_w(LPCWSTR pContents);
void WINDOWS_HELPER_error_ws(WIDE_STRING *pContents);
void WINDOWS_HELPER_error(LPCTSTR pContents);
#endif