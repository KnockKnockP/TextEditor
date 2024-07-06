/*
#include <Window.hpp>

Window *Window::singleton = nullptr;

Window *Window::GetSingleton(void) {
	return singleton;
}

ATOM Window::Register(LPCTSTR name) const {
	WNDCLASS wndclass{};
	wndclass.lpfnWndProc = Callback;
	wndclass.lpszClassName = name;

	return RegisterClass(&wndclass);
}

bool Window::Create(void) {
	return false;
}

void Window::Show(void) const {
	ShowWindow(hwnd, SW_SHOW);
}

HWND Window::GetHwnd(void) const {
	return hwnd;
}
*/