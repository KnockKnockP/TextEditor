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

static MAIN_WINDOW main_window = { 0 };

LRESULT CALLBACK MAIN_WINDOW_callback(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            const HMENU menu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MAIN_WINDOW_MENU));
            if (menu) {
                SetMenu(hwnd, menu);
            } else {
                WINDOWS_HELPER_error(TEXT("Failed to load main window's menu bar."));
            }

            /*
            HMENU menu = CreateMenu(), file_menu = CreatePopupMenu();
            if (menu && file_menu) {
                if (!AppendMenu(file_menu, MF_STRING, MAIN_WINDOW_OPEN, TEXT("Open"))) {
                    WINDOWS_HELPER_error(TEXT("Failed to add entry to main window's menu bar's submenu."));
                }

                if (!AppendMenu(file_menu, MF_STRING, MAIN_WINDOW_SAVE, TEXT("Save"))) {
                    WINDOWS_HELPER_error(TEXT("Failed to add entry to main window's menu bar's submenu."));
                }

                if (!AppendMenu(file_menu, MF_STRING, MAIN_WINDOW_QUIT, TEXT("Quit"))) {
                    WINDOWS_HELPER_error(TEXT("Failed to add entry to main window's menu bar's submenu."));
                }

                if (!AppendMenu(menu, MF_POPUP, (UINT_PTR)file_menu, TEXT("File"))) {
                    WINDOWS_HELPER_error(TEXT("Failed to add entry to main window's menu bar."));
                }

                if (!SetMenu(hwnd, menu)) {
                    WINDOWS_HELPER_error(TEXT("Failed to add main window's menu bar."));
                }
            } else {
                WINDOWS_HELPER_error(TEXT("Failed to create main window's menu bar."));
            }
            */

            TEXTBOX_initialize(main_window.width, main_window.height, hwnd, &main_window.font_file, &main_window.font_name);
            return 0;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_FILE_OPEN: {
                    const DWORD size = sizeof(TCHAR) * 32767;
                    LPTSTR pFile_name = malloc(size);
                    if (!pFile_name) {
                        break;
                    }
                    memset(pFile_name, 0, size);

                    OPENFILENAME open_file_name = { 0 };
                    open_file_name.lStructSize = 76;
                    open_file_name.hwndOwner = hwnd;
                    open_file_name.lpstrFilter = TEXT("Text Files (.txt)\0*.txt\0All Files\0*.*\0\0");
                    open_file_name.nFilterIndex = 1;
                    open_file_name.lpstrFile = pFile_name;
                    open_file_name.nMaxFile = size;
                    open_file_name.Flags = OFN_CREATEPROMPT;

                    if (GetOpenFileName(&open_file_name)) {
                        DWORD create_type = OPEN_EXISTING;
                        if (!WINDOWS_HELPER_file_exists(open_file_name.lpstrFile)) {
                            create_type = CREATE_NEW;
                        }

                        HANDLE file = CreateFile(open_file_name.lpstrFile,
                                      GENERIC_READ,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,
                                      create_type,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);

                        if (file != INVALID_HANDLE_VALUE) {
                            size_t file_size = GetFileSize(file, NULL);
                            BYTE *pBytes = malloc(sizeof(BYTE) * (file_size + sizeof(WCHAR)));
                            if (!pBytes) {
                                goto clean_up;
                            }
                            pBytes[0] = '\0';

                            DWORD bytes_read_number = 0;
                            if (!ReadFile(file, pBytes, file_size, &bytes_read_number, NULL)) {
                                WINDOWS_HELPER_error(TEXT("Failed to read file."));
                                goto clean_up;
                            }
                            pBytes[bytes_read_number] = '\0';

                            const BYTE code_page = STRING_UTILITIES_DETECT_ENCODING(pBytes, bytes_read_number);
                            WIDE_STRING text = { 0 };

                            if (code_page == STRING_UTILITIES_ANSI) {
                                text = WIDE_STRING_create_a((LPCSTR)pBytes);
                            } else if (code_page == STRING_UTILITIES_WIDE) {
                                pBytes[bytes_read_number + 1] = '\0';
                                text = WIDE_STRING_create_w((LPCWSTR)(pBytes + 2));
                            } else {
                                LPWSTR pWide = STRING_UTILITIES_UTF8_to_w((LPCSTR)(pBytes + 3));
                                text = WIDE_STRING_create_w(pWide);

                                MEMORY_HELPER_free((void **)&pWide);
                            }
                            
                            TEXTBOX_set_text(text);

                        clean_up:
                            MEMORY_HELPER_free((void **)&pBytes);
                            CloseHandle(file);
                        } else {
                            WINDOWS_HELPER_error(TEXT("Failed to open file."));
                        }
                    }

                    MEMORY_HELPER_free((void **)&pFile_name);
                    break;
                }

                case ID_FILE_QUIT:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
            }
            return 0;

        case WM_IME_COMPOSITION:
            SendMessage(TEXTBOX_get()->hwnd, uMsg, wParam, lParam);
            return 0;

        case WM_CHAR:
            SendMessage(TEXTBOX_get()->hwnd, uMsg, wParam, lParam);
            return 0;

        case WM_KEYDOWN:
            SendMessage(TEXTBOX_get()->hwnd, uMsg, wParam, lParam);
            return 0;

        case WM_SIZE:
            main_window.width = LOWORD(lParam);
            main_window.height = HIWORD(lParam);

            HWND textbox = TEXTBOX_get()->hwnd;
            if (textbox) {
                if (!MoveWindow(textbox,
                    0, 0,
                    main_window.width, main_window.height,
                    FALSE)) {
                    WINDOWS_HELPER_error(TEXT("Failed to resize."));
                }

                TEXTBOX_request_redraw();
            }
            return 0;

        case WM_CLOSE: {
            WIDE_STRING text = WIDE_STRING_create_w(L"정말로 종료하시겠습니까?"),
                        caption = WIDE_STRING_create_w(L"Are you sure you want to quit?");
            LPCTSTR pText = WIDE_STRING_get_t_string(&text), pCaption = WIDE_STRING_get_t_string(&caption);

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
    ATOM_WRAPPER_initialize(&main_window_class, &main_window_class_name, 0, MAIN_WINDOW_callback);
    
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

void MAIN_WINDOW_destroy(void) {
    WIDE_STRING_destroy(&main_window.font_file);
    WIDE_STRING_destroy(&main_window.font_name);
}