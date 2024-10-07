#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stdint.h>
#include <Windows.h>
#include <string_utilities.h>

typedef struct _TEXTBOX {
    HWND hwnd;
    WIDE_STRING *pFont_file, *pFont_name, text;
#ifndef UNICODE
    CHAR multibyte_buffer[3];
#endif
    HFONT font;
    size_t font_width, font_height, wrapped_line_count;
    WORD width, height;
} TEXTBOX;

LRESULT CALLBACK TEXTBOX_callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam);
void TEXTBOX_initialize(const UINT width, const UINT height, const HWND parent, WIDE_STRING *pFont_file, WIDE_STRING *pFont_name);
const TEXTBOX *TEXTBOX_get(void);
void TEXTBOX_destory(void);

#ifndef UNICODE
void TEXTBOX_reset_multibyte_buffer(void);
#endif
#endif