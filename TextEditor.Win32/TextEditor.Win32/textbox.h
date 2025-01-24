#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stdint.h>
#include <Windows.h>
#include <XY.h>
#include <atom_wrapper.h>
#include <windows_helper.h>
#include <string_utilities.h>

typedef struct _TEXT_FILE {
    WIDE_STRING text;
    int encoding;
} TEXT_FILE;

void TEXT_FILE_destroy(TEXT_FILE *pText_file);

typedef struct _FONT {
    HFONT font;
    XY size;
    WIDE_STRING *pFont_file, *pFont_name;
    LPCTSTR pFont_file_t, pFont_name_t;
} FONT;

typedef struct _TEXTBOX {
    HWND hwnd, status_bar_hwnd;
    XY size, caret, caret_pixels;
    TEXT_FILE file;
    FONT font;
    BOOL focus;

    WIDE_STRING ime;
#ifndef UNICODE
    CHAR multibyte_buffer[3];
#endif
} TEXTBOX;

typedef TEXTBOX* PTEXTBOX;
VECTOR_DECLARE_ALL(PTEXTBOX);

extern LPCTSTR pTextbox_registered_class_name;

TEXTBOX *TEXTBOX_create(const HWND parent,
                        const XY size,
                        WIDE_STRING *pFont_file,
                        WIDE_STRING *pFont_name);
void TEXTBOX_set_file(TEXTBOX *pTextbox, const TEXT_FILE text_file);
TEXTBOX *TEXTBOX_find_by_HWND(const HWND hwnd);
void TEXTBOX_request_redraw(const TEXTBOX *pTextbox);
void TEXTBOX_mdi_redraw(void);
void TEXTBOX_set_caret_position(TEXTBOX *pTextbox, int x, int y);

#ifndef UNICODE
void TEXTBOX_reset_multibyte_buffer(TEXTBOX *pTextbox);
#endif
#endif