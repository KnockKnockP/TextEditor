#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stdint.h>
#include <Windows.h>
#include <windows_helper.h>
#include <string_utilities.h>

typedef struct _TEXTBOX {
    HWND hwnd;

    WIDE_STRING *pFont_file, *pFont_name, text;
    LPCTSTR pFont_file_t, pFont_name_t;

    WIDE_STRING ime;

#ifndef UNICODE
    CHAR multibyte_buffer[3];
#endif

    HFONT font;
    size_t font_width, font_height;

    WORD width, height;

    int caret_x, caret_y;
} TEXTBOX;

void TEXTBOX_initialize(const WORD width, const WORD height, const HWND parent, WIDE_STRING *pFont_file, WIDE_STRING *pFont_name);
const TEXTBOX *TEXTBOX_get(void);
void TEXTBOX_set_text(const WIDE_STRING text);
void TEXTBOX_request_redraw(void);
void TEXTBOX_set_caret_position(int x, int y);
void TEXTBOX_destory(void);

#ifndef UNICODE
void TEXTBOX_reset_multibyte_buffer(void);
#endif
#endif