#ifndef WINDOWSHELPER_HPP
#define WINDOWSHELPER_HPP
#pragma once

#include <Windows.h>
#include <StringUtilities.hpp>

class WindowsHelper {
public:
    static void ErrorMessage(StringUtilities::UTF8String contents);
};
#endif