#include <leak_checker.h>

#include <resource.h>
#include <memory_helper.h>
#include <windows_helper.h>

#ifndef WINDOWS_HELPER_EXPAND
#define WINDOWS_HELPER_EXPAND(macro) #macro
#endif

#ifndef WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION
#define WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(function, library) \
function##_fetched function##_saved = NULL; \
function##_fetched WINDOWS_HELPER_get_##function(void) { \
    if (!function##_saved) { \
        function##_saved = (function##_fetched)WINDOWS_HELPER_get_function(TEXT(library), WINDOWS_HELPER_EXPAND(function)); \
    } \
    return function##_saved; \
}
#endif

BYTE WINDOWS_HELPER_document_type = MDI, WINDOWS_HELPER_style = CLASSIC;

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
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(ImmGetContext, "imm32.dll")
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(ImmGetCompositionString, "imm32.dll")
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(ImmReleaseContext, "imm32.dll")
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(OpenThemeData, "uxtheme.dll")
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(DrawThemeBackground, "uxtheme.dll")
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(CloseThemeData, "uxtheme.dll")
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(DwmEnableBlurBehindWindow, "dwmapi.dll")
WINDOWS_HELPER_GET_FUNCTION_IMPLEMENTATION(DwmGetColorizationColor, "dwmapi.dll")

void WINDOWS_HELPER_set_types(void) {
    const DWORD version = GetVersion(),
                loword = LOWORD(version),
                major = LOBYTE(loword),
                minor = HIBYTE(loword);

    if (major <= 2 || major == 4 || (major == 5 && minor == 0)) {
        WINDOWS_HELPER_document_type = SDI;
    } else if ((major == 5 && minor >= 1) || major >= 6) {
        WINDOWS_HELPER_document_type = TDI;
    }

    if (major == 6) {
        if (minor <= 1) {
            WINDOWS_HELPER_style = AERO;
        } else if (minor >= 2) {
            WINDOWS_HELPER_style = METRO;
        }
    }
}

BOOL WINDOWS_HELPER_file_exists(LPCTSTR file) {
    const DWORD attributes = GetFileAttributes(file);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

HANDLE WINDOWS_HELPER_file_dialog(const HWND owner, const BOOL is_open, WIDE_STRING *pFile_name) {
    const DWORD size = sizeof(TCHAR) * 32767;
    LPTSTR pFile_name_t = malloc(size);
    if (!pFile_name_t) {
        return INVALID_HANDLE_VALUE;
    }
    memset(pFile_name_t, 0, size);

    if (!is_open) {
        const TCHAR default_name[] = TEXT("*.txt");
        memcpy(pFile_name, default_name, sizeof(default_name));
    }

    OPENFILENAME open_file_name = { 0 };
    open_file_name.lStructSize = 76;
    open_file_name.hwndOwner = owner;
    open_file_name.lpstrFilter = TEXT("Text Files (.txt)\0*.txt\0All Files\0*.*\0\0");
    open_file_name.nFilterIndex = 1;
    open_file_name.lpstrFile = pFile_name_t;
    open_file_name.nMaxFile = size;
    open_file_name.Flags = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;

    BOOL (WINAPI *file_dialog)(LPOPENFILENAME unnamedParam1) = &GetOpenFileName;
    if (!is_open) {
        file_dialog = &GetSaveFileName;
    }

    HANDLE file = INVALID_HANDLE_VALUE;
    if (file_dialog(&open_file_name)) {
        DWORD access_type = GENERIC_READ, create_type = OPEN_EXISTING;
        if (!is_open) {
            create_type = TRUNCATE_EXISTING;
            access_type = GENERIC_WRITE;
        }
        if (!WINDOWS_HELPER_file_exists(open_file_name.lpstrFile)) {
            create_type = CREATE_NEW;
        }

        file = CreateFile(open_file_name.lpstrFile,
                          access_type,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          create_type,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

        *pFile_name = WIDE_STRING_create(pFile_name_t);
        WIDE_STRING_extract_file_name_from_path(pFile_name);
    }

    MEMORY_HELPER_free((void **)&pFile_name_t);
    return file;
}

void WINDOWS_HELPER_warning(LPCTSTR pContents) {
    MessageBox(NULL, pContents, TEXT("Warning"), MB_OK | MB_ICONWARNING);
}

static void WINDOWS_HELPER_error_clean_up(WIDE_STRING *pContents) {
    WINDOWS_HELPER_error_ws(pContents);
    WIDE_STRING_destroy(pContents);
    WINDOWS_HELPER_THROW();
}

void WINDOWS_HELPER_error_a(LPCSTR pContents) {
    WIDE_STRING string = WIDE_STRING_create_a(pContents);
    WINDOWS_HELPER_error_clean_up(&string);
}

void WINDOWS_HELPER_error_w(LPCWSTR pContents) {
    WIDE_STRING string = WIDE_STRING_create_w(pContents);
    WINDOWS_HELPER_error_clean_up(&string);
}

void WINDOWS_HELPER_error_ws(WIDE_STRING *pContents) {
    LPCTSTR pContents_t = WIDE_STRING_get_t_string(pContents);
    MessageBox(NULL, pContents_t, TEXT("Error"), MB_OK | MB_ICONERROR);
    MEMORY_HELPER_free((void **)&pContents_t);
}

void WINDOWS_HELPER_error(LPCTSTR pContents) {
    WIDE_STRING string = WIDE_STRING_create(pContents);
    WINDOWS_HELPER_error_clean_up(&string);
}