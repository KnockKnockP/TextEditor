#include <leak_checker.h>

#include <strings.h>

#ifndef IMPLEMENT_STRING
#define IMPLEMENT_STRING(name, en_us, ko_kr) \
const WCHAR STRINGS_##name##_en_us[] = en_us; \
const WCHAR STRINGS_##name##_ko_kr[] = ko_kr; \
LPCWSTR STRINGS_##name(void) { \
    if (locale == KO_KR) { \
        return STRINGS_##name##_ko_kr; \
    } \
    return STRINGS_##name##_en_us; \
}
#endif

LOCALE_DOUBLE locale = EN_US;

IMPLEMENT_STRING(MAIN_WINDOW_TITLE, L"Text Editor", L"문서 편집기")
IMPLEMENT_STRING(UNTITLED, L"Untitled", L"무제");
IMPLEMENT_STRING(ENCODING, L"Encoding", L"인코딩");

void STRINGS_initialize(void) {
    int characters = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLANGUAGE, NULL, 0);
    LPTSTR string = malloc(sizeof(TCHAR) * characters);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLANGUAGE, string, characters);

    if (!
#ifdef UNICODE
    wcscmp
#else
    strcmp
#endif
    (string, TEXT("Korean (Korea)"))) {
        locale = KO_KR;
    }

    free(string);
}