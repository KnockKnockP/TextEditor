#pragma once

#include <Windows.h>

class WindowsHelper {
public:
	static ATOM Register(const WNDPROC callback,
						 const LPCTSTR name,
						 const HINSTANCE hInstance);
};