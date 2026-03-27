#include <leak_checker.h>

#include <strings.h>

#include <memory_helper.h>

namespace TextEditor {

namespace {

const WCHAR kMainWindowTitleEnglish[] = L"Text Editor";
const WCHAR kMainWindowTitleKorean[] = L"臾몄꽌 ?몄쭛湲?";

const WCHAR kUntitledEnglish[] = L"Untitled";
const WCHAR kUntitledKorean[] = L"臾댁젣";

const WCHAR kEncodingEnglish[] = L"Encoding";
const WCHAR kEncodingKorean[] = L"?몄퐫??";

}  // namespace

Strings::Locale Strings::current_locale_ = Strings::kEnglishUnitedStates;

void Strings::Initialize() {
    const int characters = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLANGUAGE, NULL, 0);
    TCHAR *language = new TCHAR[characters];
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLANGUAGE, language, characters);

    if (
#ifdef UNICODE
        wcscmp
#else
        strcmp
#endif
        (language, TEXT("Korean (Korea)")) == 0) {
        current_locale_ = kKoreanSouthKorea;
    }

    delete[] language;
}

Strings::Locale Strings::locale() {
    return current_locale_;
}

const WCHAR *Strings::MainWindowTitle() {
    return Select(kMainWindowTitleEnglish, kMainWindowTitleKorean);
}

const WCHAR *Strings::Untitled() {
    return Select(kUntitledEnglish, kUntitledKorean);
}

const WCHAR *Strings::Encoding() {
    return Select(kEncodingEnglish, kEncodingKorean);
}

const WCHAR *Strings::Select(const WCHAR *english, const WCHAR *korean) {
    if (current_locale_ == kKoreanSouthKorea) {
        return korean;
    }

    return english;
}

}  // namespace TextEditor
