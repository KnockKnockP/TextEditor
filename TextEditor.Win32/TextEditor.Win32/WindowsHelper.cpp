#include <WindowsHelper.hpp>

ATOM WindowsHelper::Register(const WNDPROC callback, const std::string name) {
	WNDCLASS wndclass = {};
	wndclass.lpfnWndProc = callback;
	//wndclass.lpszClassName = name;

	return RegisterClass(&wndclass);
}