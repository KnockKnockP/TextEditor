#include <textbox.h>
#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>

static TEXTBOX textbox = { 0 };

static void TEXTBOX_request_redraw(void) {
    InvalidateRect(textbox.hwnd, NULL, TRUE);
}

LRESULT CALLBACK TEXTBOX_callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            if (AddFontResourceEx_saved) {
                textbox.pFont_file_t = WIDE_STRING_get_T_string(textbox.pFont_file);
                if (!AddFontResourceEx_saved(textbox.pFont_file_t, FR_PRIVATE, NULL)) {
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
                                          textbox.pFont_file_t);
                if (!textbox.font) {
                    WINDOWS_HELPER_error(TEXT("Failed to create main window's text box's font."));
                }
            }

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
                TEXTBOX_set_caret_position(textbox.caret_x - 1, textbox.caret_y);
                if (wcslen(textbox.text.individual_lines.pArray[textbox.caret_y]) > 0) {
                    WIDE_STRING_remove_character_at_line(&textbox.text, textbox.caret_y, textbox.caret_x);
                }
            } else {
#ifndef UNICODE
                if (IsDBCSLeadByte(wParam)) {
                    if (!textbox.multibyte_buffer[0]) {
                        textbox.multibyte_buffer[0] = wParam;
                    } else {
                        textbox.multibyte_buffer[1] = wParam;

                        WIDE_STRING_append_string_a_at_line(&textbox.text, textbox.multibyte_buffer, textbox.caret_y, textbox.caret_x);
                        TEXTBOX_reset_multibyte_buffer();
                        TEXTBOX_set_caret_position(textbox.caret_x + 1, textbox.caret_y);
                    }
                } else {
#endif
                    WCHAR converted = (WCHAR)wParam;
                    if (converted == L'\r') {
                        converted = L'\n';

                        const int y = textbox.caret_y;
                        TEXTBOX_set_caret_position(0, textbox.caret_y);
                        WIDE_STRING_append_wide_char_at_line(&textbox.text, converted, y, wcslen(textbox.text.individual_lines.pArray[y]));

                        TEXTBOX_set_caret_position(0, y + 1);
                    } else {
                        WIDE_STRING_append_wide_char_at_line(&textbox.text, converted, textbox.caret_y, textbox.caret_x);
                        TEXTBOX_set_caret_position(textbox.caret_x + 1, textbox.caret_y);
                    }
#ifndef UNICODE
                }
#endif
            }

            TEXTBOX_request_redraw();
            return 0;

        case WM_KEYDOWN: {
            if (wParam == VK_LEFT) {
                TEXTBOX_set_caret_position(textbox.caret_x - 1, textbox.caret_y);
                TEXTBOX_request_redraw();
            } else if (wParam == VK_RIGHT) {
                TEXTBOX_set_caret_position(textbox.caret_x + 1, textbox.caret_y);
                TEXTBOX_request_redraw();
            } else if (wParam == VK_UP) {
                TEXTBOX_set_caret_position(textbox.caret_x, textbox.caret_y - 1);
                TEXTBOX_request_redraw();
            } else if (wParam == VK_DOWN) {
                TEXTBOX_set_caret_position(textbox.caret_x, textbox.caret_y + 1);
                TEXTBOX_request_redraw();
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT paint_struct = { 0 };
            HDC hdc = BeginPaint(hwnd, &paint_struct);
            if (!hdc) {
                WINDOWS_HELPER_error(TEXT("Failed to fetch main window's text box's paint data."));
                return 0;
            }

            if (AddFontResourceEx_saved) {
                if (!SelectObject(hdc, textbox.font)) {
                    WINDOWS_HELPER_error(TEXT("Failed to select main window's text box's font."));
                }
            }

            HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
            if (!brush) {
                WINDOWS_HELPER_error(TEXT("Failed to create background brush."));
                return 0;
            }

            if (!FillRect(hdc, &paint_struct.rcPaint, brush)) {
                WINDOWS_HELPER_error(TEXT("Failed to fill main window's text box's rectangle."));
                return 0;
            }

            DeleteObject(brush);

            for (size_t i = 0; i < textbox.text.individual_lines.size; ++i) {
                LPTSTR pLine = STRING_UTILITIES_wide_to_T(textbox.text.individual_lines.pArray[i]);
                if (!pLine) {
                    WINDOWS_HELPER_error(TEXT("Failed to convert main window's text box's text."));
                    continue;
                }

                if (!DrawText(hdc, pLine, -1, &paint_struct.rcPaint, DT_EXPANDTABS)) {
                    WINDOWS_HELPER_error(TEXT("Failed to fill main window's text box's text."));
                    continue;
                }

                paint_struct.rcPaint.top += textbox.font_height;

                if ((int)i == textbox.caret_y) {
                    const size_t size = sizeof(WCHAR) * (textbox.caret_x + 1);
                    LPWSTR caret_line = malloc(size);
                    if (!caret_line) {
                        continue;
                    }

                    memcpy(caret_line, textbox.text.individual_lines.pArray[i], size - sizeof(WCHAR));
                    caret_line[textbox.caret_x] = L'\0';

                    LPTSTR caret_line_t = STRING_UTILITIES_wide_to_T(caret_line);

                    RECT text_size = { 0 };
                    DrawText(hdc, caret_line_t, -1, &text_size, DT_CALCRECT | DT_EXPANDTABS);
                    SetCaretPos(text_size.right, textbox.font_height * textbox.caret_y);

                    MEMORY_HELPER_free((void **)&caret_line);
                    MEMORY_HELPER_free((void **)&caret_line_t);
                }

                MEMORY_HELPER_free((void **)&pLine);
            }

            EndPaint(hwnd, &paint_struct);
            return 0;
        }

        case WM_DESTROY:
            if (RemoveFontResourceEx_saved) {
                DeleteObject(textbox.font);
                RemoveFontResourceEx_saved(textbox.pFont_file_t, FR_PRIVATE, 0);
                MEMORY_HELPER_free((void **)&textbox.pFont_file_t);
            }

            HideCaret(hwnd);
            DestroyCaret();
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void TEXTBOX_initialize(const WORD width, const WORD height, const HWND parent, WIDE_STRING *pFont_file, WIDE_STRING *pFont_name) {
    WINDOWS_HELPER_get_AddFontResourceEx();
    WINDOWS_HELPER_get_RemoveFontResourceEx();

    textbox.pFont_file = pFont_file;
    textbox.pFont_name = pFont_name;
    textbox.text = WIDE_STRING_create_empty();
#ifndef UNICODE
    TEXTBOX_reset_multibyte_buffer();
#endif
    textbox.font_width = 6;
    textbox.font_height = 16;
    textbox.width = width;
    textbox.height = height;
    textbox.caret_x = 0;
    textbox.caret_y = 0;

    ATOM_WRAPPER textbox_class = { 0 };
    WIDE_STRING textbox_class_name = WIDE_STRING_create_w(L"Text Box");
    ATOM_WRAPPER_initialize(&textbox_class, &textbox_class_name, TEXTBOX_callback);

    LPCTSTR pClass_name = WIDE_STRING_get_T_string(&textbox_class_name);
    textbox.hwnd = CreateWindow(pClass_name,
                                NULL,
                                WS_CHILD | WS_VISIBLE,
                                0, 0,
                                textbox.width, textbox.height,
                                parent, NULL, NULL, NULL);
    if (!textbox.hwnd) {
        WINDOWS_HELPER_error(TEXT("Failed to create main window's text box."));
    }

    WIDE_STRING_destroy(&textbox_class_name);
    MEMORY_HELPER_free((void **)&pClass_name);
}

const TEXTBOX *TEXTBOX_get(void) {
    return &textbox;
}

void TEXTBOX_set_caret_position(int x, int y) {
    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        y = 0;
    }

    if (y >= (int)textbox.text.individual_lines.size) {
        y = textbox.text.individual_lines.size - 1;
    }

    const size_t characters = wcslen(textbox.text.individual_lines.pArray[y]);
    if (x > (int)characters) {
        x = characters;
    }

    textbox.caret_x = x;
    textbox.caret_y = y;
}

void TEXTBOX_destory(void) {
    WIDE_STRING_destroy(&textbox.text);
}

#ifndef UNICODE
void TEXTBOX_reset_multibyte_buffer(void) {
    memset(textbox.multibyte_buffer, 0, 3);
}
#endif