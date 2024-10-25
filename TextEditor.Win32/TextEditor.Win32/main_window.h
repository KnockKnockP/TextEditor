#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Windows.h>
#include <textbox.h>

typedef struct _MAIN_WINDOW {
    HWND hwnd;
    WORD width, height;

    TEXTBOX textbox;
    WIDE_STRING font_file, font_name;
} MAIN_WINDOW;

void MAIN_WINDOW_initialize(void);
void MAIN_WINDOW_show(void);
void MAIN_WINDOW_destroy(void);
#endif