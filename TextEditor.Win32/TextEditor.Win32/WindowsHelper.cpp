#include <WindowsHelper.hpp>

void WindowsHelper::ErrorMessage(UnifiedString contents) {
    MessageBox(nullptr, contents.GetWindowsString(), UnifiedString{ UT("Error") }.GetWindowsString(), MB_OK | MB_ICONERROR);
}