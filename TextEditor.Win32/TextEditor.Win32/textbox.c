#include <leak_checker.h>

#include <strings.h>
#include <textbox.h>
#include <commctrl.h>
#include <resource.h>
#include <main_window.h>
#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>

VECTOR_IMPLEMENTATION(PTEXTBOX)

static ATOM_WRAPPER registered_class = { 0 };
LPCTSTR pTextbox_registered_class_name = NULL;

static TEXTBOX *pBeing_created = NULL;
static VECTOR_PTEXTBOX textboxes = { 0 };

void TEXT_FILE_destroy(TEXT_FILE *pText_file) {
    WIDE_STRING_destroy(&pText_file->name);
    WIDE_STRING_destroy(&pText_file->text);
}

static void TEXTBOX_CreateCaret(const TEXTBOX *pTextbox) {
    if (!CreateCaret(pTextbox->hwnd, NULL, 2, pTextbox->font.size.y)) {
        WINDOWS_HELPER_warning(TEXT("Failed to create main window's textbox's caret."));
    }
    ShowCaret(pTextbox->hwnd);
}

static LPCTSTR TEXTBOX_get_encoding_string(const int encoding_enum) {
    WIDE_STRING encoding = WIDE_STRING_create_w(STRINGS_ENCODING());
    WIDE_STRING_append_string(&encoding, TEXT(": "));
    WIDE_STRING_append_string(&encoding, STRING_UTILITIES_encoding_enum_to_string(encoding_enum));
    
    LPCTSTR encoding_string = WIDE_STRING_get_t_string(&encoding);
    WIDE_STRING_destroy(&encoding);
    return encoding_string;
}

static void TEXTBOX_set_caret_position_in_pixels(TEXTBOX *pTextbox, const HDC hdc) {
    size_t size = sizeof(WCHAR) * (pTextbox->caret.x + 1), ime_characters = 0, ime_size = 0;
    if (pTextbox->ime.pWide_string) {
        ime_characters = wcslen(pTextbox->ime.pWide_string);
        ime_size = sizeof(WCHAR) * ime_characters;
    }

    LPWSTR caret_line = malloc(size + ime_size);
    if (!caret_line) {
        return;
    }

    memcpy(caret_line, pTextbox->file.text.individual_lines.pArray[pTextbox->caret.y], size - sizeof(WCHAR));
    size_t ending = pTextbox->caret.x;

    if (pTextbox->ime.pWide_string) {
        memcpy(caret_line + pTextbox->caret.x, pTextbox->ime.pWide_string, ime_size);
        ending += ime_characters;
    }

    caret_line[ending] = L'\0';

    LPTSTR caret_line_t = STRING_UTILITIES_w_to_t(caret_line);

    RECT text_size = { 0 };
    DrawText(hdc, caret_line_t, -1, &text_size, DT_CALCRECT | DT_EXPANDTABS);

    pTextbox->caret_pixels.x = text_size.right;
    pTextbox->caret_pixels.y = pTextbox->font.size.y * pTextbox->caret.y;

    MEMORY_HELPER_free((void **)&caret_line);
    MEMORY_HELPER_free((void **)&caret_line_t);
}

static void TEXTBOX_update_title(TEXTBOX *pTextbox) {
    LPCTSTR pFile_name_t = WIDE_STRING_get_t_string(&pTextbox->file.name);
    SetWindowText(pTextbox->hwnd, pFile_name_t);
    MEMORY_HELPER_free((void **)&pFile_name_t);
}

