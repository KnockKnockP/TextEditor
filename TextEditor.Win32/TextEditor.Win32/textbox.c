#include <textbox.h>
#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>
#include <windows_versions.h>

static TEXTBOX textbox = { 0 };

LRESULT CALLBACK TEXTBOX_callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            if (!AddFontResourceEx(WIDE_STRING_get_T_string(textbox.pFont_file), FR_PRIVATE, NULL)) {
                WINDOWS_HELPER_error(TEXT("Failed to add main window's text box's font file."));
            }

            textbox.font = CreateFont(textbox.font_height, textbox.font_width,
                                      0,
                                      0,
                                      0,
                                      FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS,
                                      CLIP_DEFAULT_PRECIS,
                                      ANTIALIASED_QUALITY,
                                      FF_DONTCARE,
                                      WIDE_STRING_get_T_string(textbox.pFont_file));
            if (!textbox.font) {
                WINDOWS_HELPER_error(TEXT("Failed to create main window's text box's font."));
            }
#endif

            if (!CreateCaret(hwnd, NULL, 2, textbox.font_height)) {
                WINDOWS_HELPER_error(TEXT("Failed to create main window's text box's caret."));
            }
            ShowCaret(hwnd);
            return 0;
        }

        case WM_SIZE:
            textbox.width = LOWORD(lParam);
            textbox.height = HIWORD(lParam);
            break;

        case WM_CHAR:
            if (wParam == VK_BACK) {
                WIDE_STRING_remove_last_character(&textbox.text);
            } else {
#ifndef UNICODE
                if (IsDBCSLeadByte(wParam)) {
                    if (!textbox.multibyte_buffer[0]) {
                        textbox.multibyte_buffer[0] = wParam;
                    } else {
                        textbox.multibyte_buffer[1] = wParam;

                        WIDE_STRING_append_string_a(&textbox.text, textbox.multibyte_buffer);
                        TEXTBOX_reset_multibyte_buffer();
                    }
                } else {
#endif
                    WCHAR converted = wParam;
                    if (converted == L'\r') {
                        converted = L'\n';
                    }
                    WIDE_STRING_append_wide_char(&textbox.text, converted);
#ifndef UNICODE
                }
#endif
            }

            InvalidateRect(hwnd, NULL, TRUE);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT paint_struct = { 0 };
            HDC hdc = BeginPaint(hwnd, &paint_struct);
            if (!hdc) {
                WINDOWS_HELPER_error(TEXT("Failed to fetch main window's text box's paint data."));
                return 0;
            }

#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            if (!SelectObject(hdc, textbox.font)) {
                WINDOWS_HELPER_error(TEXT("Failed to select main window's text box's font."));
            }
#endif

            HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
            if (!brush) {
                WINDOWS_HELPER_error(TEXT("Failed to create background brush."));
                return 0;
            }

            if (!FillRect(hdc, &paint_struct.rcPaint, brush)) {
                WINDOWS_HELPER_error(TEXT("Failed to fill main window's text box's rectangle."));
                return 0;
            }

            DeleteObject(brush);

            
            size_t i = 0, new_lines = 0, start_of_last_line = 0;
            while (textbox.text.pWide_string[i]) {
                if (textbox.text.pWide_string[i++] == L'\n') {
                    ++new_lines;
                    start_of_last_line = i;
                }
            }

            i = start_of_last_line;
            WIDE_STRING last_line = WIDE_STRING_create_empty();

            while (textbox.text.pWide_string[i]) {
                WIDE_STRING_append_wide_char(&last_line, textbox.text.pWide_string[i++]);
            }

            RECT text_size = { 0 };
            DrawText(hdc, WIDE_STRING_get_T_string(&last_line), -1, &text_size, DT_CALCRECT | DT_EXPANDTABS);
            WIDE_STRING_destroy(&last_line);

            SetCaretPos(text_size.right, new_lines * textbox.font_height);

            if (!DrawText(hdc, WIDE_STRING_get_T_string(&textbox.text), -1, &paint_struct.rcPaint, DT_EXPANDTABS)) {
                WINDOWS_HELPER_error(TEXT("Failed to fill main window's text box's text."));
                return 0;
            }

            EndPaint(hwnd, &paint_struct);
            return 0;
        }

        case WM_DESTROY:
#if WINDOWS_VERSION > _WIN32_WINNT_NT4
            DeleteObject(textbox.font);
            RemoveFontResourceEx(WIDE_STRING_get_T_string(textbox.pFont_file), FR_PRIVATE, 0);
#endif
            HideCaret(hwnd);
            DestroyCaret();
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void TEXTBOX_initialize(const UINT width, const UINT height, const HWND parent, WIDE_STRING *pFont_file, WIDE_STRING *pFont_name) {
    textbox.pFont_file = pFont_file;
    textbox.pFont_name = pFont_name;
    textbox.text = WIDE_STRING_create_empty();
#ifndef UNICODE
    TEXTBOX_reset_multibyte_buffer();
#endif
    textbox.font_width = 6;
    textbox.font_height = 16;
    textbox.wrapped_line_count = 0;
    textbox.width = width;
    textbox.height = height;

    ATOM_WRAPPER textbox_class = { 0 };
    WIDE_STRING textbox_class_name = WIDE_STRING_create_w(L"Text Box");
    ATOM_WRAPPER_initialize(&textbox_class, &textbox_class_name, TEXTBOX_callback);

    textbox.hwnd = CreateWindow(WIDE_STRING_get_T_string(&textbox_class_name),
                                NULL,
                                WS_CHILD | WS_VISIBLE,
                                0, 0,
                                textbox.width, textbox.height,
                                parent, NULL, NULL, NULL);
    if (!textbox.hwnd) {
        WINDOWS_HELPER_error(TEXT("Failed to create main window's text box."));
    }

    WIDE_STRING_destroy(&textbox_class_name);
}

const TEXTBOX *TEXTBOX_get(void) {
    return &textbox;
}

void TEXTBOX_destory(void) {
    WIDE_STRING_destroy(&textbox.text);
}

#ifndef UNICODE
void TEXTBOX_reset_multibyte_buffer(void) {
    memset(textbox.multibyte_buffer, 0, 3);
}
#endif