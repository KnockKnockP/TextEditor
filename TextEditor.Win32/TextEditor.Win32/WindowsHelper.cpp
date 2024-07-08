#include <WindowsHelper.hpp>

ATOM WindowsHelper::Register(const WNDPROC callback,
							 const LPCTSTR name,
							 const HINSTANCE hInstance) {
	WNDCLASS wndclass = {};
	wndclass.lpfnWndProc = callback;
	wndclass.lpszClassName = name;
	wndclass.hInstance = hInstance;

	return RegisterClass(&wndclass);
}