#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Windows.h>
#include <ribbon_ported.h>
#include <textbox.h>

typedef struct _MAIN_WINDOW {
    HWND hwnd, mdi, toolbar, tdi;
    LONG_PTR mdi_callback, toolbar_callback;
    XY size, tdi_size;

    TEXTBOX *pSdi;
    WIDE_STRING font_file, font_name;

    IUIFramework *pFramework;
    IUIApplication *pApplication;
    IUICommandHandler *pCommand_handler;
    UINT32 ribbon_height, tdi_strip_height;
} MAIN_WINDOW;

extern MAIN_WINDOW main_window;

void MAIN_WINDOW_initialize(void);
void MAIN_WINDOW_open_file(void);
void MAIN_WINDOW_save_file(void);
void MAIN_WINDOW_exit(void);
#endif