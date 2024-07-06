#pragma once

#include <Windows.h>
#include <TextBox.hpp>

class MainWindow {
private:
	static HWND hwnd;
	static UINT width, height;
	static TextBox textBox;

	static LRESULT CALLBACK Callback(HWND hwnd,
									 UINT uMsg,
									 WPARAM wParam,
									 LPARAM lParam);

public:
	MainWindow(void);
	void Show(void) const;
};