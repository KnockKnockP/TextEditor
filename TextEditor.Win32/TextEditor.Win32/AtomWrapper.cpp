#include <AtomWrapper.hpp>
#include <WindowsHelper.hpp>

AtomWrapper::AtomWrapper(UnifiedString name, const WNDPROC callback) {
    this->name = name;

    WNDCLASS wndclass{};
    wndclass.lpfnWndProc = callback;
    wndclass.lpszClassName = name.GetWindowsString();

    atom = RegisterClass(&wndclass);
    if (!atom) {
        WindowsHelper::ErrorMessage(UT("Failed to register class ") + name + UT('.'));
    }
}

UnifiedString AtomWrapper::GetName(void) const {
    return name;
}