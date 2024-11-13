#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Windows.h>
#include <ribbon_ported.h>
#include <textbox.h>

typedef struct _MAIN_WINDOW {
    HWND hwnd, mdi;
    LONG_PTR mdi_callback;
    XY size;

    TEXTBOX *pSdi;
    WIDE_STRING font_file, font_name;

    IUIFramework *pFramework;
    UINT32 ribbon_height;
} MAIN_WINDOW;

extern MAIN_WINDOW main_window;
void MAIN_WINDOW_initialize(void);
#endif