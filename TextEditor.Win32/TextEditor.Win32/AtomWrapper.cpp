#include <AtomWrapper.hpp>
#include <StringUtils.hpp>
#include <WindowsHelper.hpp>

AtomWrapper::AtomWrapper(const std::string &name, const WNDPROC callback) {
    this->name = name;

    WNDCLASS wndclass = {};
    wndclass.lpfnWndProc = callback;

    const TStringContainer tName{ TStringContainer{ name } };
    wndclass.lpszClassName = tName.GetString();

    atom = RegisterClass(&wndclass);
    if (!atom) {
        WindowsHelper::ErrorMessage("Failed to register class " + name + '.');
    }
}

const std::string &AtomWrapper::GetName(void) const {
    return name;
}