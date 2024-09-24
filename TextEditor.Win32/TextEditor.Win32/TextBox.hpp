#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP
#pragma once

#include <Windows.h>
#include <StringUtilities.hpp>

class TextBox {
private:
    static HWND hwnd;
    static StringUtilities::UTF8String fontFile, fontName, text;
#ifndef UNICODE
    static CHAR multibyteBuffer[3];
#endif
    static HFONT font;
    static size_t fontWidth, fontHeight, wrappedLineCount;
    static WORD width, height;

    static LRESULT CALLBACK Callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam);

public:
    TextBox(void);
    TextBox(const UINT width, const UINT height, const HWND parent, StringUtilities::UTF8String fontFile, const StringUtilities::UTF8String fontName);
    
    HWND GetHwnd(void) const;
    void Keystroke(const WPARAM wParam) const;
};
#endif