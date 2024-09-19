#include <AtomWrapper.hpp>
#include <WindowsHelper.hpp>

AtomWrapper::AtomWrapper(StringUtilities::UTF8String name, const WNDPROC callback) {
    this->name = name;

    WNDCLASS wndclass{};
    wndclass.lpfnWndProc = callback;

    std::shared_ptr<const TCHAR[]> string{ name.GetWindowsTString() };
    wndclass.lpszClassName = string.get();

    atom = RegisterClass(&wndclass);
    if (!atom) {
        WindowsHelper::ErrorMessage(TEXT("Failed to register class ") + name + TEXT('.'));
    }
}

StringUtilities::UTF8String AtomWrapper::GetName(void) const {
    return name;
}