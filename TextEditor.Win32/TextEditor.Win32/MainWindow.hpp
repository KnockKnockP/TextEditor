#pragma once

#include <Windows.h>
#include <TextBox.hpp>

class MainWindow {
private:
	static HWND hwnd;
	static UINT width, height;
	static TextBox textBox;

	static LRESULT CALLBACK Callback(const HWND hwnd,
									 const UINT uMsg,
									 const WPARAM wParam,
									 const LPARAM lParam);

public:
	MainWindow(void);
	void Show(void) const;
};