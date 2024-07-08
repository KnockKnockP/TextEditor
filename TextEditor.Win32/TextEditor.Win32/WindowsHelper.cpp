#include <WindowsHelper.hpp>
#include <StringUtils.hpp>

ATOM WindowsHelper::Register(const WNDPROC callback, const std::string &name) {
    WNDCLASSW wndclass = {};
    wndclass.lpfnWndProc = callback;

    const std::wstring wstring{ StringUtils::ToUTF16(name) };
    wndclass.lpszClassName = wstring.c_str();

    return RegisterClassW(&wndclass);
}

void WindowsHelper::ErrorMessage(const std::string &contents) {
    const std::wstring wstring{ StringUtils::ToUTF16(contents) };

    MessageBoxW(NULL,
                wstring.c_str(),
                L"Error",
                (MB_OK | MB_ICONERROR));
}