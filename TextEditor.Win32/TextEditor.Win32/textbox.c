#include <textbox.h>
#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>

static TEXTBOX textbox = { 0 };

void TEXTBOX_CreateCaret(const HWND hwnd) {
    if (!CreateCaret(hwnd, NULL, 2, textbox.font_height)) {
        WINDOWS_HELPER_error(TEXT("Failed to create main window's text box's caret."));
    }
    ShowCaret(hwnd);
}

LRESULT CALLBACK TEXTBOX_callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            if (AddFontResourceEx_saved) {
                textbox.font_width = 16;
                textbox.font_height = 16;

                textbox.pFont_file_t = WIDE_STRING_get_t_string(textbox.pFont_file);
                if (!AddFontResourceEx_saved(textbox.pFont_file_t, FR_PRIVATE, NULL)) {
                    WINDOWS_HELPER_error(TEXT("Failed to add main window's text box's font file."));
                }

                textbox.pFont_name_t = WIDE_STRING_get_t_string(textbox.pFont_name);
                textbox.font = CreateFont(textbox.font_height, textbox.font_width,
                                          0,
                                          0,
                                          0,
                                          FALSE, FALSE, FALSE,
                                          DEFAULT_CHARSET,
                                          OUT_DEFAULT_PRECIS,
                                          CLIP_DEFAULT_PRECIS,
                                          DEFAULT_QUALITY,
                                          FF_DONTCARE,
                                          textbox.pFont_name_t);
                if (!textbox.font) {
                    WINDOWS_HELPER_error(TEXT("Failed to create main window's text box's font."));
                }
            }

            TEXTBOX_CreateCaret(hwnd);
            return 0;
        }

        case WM_SIZE:
            textbox.width = LOWORD(lParam);
            textbox.height = HIWORD(lParam);
            break;

        case WM_IME_COMPOSITION: {
            if (ImmGetContext_saved) {
                const HIMC himc = ImmGetContext_saved(hwnd);

                BOOL comp = lParam & GCS_COMPSTR, result = lParam & GCS_RESULTSTR;
                if (comp || result) {
                    DWORD flag = GCS_COMPSTR;
                    if (result) {
                        flag = GCS_RESULTSTR;
                    }

                    WIDE_STRING_destroy(&textbox.ime);

                    const LONG bytes = ImmGetCompositionString_saved(himc, flag, NULL, 0);
                    LPTSTR temporary = malloc(bytes + sizeof(TCHAR));
                    if (!temporary) {
                        WINDOWS_HELPER_error(TEXT("Failed to get main window's text box's IME string."));
                        break;
                    }

                    ImmGetCompositionString_saved(himc, flag, temporary, bytes);

                    temporary[bytes / sizeof(TCHAR)] = TEXT('\0');
                    textbox.ime = WIDE_STRING_create(temporary);

                    if (result) {
                        WIDE_STRING_append_string_w_at_line(&textbox.text, textbox.ime.pWide_string, textbox.caret_y, textbox.caret_x);
                        TEXTBOX_set_caret_position(textbox.caret_x + wcslen(textbox.ime.pWide_string), textbox.caret_y);
                        WIDE_STRING_destroy(&textbox.ime);
                    }

                    MEMORY_HELPER_free((void **)&temporary);
                }

                ImmReleaseContext_saved(hwnd, himc);
            }

            TEXTBOX_request_redraw();
            return 0;
        }

        case WM_CHAR:
            if (wParam == VK_BACK) {
                TEXTBOX_set_caret_position(textbox.caret_x - 1, textbox.caret_y);

                const size_t y_characters = wcslen(textbox.text.individual_lines.pArray[textbox.caret_y]);
                if (!textbox.caret_x && !y_characters && textbox.caret_y) {
                    size_t new_line_position = 0;
                    for (int i = 0; i < textbox.caret_y; ++i) {
                        new_line_position += wcslen(textbox.text.individual_lines.pArray[i]) + 1;
                    }

                    WIDE_STRING_remove_character_at(&textbox.text, new_line_position);
                    TEXTBOX_set_caret_position(wcslen(textbox.text.individual_lines.pArray[textbox.caret_y - 1]), textbox.caret_y - 1);
                } else if (y_characters) {
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
            InvalidateRect(textbox.hwnd, NULL, TRUE);

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
                WIDE_STRING pLine = { 0 };

                BOOL is_IME_line = textbox.ime.pWide_string && (int)i == textbox.caret_y;
                if (is_IME_line) {
                    pLine = WIDE_STRING_create_empty();
                    int j = 0;
                    for (; j < textbox.caret_x; ++j) {
                        WIDE_STRING_append_wide_char(&pLine, textbox.text.individual_lines.pArray[i][j]);
                    }

                    WIDE_STRING_append_WIDE_STRING(&pLine, &textbox.ime);

                    for (; j < (int)wcslen(textbox.text.individual_lines.pArray[i]); ++j) {
                        WIDE_STRING_append_wide_char(&pLine, textbox.text.individual_lines.pArray[i][j]);
                    }
                } else {
                    pLine = WIDE_STRING_create_w(textbox.text.individual_lines.pArray[i]);
                }

                LPCTSTR pT_line = WIDE_STRING_get_t_string(&pLine);
                if (!DrawText(hdc, pT_line, -1, &paint_struct.rcPaint, DT_EXPANDTABS)) {
                    WINDOWS_HELPER_error(TEXT("Failed to fill main window's text box's text."));
                    continue;
                }
                MEMORY_HELPER_free((void **)&pT_line);

                paint_struct.rcPaint.top += textbox.font_height;

                if ((int)i == textbox.caret_y) {
                    size_t size = sizeof(WCHAR) * (textbox.caret_x + 1);

                    size_t ime_characters = 0, ime_size = 0;
                    if (is_IME_line) {
                        ime_characters = wcslen(textbox.ime.pWide_string);
                        ime_size = sizeof(WCHAR) * ime_characters;
                    }

                    LPWSTR caret_line = malloc(size + ime_size);
                    if (!caret_line) {
                        continue;
                    }

                    memcpy(caret_line, textbox.text.individual_lines.pArray[i], size - sizeof(WCHAR));
                    size_t ending = textbox.caret_x;

                    if (is_IME_line) {
                        memcpy(caret_line + textbox.caret_x, textbox.ime.pWide_string, ime_size);
                        ending += ime_characters;
                    }

                    caret_line[ending] = L'\0';

                    LPTSTR caret_line_t = STRING_UTILITIES_w_to_t(caret_line);

                    RECT text_size = { 0 };
                    DrawText(hdc, caret_line_t, -1, &text_size, DT_CALCRECT | DT_EXPANDTABS);
                    SetCaretPos(text_size.right, textbox.font_height * textbox.caret_y);

                    MEMORY_HELPER_free((void **)&caret_line);
                    MEMORY_HELPER_free((void **)&caret_line_t);
                }
                WIDE_STRING_destroy(&pLine);

                if (is_IME_line) {
                    WIDE_STRING_destroy(&textbox.ime);
                }
            }

            EndPaint(hwnd, &paint_struct);
            return 0;
        }

        case WM_DESTROY:
            if (RemoveFontResourceEx_saved) {
                DeleteObject(textbox.font);
                RemoveFontResourceEx_saved(textbox.pFont_file_t, FR_PRIVATE, 0);
                MEMORY_HELPER_free((void **)&textbox.pFont_file_t);
                MEMORY_HELPER_free((void **)&textbox.pFont_name_t);
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
    WINDOWS_HELPER_get_ImmGetContext();
    WINDOWS_HELPER_get_ImmGetCompositionString();
    WINDOWS_HELPER_get_ImmReleaseContext();

    textbox.pFont_file = pFont_file;
    textbox.pFont_name = pFont_name;
    textbox.text = WIDE_STRING_create_empty();
    textbox.ime.pWide_string = NULL;
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
    ATOM_WRAPPER_initialize(&textbox_class, &textbox_class_name, CS_HREDRAW | CS_VREDRAW, TEXTBOX_callback);

    LPCTSTR pClass_name = WIDE_STRING_get_t_string(&textbox_class_name);
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

void TEXTBOX_set_text(const WIDE_STRING text) {
    WIDE_STRING_destroy(&textbox.text);
    textbox.text = text;

    //For some reason, I have to completely reset the caret.
    DestroyCaret();
    TEXTBOX_CreateCaret(textbox.hwnd);
    TEXTBOX_set_caret_position(0, 0);

    TEXTBOX_request_redraw();
}

void TEXTBOX_request_redraw(void) {
    InvalidateRect(textbox.hwnd, NULL, FALSE);
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
    WIDE_STRING_destroy(&textbox.ime);
}

#ifndef UNICODE
void TEXTBOX_reset_multibyte_buffer(void) {
    memset(textbox.multibyte_buffer, 0, 3);
}
#endif