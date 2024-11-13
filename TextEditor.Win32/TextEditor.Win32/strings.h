#ifndef STRINGS_H
#define STRINGS_H

#include <Windows.h>

#ifndef DEFINE_STRING
#define DEFINE_STRING(name) \
extern const WCHAR STRINGS_##name##_en_us[]; \
extern const WCHAR STRINGS_##name##_ko_kr[]; \
LPCWSTR STRINGS_##name(void)
#endif

typedef enum _LOCALE_DOUBLE {
	EN_US,
	KO_KR
} LOCALE_DOUBLE;

extern LOCALE_DOUBLE STRINGS_locale;

DEFINE_STRING(MAIN_WINDOW_TITLE);

void STRINGS_initialize(void);
#endif