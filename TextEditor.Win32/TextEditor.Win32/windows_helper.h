#ifndef TEXTEDITOR_WINDOWS_HELPER_H
#define TEXTEDITOR_WINDOWS_HELPER_H

#include <Windows.h>
#include <dwmapi.h>
#include <Uxtheme.h>
#include <signal.h>

#include <string_utilities.h>

namespace TextEditor {

class Win32Api {
public:
    enum DocumentInterface {
        kDocumentNone,
        kDocumentMdi,
        kDocumentSdi,
        kDocumentTdi
    };

    enum Style {
        kStyleNone,
        kStyleClassic,
        kStyleAero,
        kStyleMetro
    };

    typedef int (WINAPI *AddFontResourceExFunction)(LPCTSTR name, DWORD flags, PVOID reserved);
    typedef BOOL (WINAPI *RemoveFontResourceExFunction)(LPCTSTR name, DWORD flags, PVOID reserved);
    typedef HIMC (WINAPI *ImmGetContextFunction)(HWND window);
    typedef LONG (WINAPI *ImmGetCompositionStringFunction)(HIMC input_context, DWORD index, LPVOID buffer, DWORD bytes);
    typedef BOOL (WINAPI *ImmReleaseContextFunction)(HWND window, HIMC input_context);
    typedef HTHEME (WINAPI *OpenThemeDataFunction)(HWND window, LPCWSTR class_list);
    typedef HRESULT (WINAPI *DrawThemeBackgroundFunction)(HTHEME theme, HDC hdc, int part_id, int state_id, LPCRECT rect, LPCRECT clip_rect);
    typedef HRESULT (WINAPI *CloseThemeDataFunction)(HTHEME theme);
    typedef HRESULT (WINAPI *DwmEnableBlurBehindWindowFunction)(HWND window, const DWM_BLURBEHIND *blur_behind);
    typedef HRESULT (WINAPI *DwmGetColorizationColorFunction)(DWORD *color, BOOL *opaque_blend);

    static void Initialize();

    static DocumentInterface document_interface();
    static Style style();

    static AddFontResourceExFunction AddFontResourceEx();
    static RemoveFontResourceExFunction RemoveFontResourceEx();
    static ImmGetContextFunction ImmGetContext();
    static ImmGetCompositionStringFunction ImmGetCompositionString();
    static ImmReleaseContextFunction ImmReleaseContext();
    static OpenThemeDataFunction OpenThemeData();
    static DrawThemeBackgroundFunction DrawThemeBackground();
    static CloseThemeDataFunction CloseThemeData();
    static DwmEnableBlurBehindWindowFunction DwmEnableBlurBehindWindow();
    static DwmGetColorizationColorFunction DwmGetColorizationColor();

    static bool FileExists(LPCTSTR path);
    static HANDLE ShowFileDialog(HWND owner, bool open_file, WideString *file_name);

    static void Warning(LPCTSTR message);
    static void Error(const char *message);
    static void Error(const wchar_t *message);
    static void Error(const WideString &message);

private:
    static FARPROC GetFunction(LPCTSTR library_name, LPCSTR function_name);

    static void RaiseFatalError(const WideString &message);

    static DocumentInterface document_interface_;
    static Style style_;
    static AddFontResourceExFunction add_font_resource_ex_;
    static RemoveFontResourceExFunction remove_font_resource_ex_;
    static ImmGetContextFunction imm_get_context_;
    static ImmGetCompositionStringFunction imm_get_composition_string_;
    static ImmReleaseContextFunction imm_release_context_;
    static OpenThemeDataFunction open_theme_data_;
    static DrawThemeBackgroundFunction draw_theme_background_;
    static CloseThemeDataFunction close_theme_data_;
    static DwmEnableBlurBehindWindowFunction dwm_enable_blur_behind_window_;
    static DwmGetColorizationColorFunction dwm_get_colorization_color_;
};

}  // namespace TextEditor

#endif
