#ifndef TEXTEDITOR_STRINGS_H
#define TEXTEDITOR_STRINGS_H

#include <Windows.h>

namespace TextEditor {

class Strings {
public:
    enum Locale {
        kEnglishUnitedStates,
        kKoreanSouthKorea
    };

    static void Initialize();

    static Locale locale();

    static const WCHAR *MainWindowTitle();
    static const WCHAR *Untitled();
    static const WCHAR *Encoding();

private:
    static const WCHAR *Select(const WCHAR *english, const WCHAR *korean);

    static Locale current_locale_;
};

}  // namespace TextEditor

#endif
