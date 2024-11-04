#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>

void ATOM_WRAPPER_initialize(ATOM_WRAPPER *pAtom_wrapper, WIDE_STRING *pName, const UINT style, const WNDPROC callback) {
    pAtom_wrapper->class_name_t = WIDE_STRING_get_t_string(pName);
    
    WNDCLASS wnd_class = { 0 };
    wnd_class.lpszClassName = pAtom_wrapper->class_name_t;
    wnd_class.style = style;
    wnd_class.lpfnWndProc = callback;

    pAtom_wrapper->atom = RegisterClass(&wnd_class);
    if (!pAtom_wrapper->atom) {
        WIDE_STRING message = WIDE_STRING_create_w(L"Failed to register class ");
        WIDE_STRING_append_WIDE_STRING(&message, pName);
        WIDE_STRING_append_wide_char(&message, L'.');

        WINDOWS_HELPER_error_ws(&message);

        WIDE_STRING_destroy(&message);
    }
}

void ATOM_WRAPPER_destroy(ATOM_WRAPPER *pAtom_wrapper) {
    MEMORY_HELPER_free((void **)&pAtom_wrapper->class_name_t);
}