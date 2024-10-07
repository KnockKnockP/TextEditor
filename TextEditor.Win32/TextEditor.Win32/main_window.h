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

LRESULT CALLBACK MAIN_WINDOW_callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam);

void MAIN_WINDOW_initialize(void);
void MAIN_WINDOW_show(void);
void MAIN_WINDOW_destroy(void);
#endif