#ifndef ATOMWRAPPER_HPP
#define ATOMWRAPPER_HPP
#pragma once

#include <Windows.h>
#include <StringUtilities.hpp>

class AtomWrapper {
private:
    ATOM atom{ 0 };
    StringUtilities::UTF8String name{};

public:
    AtomWrapper(StringUtilities::UTF8String name, const WNDPROC callback);

    StringUtilities::UTF8String GetName(void) const;
};
#endif