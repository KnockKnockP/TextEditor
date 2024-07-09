#include <AtomWrapper.hpp>
#include <StringUtils.hpp>
#include <WindowsHelper.hpp>

AtomWrapper::AtomWrapper(const std::string &name, WNDPROC callback) {
    this->name = name;

    WNDCLASS wndclass = {};
    wndclass.lpfnWndProc = callback;

    //const std::wstring wstring{ StringUtils::ToUTF16(name) };
    TStringContainer tName{ StringUtils::ToUTF16(name) };
    wndclass.lpszClassName = tName.GetString();

    atom = RegisterClass(&wndclass);
    if (!atom) {
        WindowsHelper::ErrorMessage("Failed to register class " + name + '.');
    }
}

const std::string &AtomWrapper::GetName(void) const {
    return name;
}