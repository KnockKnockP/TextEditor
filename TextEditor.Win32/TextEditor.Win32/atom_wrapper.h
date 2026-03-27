#ifndef TEXTEDITOR_WINDOW_CLASS_H
#define TEXTEDITOR_WINDOW_CLASS_H

#include <Windows.h>

#include <string_utilities.h>

namespace TextEditor {

class WindowClass {
public:
    WindowClass();

    bool Register(const WideString &name, UINT style, WNDPROC callback);
    ATOM atom() const;

private:
    ATOM atom_;
};

}  // namespace TextEditor

#endif
