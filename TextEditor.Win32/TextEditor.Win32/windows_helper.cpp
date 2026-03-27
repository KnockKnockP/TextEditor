#include <leak_checker.h>

#include <windows_helper.h>

#include <resource.h>

namespace TextEditor {

namespace {
    #define TEXTEDITOR_LOAD_FUNCTION(member_name, function_name, library_name, type_name) \
    if (!member_name) \
        member_name = reinterpret_cast<type_name>(GetFunction(TEXT(library_name), function_name)); \
    return member_name
}

Win32Api::DocumentInterface Win32Api::document_interface_ = Win32Api::kDocumentMdi;
Win32Api::Style Win32Api::style_ = Win32Api::kStyleClassic;
Win32Api::AddFontResourceExFunction Win32Api::add_font_resource_ex_ = NULL;
Win32Api::RemoveFontResourceExFunction Win32Api::remove_font_resource_ex_ = NULL;
Win32Api::ImmGetContextFunction Win32Api::imm_get_context_ = NULL;
Win32Api::ImmGetCompositionStringFunction Win32Api::imm_get_composition_string_ = NULL;
Win32Api::ImmReleaseContextFunction Win32Api::imm_release_context_ = NULL;
Win32Api::OpenThemeDataFunction Win32Api::open_theme_data_ = NULL;
Win32Api::DrawThemeBackgroundFunction Win32Api::draw_theme_background_ = NULL;
Win32Api::CloseThemeDataFunction Win32Api::close_theme_data_ = NULL;
Win32Api::DwmEnableBlurBehindWindowFunction Win32Api::dwm_enable_blur_behind_window_ = NULL;
Win32Api::DwmGetColorizationColorFunction Win32Api::dwm_get_colorization_color_ = NULL;

void Win32Api::Initialize() {
    const DWORD version = GetVersion();
    const WORD low_word = LOWORD(version);
    const BYTE major = LOBYTE(low_word);
    const BYTE minor = HIBYTE(low_word);

    document_interface_ = kDocumentMdi;
    style_ = kStyleClassic;

    if (major <= 2 || major == 4 || (major == 5 && minor == 0)) {
        document_interface_ = kDocumentSdi;
    } else if ((major == 5 && minor >= 1) || major >= 6) {
        document_interface_ = kDocumentTdi;
    }

    if (major == 6) {
        if (minor <= 1) {
            style_ = kStyleAero;
        } else {
            style_ = kStyleMetro;
        }
    }
}

Win32Api::DocumentInterface Win32Api::document_interface() {
    return document_interface_;
}

Win32Api::Style Win32Api::style() {
    return style_;
}

Win32Api::AddFontResourceExFunction Win32Api::AddFontResourceEx() {
    TEXTEDITOR_LOAD_FUNCTION(add_font_resource_ex_, "AddFontResourceExW", "gdi32.dll", AddFontResourceExFunction);
}

Win32Api::RemoveFontResourceExFunction Win32Api::RemoveFontResourceEx() {
    TEXTEDITOR_LOAD_FUNCTION(remove_font_resource_ex_, "RemoveFontResourceExW", "gdi32.dll", RemoveFontResourceExFunction);
}

Win32Api::ImmGetContextFunction Win32Api::ImmGetContext() {
    TEXTEDITOR_LOAD_FUNCTION(imm_get_context_, "ImmGetContext", "imm32.dll", ImmGetContextFunction);
}

Win32Api::ImmGetCompositionStringFunction Win32Api::ImmGetCompositionString() {
    TEXTEDITOR_LOAD_FUNCTION(imm_get_composition_string_, "ImmGetCompositionStringW", "imm32.dll", ImmGetCompositionStringFunction);
}

Win32Api::ImmReleaseContextFunction Win32Api::ImmReleaseContext() {
    TEXTEDITOR_LOAD_FUNCTION(imm_release_context_, "ImmReleaseContext", "imm32.dll", ImmReleaseContextFunction);
}

Win32Api::OpenThemeDataFunction Win32Api::OpenThemeData() {
    TEXTEDITOR_LOAD_FUNCTION(open_theme_data_, "OpenThemeData", "uxtheme.dll", OpenThemeDataFunction);
}

Win32Api::DrawThemeBackgroundFunction Win32Api::DrawThemeBackground() {
    TEXTEDITOR_LOAD_FUNCTION(draw_theme_background_, "DrawThemeBackground", "uxtheme.dll", DrawThemeBackgroundFunction);
}

Win32Api::CloseThemeDataFunction Win32Api::CloseThemeData() {
    TEXTEDITOR_LOAD_FUNCTION(close_theme_data_, "CloseThemeData", "uxtheme.dll", CloseThemeDataFunction);
}

Win32Api::DwmEnableBlurBehindWindowFunction Win32Api::DwmEnableBlurBehindWindow() {
    TEXTEDITOR_LOAD_FUNCTION(dwm_enable_blur_behind_window_, "DwmEnableBlurBehindWindow", "dwmapi.dll", DwmEnableBlurBehindWindowFunction);
}

Win32Api::DwmGetColorizationColorFunction Win32Api::DwmGetColorizationColor() {
    TEXTEDITOR_LOAD_FUNCTION(dwm_get_colorization_color_, "DwmGetColorizationColor", "dwmapi.dll", DwmGetColorizationColorFunction);
}

bool Win32Api::FileExists(LPCTSTR path) {
    const DWORD attributes = GetFileAttributes(path);
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

HANDLE Win32Api::ShowFileDialog(HWND owner, bool open_file, WideString *file_name) {
    const DWORD max_file_characters = 32767;
    TCHAR *buffer = new TCHAR[max_file_characters];
    memset(buffer, 0, sizeof(TCHAR) * max_file_characters);

    if (!open_file) {
        const TCHAR default_name[] = TEXT("*.txt");
        memcpy(buffer, default_name, sizeof(default_name));
    }

    OPENFILENAME dialog = { 0 };
    dialog.lStructSize = 76;
    dialog.hwndOwner = owner;
    dialog.lpstrFilter = TEXT("Text Files (.txt)\0*.txt\0All Files\0*.*\0\0");
    dialog.nFilterIndex = 1;
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = max_file_characters;
    dialog.Flags = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;

    BOOL (WINAPI *show_dialog)(LPOPENFILENAME) = &GetOpenFileName;
    if (!open_file) {
        show_dialog = &GetSaveFileName;
    }

    HANDLE file = INVALID_HANDLE_VALUE;
    if (show_dialog(&dialog)) {
        DWORD access_type = GENERIC_READ;
        DWORD creation_type = OPEN_EXISTING;

        if (!open_file) {
            access_type = GENERIC_WRITE;
            creation_type = TRUNCATE_EXISTING;
        }

        if (!FileExists(dialog.lpstrFile)) {
            creation_type = CREATE_NEW;
        }

        file = CreateFile(dialog.lpstrFile,
                          access_type,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          creation_type,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

        *file_name = WideString(dialog.lpstrFile);
        file_name->ExtractFileNameFromPath();
    }

    delete[] buffer;
    return file;
}

void Win32Api::Warning(LPCTSTR message) {
    MessageBox(NULL, message, TEXT("Warning"), MB_OK | MB_ICONWARNING);
}

void Win32Api::Error(const char *message) {
    RaiseFatalError(WideString(message));
}

void Win32Api::Error(const wchar_t *message) {
    RaiseFatalError(WideString(message));
}

void Win32Api::Error(const WideString &message) {
    RaiseFatalError(message);
}

FARPROC Win32Api::GetFunction(LPCTSTR library_name, LPCSTR function_name) {
    const HMODULE library = LoadLibrary(library_name);
    if (!library) {
        return NULL;
    }

    return GetProcAddress(library, function_name);
}

void Win32Api::RaiseFatalError(const WideString &message) {
    MessageBox(NULL, message.ToTString().c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
    raise(SIGABRT);
}

}  // namespace TextEditor
