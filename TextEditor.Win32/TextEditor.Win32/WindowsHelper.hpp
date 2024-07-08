#pragma once

#include <string>
#include <Windows.h>

class WindowsHelper {
public:
    static ATOM Register(const WNDPROC callback, const std::string &name);
    static void ErrorMessage(const std::string &contents);
};