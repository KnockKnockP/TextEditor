#include <WindowsHelper.hpp>

ATOM WindowsHelper::Register(const WNDPROC callback, const LPCTSTR name) {
	WNDCLASS wndclass = {};
	wndclass.lpfnWndProc = callback;
	wndclass.lpszClassName = name;

	return RegisterClass(&wndclass);
}