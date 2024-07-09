#include <WindowsHelper.hpp>
#include <StringUtils.hpp>

void WindowsHelper::ErrorMessage(const std::string &contents) {
    const TStringContainer tContents{ contents }, tTitle{ "Error" };

    MessageBox(NULL,
               tContents.GetString(),
               tTitle.GetString(),
               (MB_OK | MB_ICONERROR));
}