#include <main_window.h>
#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>
#include <string_utilities.h>

static MAIN_WINDOW main_window = { 0 };

LRESULT CALLBACK MAIN_WINDOW_callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            TEXTBOX_initialize(main_window.width, main_window.height, hwnd, &main_window.font_file, &main_window.font_name);
            return 0;

        case WM_CHAR:
            SendMessage(TEXTBOX_get()->hwnd, uMsg, wParam, lParam);
            break;

        case WM_SIZE:
            main_window.width = LOWORD(lParam);
            main_window.height = HIWORD(lParam);

            SetWindowPos(TEXTBOX_get()->hwnd, NULL,
                         0, 0,
                         main_window.width, main_window.height,
                         SWP_NOREPOSITION | SWP_NOZORDER);
            return 0;

        case WM_CLOSE: {
            WIDE_STRING text = WIDE_STRING_create_w(L"정말로 종료하시겠습니까?"),
                        caption = WIDE_STRING_create_w(L"Are you sure you want to quit?");
            LPCTSTR pText = WIDE_STRING_get_T_string(&text), pCaption = WIDE_STRING_get_T_string(&caption);

            if (MessageBox(hwnd,
                           pText,
                           pCaption,
                           MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
                DestroyWindow(hwnd);
            }

            WIDE_STRING_destroy(&text);
            WIDE_STRING_destroy(&caption);
            MEMORY_HELPER_free((void **)&pText);
            MEMORY_HELPER_free((void **)&pCaption);
            return 0;
        }

        case WM_DESTROY:
            TEXTBOX_destory();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MAIN_WINDOW_initialize(void) {
    main_window.width = 0;
    main_window.height = 0;
    main_window.font_file = WIDE_STRING_create_w(L"unifont-15.1.05.otf");
    main_window.font_name = WIDE_STRING_create_w(L"Unifont");

    ATOM_WRAPPER main_window_class = { 0 };
    WIDE_STRING main_window_class_name = WIDE_STRING_create_w(L"Text Editor"),
                main_window_title = WIDE_STRING_create_w(L"Text Editor / 문서 편집기");
    ATOM_WRAPPER_initialize(&main_window_class, &main_window_class_name, MAIN_WINDOW_callback);
    
    LPCTSTR pMain_window_class_name = WIDE_STRING_get_T_string(&main_window_class_name),
            pMain_window_title = WIDE_STRING_get_T_string(&main_window_title);
    main_window.hwnd = CreateWindow(pMain_window_class_name,
                                    pMain_window_title,
                                    WS_OVERLAPPEDWINDOW,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);
    if (!main_window.hwnd) {
        WINDOWS_HELPER_error(TEXT("Failed to create main window."));
    }

    WIDE_STRING_destroy(&main_window_class_name);
    WIDE_STRING_destroy(&main_window_title);
    MEMORY_HELPER_free((void **)&pMain_window_class_name);
    MEMORY_HELPER_free((void **)&pMain_window_title);
}

void MAIN_WINDOW_show(void) {
    ShowWindow(main_window.hwnd, SW_SHOW);
}

void MAIN_WINDOW_destroy(void) {
    WIDE_STRING_destroy(&main_window.font_file);
    WIDE_STRING_destroy(&main_window.font_name);
}