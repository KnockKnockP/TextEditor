#include <leak_checker.h>

#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>

void ATOM_WRAPPER_initialize(ATOM_WRAPPER *pAtom_wrapper, WIDE_STRING *pName, const UINT style, const WNDPROC callback) {
    LPCTSTR name = WIDE_STRING_get_t_string(pName);
    
    WNDCLASS wnd_class = { 0 };
    wnd_class.lpszClassName = name;
    wnd_class.style = style;
    wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    wnd_class.lpfnWndProc = callback;

    pAtom_wrapper->atom = RegisterClass(&wnd_class);
    MEMORY_HELPER_free((void **)&name);
    if (!pAtom_wrapper->atom) {
        WIDE_STRING message = WIDE_STRING_create_w(L"Failed to register class ");
        WIDE_STRING_append_WIDE_STRING(&message, pName);
        WIDE_STRING_append_wide_char(&message, L'.');

        WINDOWS_HELPER_error_ws(&message);

        WIDE_STRING_destroy(&message);
    }
}