#include <WindowsHelper.hpp>

void WindowsHelper::ErrorMessage(StringUtilities::UTF8String contents) {
    MessageBox(nullptr,
               contents.GetWindowsTString().get(),
               StringUtilities::UTF8String{TEXT("Error")}.GetWindowsTString().get(),
               MB_OK | MB_ICONERROR);
}