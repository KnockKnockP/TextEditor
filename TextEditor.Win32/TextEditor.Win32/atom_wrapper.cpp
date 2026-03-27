#include <leak_checker.h>

#include <atom_wrapper.h>

#include <windows_helper.h>

namespace TextEditor {

WindowClass::WindowClass()
    : atom_(0) {
}

bool WindowClass::Register(const WideString &name, UINT style, WNDPROC callback) {
    TStringBuffer class_name = name.ToTString();

    WNDCLASS window_class = { 0 };
    window_class.lpszClassName = class_name.c_str();
    window_class.style = style;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.lpfnWndProc = callback;

    atom_ = RegisterClass(&window_class);
    if (!atom_) {
        WideString message(L"Failed to register class ");
        message.Append(name);
        message.AppendChar(L'.');
        Win32Api::Error(message);
        return false;
    }

    return true;
}

ATOM WindowClass::atom() const {
    return atom_;
}

}  // namespace TextEditor
