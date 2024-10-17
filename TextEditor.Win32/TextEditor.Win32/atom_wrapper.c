#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>

void ATOM_WRAPPER_initialize(ATOM_WRAPPER *pAtom_wrapper, WIDE_STRING *pName, const WNDPROC callback) {
    WNDCLASS wndClass = { 0 };
    wndClass.lpfnWndProc = callback;
    wndClass.lpszClassName = WIDE_STRING_get_T_string(pName);

    pAtom_wrapper->atom = RegisterClass(&wndClass);
    if (!pAtom_wrapper->atom) {
        WIDE_STRING message = WIDE_STRING_create_w(L"Failed to register class ");
        WIDE_STRING_append_WIDE_STRING(&message, pName);
        WIDE_STRING_append_wide_char(&message, L'.');

        WINDOWS_HELPER_error_ws(&message);

        WIDE_STRING_destroy(&message);
    }

    MEMORY_HELPER_free((void **)& wndClass.lpszClassName);
}