#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP
#pragma once

#include <string>
#include <Windows.h>
#include <StringUtils.hpp>

class TextBox {
private:
    static HWND hwnd;
    static UnifiedString fontFile, fontName, text;
#ifndef UNICODE
    static CHAR multibyteBuffer[3];
#endif
    static HFONT font;

    static LRESULT CALLBACK Callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam);

public:
    TextBox(void);
    TextBox(const UINT width, const UINT height, const HWND parent, UnifiedString fontFile, const UnifiedString fontName);
    
    HWND GetHwnd(void) const;
    void Keystroke(const WPARAM wParam) const;
};
#endif