static LRESULT CALLBACK TEXTBOX_DefWindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            TEXTBOX *pTextbox = pBeing_created;
            pTextbox->hwnd = hwnd;

            TEXTBOX_update_title(pTextbox);
            
            if (AddFontResourceEx_saved) {
                pTextbox->font.pFont_file_t = WIDE_STRING_get_t_string(pTextbox->font.pFont_file);

                const BOOL success = AddFontResourceEx_saved(pTextbox->font.pFont_file_t, FR_PRIVATE, 0);
                if (success) {
                    pTextbox->font.size.x = 16;
                    pTextbox->font.size.y = 16;
                } else {
                    WINDOWS_HELPER_warning(TEXT("Failed to add main window's textbox's font file."));
                }

                pTextbox->font.pFont_name_t = WIDE_STRING_get_t_string(pTextbox->font.pFont_name);
                pTextbox->font.font = CreateFont(pTextbox->font.size.y, pTextbox->font.size.x,
                                                 0,
                                                 0,
                                                 0,
                                                 FALSE, FALSE, FALSE,
                                                 DEFAULT_CHARSET,
                                                 OUT_DEFAULT_PRECIS,
                                                 CLIP_DEFAULT_PRECIS,
                                                 DEFAULT_QUALITY,
                                                 FF_DONTCARE,
                                                 pTextbox->font.pFont_name_t);
                if (!pTextbox->font.font) {
                    WINDOWS_HELPER_warning(TEXT("Failed to create main window's textbox's font."));
                }
            }

            LPCTSTR encoding_string = TEXTBOX_get_encoding_string(pTextbox->file.encoding);
            pTextbox->status_bar_hwnd = CreateStatusWindow(WS_CHILD | WS_VISIBLE, encoding_string, hwnd, IDR_TEXTBOX_STATUS_BAR);
            MEMORY_HELPER_free((void **)&encoding_string);
            if (!pTextbox->status_bar_hwnd) {
                WINDOWS_HELPER_warning(TEXT("Failed to create main window's textbox's status bar."));
            }
            return 0;
        }

        case WM_LBUTTONDOWN:
            SetFocus(hwnd);
            return 0;

        case WM_SETFOCUS: {
            TEXTBOX *pTextbox = TEXTBOX_find_by_HWND(hwnd);
            if (!pTextbox) {
                break;
            }

            pTextbox->focus = TRUE;
            TEXTBOX_CreateCaret(pTextbox);
            return 0;
        }

        case WM_SIZE: {
            TEXTBOX *pTextbox = TEXTBOX_find_by_HWND(hwnd);
            if (!pTextbox) {
                break;
            }

            pTextbox->size.x = LOWORD(lParam);
            pTextbox->size.y = HIWORD(lParam);
            SendMessage(pTextbox->status_bar_hwnd, WM_SIZE, wParam, lParam);
            break;
        }

        case WM_IME_COMPOSITION: {
            TEXTBOX *pTextbox = TEXTBOX_find_by_HWND(hwnd);
            if (!pTextbox) {
                break;
            }

            if (ImmGetContext_saved) {
                const HIMC himc = ImmGetContext_saved(hwnd);

                BOOL comp = lParam & GCS_COMPSTR, result = lParam & GCS_RESULTSTR;
                if (comp || result) {
                    DWORD flag = GCS_COMPSTR;
                    if (result) {
                        flag = GCS_RESULTSTR;
                    }

                    WIDE_STRING_destroy(&pTextbox->ime);

                    const LONG bytes = ImmGetCompositionString_saved(himc, flag, NULL, 0);
                    LPTSTR temporary = malloc(bytes + sizeof(TCHAR));
                    if (!temporary) {
                        WINDOWS_HELPER_warning(TEXT("Failed to get main window's textbox's IME string."));
                        break;
                    }

                    ImmGetCompositionString_saved(himc, flag, temporary, bytes);

                    temporary[bytes / sizeof(TCHAR)] = TEXT('\0');
                    pTextbox->ime = WIDE_STRING_create(temporary);

                    if (result) {
                        WIDE_STRING_append_string_w_at_line(&pTextbox->file.text,
                                                            pTextbox->ime.pWide_string,
                                                            pTextbox->caret.y,
                                                            pTextbox->caret.x);
                        TEXTBOX_set_caret_position(pTextbox,
                                                   pTextbox->caret.x + wcslen(pTextbox->ime.pWide_string),
                                                   pTextbox->caret.y);
                        WIDE_STRING_destroy(&pTextbox->ime);
                    }

                    MEMORY_HELPER_free((void **)&temporary);
                }

                ImmReleaseContext_saved(hwnd, himc);
            }

            TEXTBOX_request_redraw(pTextbox);
            return 0;
        }

        case WM_CHAR: {
            TEXTBOX *pTextbox = TEXTBOX_find_by_HWND(hwnd);
            if (!pTextbox) {
                break;
            }

            if (wParam == VK_BACK) {
                TEXTBOX_set_caret_position(pTextbox, pTextbox->caret.x - 1, pTextbox->caret.y);

                const size_t y_characters = wcslen(pTextbox->file.text.individual_lines.pArray[pTextbox->caret.y]);
                if (!pTextbox->caret.x && !y_characters && pTextbox->caret.y) {
                    size_t new_line_position = 0;
                    for (int i = 0; i < pTextbox->caret.y; ++i) {
                        new_line_position += wcslen(pTextbox->file.text.individual_lines.pArray[i]) + 1;
                    }

                    WIDE_STRING_remove_character_at(&pTextbox->file.text, new_line_position);
                    TEXTBOX_set_caret_position(pTextbox,
                                               wcslen(pTextbox->file.text.individual_lines.pArray[pTextbox->caret.y - 1]),
                                               pTextbox->caret.y - 1);
                } else if (y_characters) {
                    WIDE_STRING_remove_character_at_line(&pTextbox->file.text, pTextbox->caret.y, pTextbox->caret.x);
                }
            } else {
#ifndef UNICODE
                if (IsDBCSLeadByte(wParam)) {
                    if (!pTextbox->multibyte_buffer[0]) {
                        pTextbox->multibyte_buffer[0] = wParam;
                    } else {
                        pTextbox->multibyte_buffer[1] = wParam;

                        WIDE_STRING_append_string_a_at_line(&pTextbox->file.text,
                                                            pTextbox->multibyte_buffer,
                                                            pTextbox->caret.y,
                                                            pTextbox->caret.x);
                        TEXTBOX_reset_multibyte_buffer(pTextbox);
                        TEXTBOX_set_caret_position(pTextbox, pTextbox->caret.x + 1, pTextbox->caret.y);
                    }
                } else {
#endif
                    WCHAR converted = (WCHAR)wParam;
                    if (converted == L'\r') {
                        converted = L'\n';

                        const int y = pTextbox->caret.y;
                        TEXTBOX_set_caret_position(pTextbox, 0, pTextbox->caret.y);
                        WIDE_STRING_append_wide_char_at_line(&pTextbox->file.text,
                                                             converted,
                                                             y,
                                                             wcslen(pTextbox->file.text.individual_lines.pArray[y]));

                        TEXTBOX_set_caret_position(pTextbox, 0, y + 1);
                    } else {
                        WIDE_STRING_append_wide_char_at_line(&pTextbox->file.text,
                                                             converted,
                                                             pTextbox->caret.y,
                                                             pTextbox->caret.x);
                        TEXTBOX_set_caret_position(pTextbox, pTextbox->caret.x + 1, pTextbox->caret.y);
                    }
#ifndef UNICODE
                }
#endif
            }

            TEXTBOX_request_redraw(pTextbox);
            return 0;
        }

        case WM_KEYDOWN: {
            TEXTBOX *pTextbox = TEXTBOX_find_by_HWND(hwnd);
            if (!pTextbox) {
                break;
            }

            if (wParam == VK_LEFT) {
                TEXTBOX_set_caret_position(pTextbox, pTextbox->caret.x - 1, pTextbox->caret.y);
                TEXTBOX_request_redraw(pTextbox);
            } else if (wParam == VK_RIGHT) {
                TEXTBOX_set_caret_position(pTextbox, pTextbox->caret.x + 1, pTextbox->caret.y);
                TEXTBOX_request_redraw(pTextbox);
            } else if (wParam == VK_UP) {
                TEXTBOX_set_caret_position(pTextbox, pTextbox->caret.x, pTextbox->caret.y - 1);
                TEXTBOX_request_redraw(pTextbox);
            } else if (wParam == VK_DOWN) {
                TEXTBOX_set_caret_position(pTextbox, pTextbox->caret.x, pTextbox->caret.y + 1);
                TEXTBOX_request_redraw(pTextbox);
            }
            return 0;
        }

        case WM_PAINT: {
            TEXTBOX *pTextbox = TEXTBOX_find_by_HWND(hwnd);
            if (!pTextbox) {
                break;
            }

            InvalidateRect(hwnd, NULL, TRUE);

            PAINTSTRUCT paint_struct = { 0 };
            HDC hdc = BeginPaint(hwnd, &paint_struct);
            if (!hdc) {
                WINDOWS_HELPER_error(TEXT("Failed to fetch main window's textbox's paint data."));
                break;
            }

            if (AddFontResourceEx_saved) {
                if (!SelectObject(hdc, pTextbox->font.font)) {
                    WINDOWS_HELPER_warning(TEXT("Failed to select main window's textbox's font."));
                }
            }

            const COLORREF black = RGB(0, 0, 0), white = RGB(255, 255, 255);
            COLORREF background_color = white, text_color = black, text_background_color = background_color;
            if (WINDOWS_HELPER_document_type == SDI) {
                if (WINDOWS_HELPER_style == AERO) {
                    text_color = white;
                    text_background_color = TRANSPARENT;
                    background_color = black;
                } else if (WINDOWS_HELPER_style == METRO && DwmGetColorizationColor_saved) {
                    DWORD color = 0;
                    BOOL opaque = FALSE;

                    if (DwmGetColorizationColor_saved(&color, &opaque) == S_OK) {
                        text_color = RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
                    }
                }
            }

            HBRUSH brush = CreateSolidBrush(background_color);
            if (!brush) {
                WINDOWS_HELPER_error(TEXT("Failed to create background brush."));
                break;
            }

            if (!FillRect(hdc, &paint_struct.rcPaint, brush)) {
                WINDOWS_HELPER_error(TEXT("Failed to fill main window's textbox's rectangle."));
                break;
            }
            DeleteObject(brush);

            TEXTBOX_set_caret_position_in_pixels(pTextbox, hdc);

            SetTextColor(hdc, text_color);
            SetBkColor(hdc, text_background_color);
            for (int i = 0; i < pTextbox->file.text.individual_lines.size; ++i) {
                WIDE_STRING pLine = { 0 };

                BOOL is_IME_line = pTextbox->ime.pWide_string && (int)i == pTextbox->caret.y;
                if (is_IME_line) {
                    pLine = WIDE_STRING_create_empty();
                    int j = 0;
                    for (; j < pTextbox->caret.x; ++j) {
                        WIDE_STRING_append_wide_char(&pLine, pTextbox->file.text.individual_lines.pArray[i][j]);
                    }

                    WIDE_STRING_append_WIDE_STRING(&pLine, &pTextbox->ime);

                    for (; j < (int)wcslen(pTextbox->file.text.individual_lines.pArray[i]); ++j) {
                        WIDE_STRING_append_wide_char(&pLine, pTextbox->file.text.individual_lines.pArray[i][j]);
                    }
                } else {
                    pLine = WIDE_STRING_create_w(pTextbox->file.text.individual_lines.pArray[i]);
                }

                LPCTSTR pT_line = WIDE_STRING_get_t_string(&pLine);
                if (!DrawText(hdc, pT_line, -1, &paint_struct.rcPaint, DT_EXPANDTABS)) {
                    WINDOWS_HELPER_error(TEXT("Failed to fill main window's textbox's text."));
                    continue;
                }
                MEMORY_HELPER_free((void **)&pT_line);

                paint_struct.rcPaint.top += pTextbox->font.size.y;
                WIDE_STRING_destroy(&pLine);

                if (is_IME_line) {
                    WIDE_STRING_destroy(&pTextbox->ime);
                }
            }

            if (pTextbox->focus) {
                SetCaretPos(pTextbox->caret_pixels.x, pTextbox->caret_pixels.y);
            }

            EndPaint(hwnd, &paint_struct);
            break;
        }

        case WM_KILLFOCUS: {
            TEXTBOX* pTextbox = TEXTBOX_find_by_HWND(hwnd);
            if (!pTextbox) {
                break;
            }

            pTextbox->focus = FALSE;

            HideCaret(hwnd);
            DestroyCaret();
            return 0;
        }

        case WM_DESTROY: {
            TEXTBOX *pTextbox = TEXTBOX_find_by_HWND(hwnd);
            if (!pTextbox) {
                break;
            }

            TEXT_FILE_destroy(&pTextbox->file);
            WIDE_STRING_destroy(&pTextbox->ime);

            if (RemoveFontResourceEx_saved) {
                DeleteObject(pTextbox->font.font);
                RemoveFontResourceEx_saved(pTextbox->font.pFont_name_t, FR_PRIVATE, 0);
                MEMORY_HELPER_free((void **)&pTextbox->font.pFont_file_t);
                MEMORY_HELPER_free((void **)&pTextbox->font.pFont_name_t);
            }

            HideCaret(hwnd);
            DestroyCaret();

            if (textboxes.pArray) {
                MEMORY_HELPER_free((void **)&pTextbox);

                if (!textboxes.size) {
                    MEMORY_HELPER_free((void **)&pTextbox_registered_class_name);
                    VECTOR_DESTROY_PTEXTBOX(&textboxes);
                }
            }
            return 0;
        }
    }

    if (WINDOWS_HELPER_document_type == MDI) {
        return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

TEXTBOX *TEXTBOX_create(const HWND parent,                    
                        const XY size,
                        const int vertical_offset,
                        WIDE_STRING *pFont_file,
                        WIDE_STRING *pFont_name) {
    TEXTBOX *pTextbox = malloc(sizeof(TEXTBOX));
    if (!pTextbox) {
        WINDOWS_HELPER_error(TEXT("Failed to allocate a new textbox."));
        return NULL;
    }

    pTextbox->status_bar_hwnd = NULL;
    pTextbox->size = size;

    const XY caret_position = { 0 };
    pTextbox->caret = caret_position;
    pTextbox->caret_pixels = caret_position;

    TEXT_FILE text_file = { 0 };
    text_file.name = WIDE_STRING_create_w(STRINGS_UNTITLED());
    text_file.text = WIDE_STRING_create_empty();
    text_file.encoding = WIDE;
    pTextbox->file = text_file;

    FONT font = { 0 };
    font.size.x = 6;
    font.size.y = 16;
    font.pFont_file = pFont_file;
    font.pFont_name = pFont_name;
    pTextbox->font = font;

    WIDE_STRING ime = { 0 };
    pTextbox->ime = ime;
#ifndef UNICODE
    TEXTBOX_reset_multibyte_buffer(pTextbox);
#endif

    if (!registered_class.atom) {
        WIDE_STRING textbox_class_name = WIDE_STRING_create_w(L"Text Box");
        pTextbox_registered_class_name = WIDE_STRING_get_t_string(&textbox_class_name);
        ATOM_WRAPPER_initialize(&registered_class, &textbox_class_name, CS_HREDRAW | CS_VREDRAW, TEXTBOX_DefWindowProc);
        WIDE_STRING_destroy(&textbox_class_name);
    }

    pBeing_created = pTextbox;

    if (WINDOWS_HELPER_document_type != MDI) {
        pTextbox->hwnd = CreateWindow(pTextbox_registered_class_name,
                                      NULL,
                                      WS_CHILD | WS_VISIBLE,
                                      0, vertical_offset,
                                      pTextbox->size.x, pTextbox->size.y,
                                      parent, NULL, NULL, NULL);
        if (!pTextbox->hwnd) {
            WINDOWS_HELPER_error(TEXT("Failed to create main window's text box."));
        }
    }

    if (textboxes.pArray == NULL) {
        textboxes = VECTOR_CREATE_PTEXTBOX();
    }
    VECTOR_PUSH_PTEXTBOX(&textboxes, pTextbox);
    return pTextbox;
}

TEXTBOX *TEXTBOX_find_by_HWND(const HWND hwnd) {
    for (int i = 0; i < textboxes.size; ++i) {
        if (textboxes.pArray[i] && textboxes.pArray[i]->hwnd == hwnd) {
            return textboxes.pArray[i];
        }
    }

    return NULL;
}

void TEXTBOX_set_file(TEXTBOX *pTextbox, const TEXT_FILE text_file) {
    TEXT_FILE_destroy(&pTextbox->file);
    pTextbox->file = text_file;

    const WPARAM wParam = MAKEWPARAM(0, SBT_OWNERDRAW);
    LPCTSTR encoding_string = TEXTBOX_get_encoding_string(pTextbox->file.encoding);
    SendMessage(pTextbox->status_bar_hwnd, SB_SETTEXT, wParam, (LPARAM)encoding_string);
    MEMORY_HELPER_free((void **)&encoding_string);

    TEXTBOX_update_title(pTextbox);
    TEXTBOX_request_redraw(pTextbox);
}

void TEXTBOX_request_redraw(const TEXTBOX *pTextbox) {
    InvalidateRect(pTextbox->hwnd, NULL, FALSE);
}

void TEXTBOX_mdi_redraw(void) {
    for (int i = 0; i < textboxes.size; ++i) {
        const TEXTBOX *pTextbox = textboxes.pArray[i];
        if (pTextbox) {
            SetWindowPos(pTextbox->hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);
        }
    }
}

void TEXTBOX_set_caret_position(TEXTBOX *pTextbox, int x, int y) {
    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        y = 0;
    }

    if (y >= (int)pTextbox->file.text.individual_lines.size) {
        y = pTextbox->file.text.individual_lines.size - 1;
    }

    const size_t characters = wcslen(pTextbox->file.text.individual_lines.pArray[y]);
    if (x > (int)characters) {
        x = characters;
    }

    pTextbox->caret.x = x;
    pTextbox->caret.y = y;
}

TEXTBOX *TEXTBOX_tdi_find(const int index) {
    return textboxes.pArray[index];
}

void TEXTBOX_tdi_select(const int index) {
    for (int i = 0; i < textboxes.size; ++i) {
        ShowWindow(textboxes.pArray[i]->hwnd, SW_HIDE);
    }

    const HWND selected = textboxes.pArray[index]->hwnd;
    ShowWindow(selected, SW_SHOW);
}

int TEXTBOX_tdi_size(void) {
    int size = 0;
    if (textboxes.pArray) {
        size = textboxes.size;
    }
    return size;
}

#ifndef UNICODE
void TEXTBOX_reset_multibyte_buffer(TEXTBOX *pTextbox) {
    memset(pTextbox->multibyte_buffer, 0, sizeof(pTextbox->multibyte_buffer));
}
#endif