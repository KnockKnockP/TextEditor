#pragma once

#include <Windows.h>

class TextBox {
private:
	static TextBox *singleton;

	HWND hwnd{ NULL };
	LPCTSTR fontFile{ NULL }, fontName{ NULL };
	HFONT font{ NULL };

	static LRESULT CALLBACK Callback(HWND hwnd,
									 UINT uMsg,
									 WPARAM wParam,
									 LPARAM lParam);

public:
	TextBox(void);
	TextBox(const UINT width,
			const UINT height,
			const HWND parent,
			const LPCTSTR fontFile,
			const LPCTSTR fontName);
	HWND GetHwnd(void) const;
};