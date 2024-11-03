#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>

void ATOM_WRAPPER_initialize(ATOM_WRAPPER *pAtom_wrapper, WIDE_STRING *pName, const UINT style, const WNDPROC callback) {
    WNDCLASS wndClass = { 0 };
    wndClass.lpszClassName = WIDE_STRING_get_t_string(pName);
    wndClass.style = style;
    wndClass.lpfnWndProc = callback;

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