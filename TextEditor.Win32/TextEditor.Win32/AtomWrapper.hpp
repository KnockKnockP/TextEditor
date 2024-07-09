#pragma once

#include <string>
#include <Windows.h>

class AtomWrapper {
private:
    ATOM atom{ 0 };
    std::string name{};

public:
    AtomWrapper(const std::string &name, const WNDPROC callback);
    const std::string &GetName(void) const;
};