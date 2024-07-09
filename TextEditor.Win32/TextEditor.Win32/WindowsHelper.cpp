#include <WindowsHelper.hpp>
#include <StringUtils.hpp>

void WindowsHelper::ErrorMessage(const std::string &contents) {
    //const std::wstring wstring{ StringUtils::ToUTF16(contents) };
    const TStringContainer tString{ StringUtils::ToUTF16(contents) };
    
    MessageBox(NULL,
               tString.GetString(),
               TEXT("Error"),
               (MB_OK | MB_ICONERROR));
}