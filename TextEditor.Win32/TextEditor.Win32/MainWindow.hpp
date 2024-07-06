#pragma once

#include <Windows.h>
#include <TextBox.hpp>

class MainWindow {
private:
	static MainWindow *singleton;

	HWND hwnd{ NULL };
	UINT width{ 0 }, height{ 0 };
	TextBox textBox;

	static LRESULT CALLBACK Callback(HWND hwnd,
									 UINT uMsg,
									 WPARAM wParam,
									 LPARAM lParam);

public:
	MainWindow(void);
	void Show(void) const;
};