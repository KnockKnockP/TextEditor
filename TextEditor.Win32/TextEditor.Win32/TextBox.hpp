#pragma once

#include <string>
#include <Windows.h>

class TextBox {
private:
    static HWND hwnd;
    static std::string fontFile, fontName;
    static HFONT font;

    static LRESULT CALLBACK Callback(const HWND hwnd,
                                     const UINT uMsg,
                                     const WPARAM wParam,
                                     const LPARAM lParam);

public:
    TextBox(void);
    TextBox(const UINT width,
            const UINT height,
            const HWND parent,
            const std::string &fontFile,
            const std::string &fontName);
    HWND GetHwnd(void) const;
};