#include <leak_checker.h>

#include <main_window.h>
#include <resource.h>
#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>
#include <string_utilities.h>

#ifndef MAIN_WINDOW_OPEN
#define MAIN_WINDOW_OPEN 1
#endif

#ifndef MAIN_WINDOW_SAVE
#define MAIN_WINDOW_SAVE 2
#endif

#ifndef MAIN_WINDOW_QUIT
#define MAIN_WINDOW_QUIT 3
#endif

MAIN_WINDOW main_window = { 0 };

static TEXTBOX *MAIN_WINDOW_create_mdi_child(void) {
    TEXTBOX *pTextbox = TEXTBOX_create(main_window.hwnd, main_window.size, &main_window.font_file, &main_window.font_name);

    MDICREATESTRUCT mdi_create_struct = { 0 };
    mdi_create_struct.szClass = pTextbox_registered_class_name;
    mdi_create_struct.szTitle = pTextbox_registered_class_name;
    mdi_create_struct.x = CW_USEDEFAULT;
    mdi_create_struct.y = CW_USEDEFAULT;

    RECT size = { 0 };
    GetWindowRect(main_window.hwnd, &size);
    mdi_create_struct.cx = size.right / 2;
    mdi_create_struct.cy = size.bottom / 2;

    pTextbox->hwnd = (HWND)SendMessage(main_window.mdi, WM_MDICREATE, 0, (LPARAM)(MDICREATESTRUCT *)&mdi_create_struct);
    return pTextbox;
}

