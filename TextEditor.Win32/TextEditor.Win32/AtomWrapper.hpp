#ifndef ATOMWRAPPER_HPP
#define ATOMWRAPPER_HPP
#pragma once

#include <Windows.h>
#include <StringUtils.hpp>

class AtomWrapper {
private:
    ATOM atom{ 0 };
    UnifiedString name{};

public:
    AtomWrapper(UnifiedString name, const WNDPROC callback);

    UnifiedString GetName(void) const;
};
#endif