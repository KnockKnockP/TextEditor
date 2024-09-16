#ifndef WINDOWSHELPER_HPP
#define WINDOWSHELPER_HPP
#pragma once

#include <Windows.h>
#include <StringUtils.hpp>

class WindowsHelper {
public:
    static void ErrorMessage(UnifiedString contents);
};
#endif