LRESULT CALLBACK MAIN_WINDOW_DefWindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            main_window.hwnd = hwnd;

            const HMENU menu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MAIN_WINDOW_MENU));
            if (menu) {
                SetMenu(hwnd, menu);
            } else {
                WINDOWS_HELPER_error(TEXT("Failed to load main window's menu bar."));
            }

            if (WINDOWS_HELPER_interface_type == WINDOWS_HELPER_MULTIPLE_DOCUMENT_INTERFACE) {
                CLIENTCREATESTRUCT client_create_struct = { 0 };
                main_window.mdi = CreateWindow(TEXT("MDICLIENT"), NULL,
                                               WS_CHILD | WS_VISIBLE,
                                               0, 0, main_window.size.x, main_window.size.y,
                                               hwnd, NULL, NULL, &client_create_struct);

                MAIN_WINDOW_create_mdi_child();
            } else if (WINDOWS_HELPER_interface_type == WINDOWS_HELPER_SINGLE_DOCUMENT_INTERFACE) {
                main_window.pSdi = TEXTBOX_create(hwnd, main_window.size, &main_window.font_file, &main_window.font_name);
            }
            return 0;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_MAIN_WINDOW_MENU_FILE_OPEN: {
                    HANDLE file = WINDOWS_HELPER_file_dialog(hwnd, TRUE);
                    if (file != INVALID_HANDLE_VALUE) {
                        size_t file_size = GetFileSize(file, NULL);
                        BYTE *pBytes = malloc(sizeof(BYTE) * (file_size + sizeof(WCHAR)));
                        if (!pBytes) {
                            goto clean_up_open;
                        }
                        pBytes[0] = '\0';

                        DWORD bytes_read_number = 0;
                        if (!ReadFile(file, pBytes, file_size, &bytes_read_number, NULL)) {
                            WINDOWS_HELPER_error(TEXT("Failed to read file."));
                            goto clean_up_open;
                        }
                        pBytes[bytes_read_number] = '\0';

                        TEXT_FILE text_file = { 0 };
                        text_file.encoding = STRING_UTILITIES_detect_encoding(pBytes, bytes_read_number);
                        if (text_file.encoding == STRING_UTILITIES_ANSI) {
                            text_file.text = WIDE_STRING_create_a((LPCSTR)pBytes);
                        } else if (text_file.encoding == STRING_UTILITIES_WIDE) {
                            pBytes[bytes_read_number + 1] = '\0';
                            text_file.text = WIDE_STRING_create_w((LPCWSTR)(pBytes + 2));
                        } else {
                            LPWSTR pWide = STRING_UTILITIES_UTF8_to_w(
                                (LPCSTR)(pBytes + (text_file.encoding == STRING_UTILITIES_UTF8_WITH_BOM ? 3 : 0))
                            );
                            text_file.text = WIDE_STRING_create_w(pWide);

                            MEMORY_HELPER_free((void **)&pWide);
                        }

                        TEXTBOX *pTextbox = main_window.pSdi;
                        if (WINDOWS_HELPER_interface_type == WINDOWS_HELPER_MULTIPLE_DOCUMENT_INTERFACE) {
                            pTextbox = MAIN_WINDOW_create_mdi_child();
                        }
                        TEXTBOX_set_file(pTextbox, text_file);

                    clean_up_open:
                        MEMORY_HELPER_free((void **)&pBytes);
                        CloseHandle(file);
                    }
                    return 0;
                }

                case ID_MAIN_WINDOW_MENU_FILE_SAVE: {
                    TEXTBOX *pTextbox = main_window.pSdi;
                    if (WINDOWS_HELPER_interface_type == WINDOWS_HELPER_MULTIPLE_DOCUMENT_INTERFACE) {
                        pTextbox = TEXTBOX_find_by_HWND((HWND)SendMessage(main_window.mdi, WM_MDIGETACTIVE, 0, 0));
                    }

                    HANDLE file = WINDOWS_HELPER_file_dialog(hwnd, FALSE);
                    if (file != INVALID_HANDLE_VALUE) {
                        const BYTE UTF_16_LE_BOM[2] = { 0xFF, 0xFE };
                        DWORD bytes_written = 0;

                        if (!WriteFile(file, UTF_16_LE_BOM, 2, &bytes_written, NULL)) {
                            WINDOWS_HELPER_warning(TEXT("Failed to save file."));
                            goto clean_up_save;
                        }

                        const LPCWSTR text = pTextbox->file.text.pWide_string;
                        if (!WriteFile(file, text, wcslen(text) * sizeof(WCHAR), &bytes_written, NULL)) {
                            WINDOWS_HELPER_warning(TEXT("Failed to save file."));
                            goto clean_up_save;
                        }

                    clean_up_save:
                        CloseHandle(file);
                    }
                    return 0;
                }

                case ID_MAIN_WINDOW_MENU_FILE_QUIT:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return 0;
            }
            break;

        case WM_IME_COMPOSITION:
            SendMessage(main_window.pSdi->hwnd, uMsg, wParam, lParam);
            return 0;

        case WM_CHAR:
            SendMessage(main_window.pSdi->hwnd, uMsg, wParam, lParam);
            return 0;

        case WM_KEYDOWN:
            SendMessage(main_window.pSdi->hwnd, uMsg, wParam, lParam);
            return 0;

        case WM_SIZE: {
            main_window.size.x = LOWORD(lParam);
            main_window.size.y = HIWORD(lParam);

            if (!main_window.pSdi) {
                break;
            }

            const HWND textbox = main_window.pSdi->hwnd;
            if (!textbox) {
                break;
            }

            if (!MoveWindow(textbox,
                            0, 0,
                            main_window.size.x, main_window.size.y,
                            FALSE)) {
                            WINDOWS_HELPER_warning(TEXT("Failed to resize."));
            }

            TEXTBOX_request_redraw(main_window.pSdi);
            return 0;
        }

        case WM_CLOSE: {
            WIDE_STRING text = WIDE_STRING_create_w(L"정말로 종료하시겠습니까?"),
                        caption = WIDE_STRING_create_w(L"Are you sure you want to quit?");
            LPCTSTR pText = WIDE_STRING_get_t_string(&text), pCaption = WIDE_STRING_get_t_string(&caption);

            if (MessageBox(main_window.hwnd,
                pText,
                pCaption,
                MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
                DestroyWindow(main_window.hwnd);
            }

            WIDE_STRING_destroy(&text);
            WIDE_STRING_destroy(&caption);
            MEMORY_HELPER_free((void **)&pText);
            MEMORY_HELPER_free((void **)&pCaption);
            return 0;
        }

        case WM_DESTROY:
            WIDE_STRING_destroy(&main_window.font_file);
            WIDE_STRING_destroy(&main_window.font_name);
            PostQuitMessage(0);
            return 0;
    }

    if (WINDOWS_HELPER_interface_type == WINDOWS_HELPER_MULTIPLE_DOCUMENT_INTERFACE) {
        return DefFrameProc(hwnd, main_window.mdi, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MAIN_WINDOW_initialize(void) {
    XY size = { 0 };
    main_window.size = size;

    main_window.pSdi = NULL;
    main_window.font_file = WIDE_STRING_create_w(L"unifont-15.1.05.otf");
    main_window.font_name = WIDE_STRING_create_w(L"Unifont");

    ATOM_WRAPPER main_window_class = { 0 };
    WIDE_STRING main_window_class_name = WIDE_STRING_create_w(L"Text Editor"),
                main_window_title = WIDE_STRING_create_w(L"Text Editor / 문서 편집기");

    ATOM_WRAPPER_initialize(&main_window_class, &main_window_class_name, 0, MAIN_WINDOW_DefWindowProc);
    
    LPCTSTR pMain_window_class_name = WIDE_STRING_get_t_string(&main_window_class_name),
            pMain_window_title = WIDE_STRING_get_t_string(&main_window_title);
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