#include <leak_checker.h>

#include <main_window.h>
#include <Uxtheme.h>
#include <vssym32.h>
#include <ribbon.h>
#include <strings.h>
#include <resource.h>
#include <atom_wrapper.h>
#include <memory_helper.h>
#include <windows_helper.h>
#include <string_utilities.h>

MAIN_WINDOW main_window = { 0 };
LONG ribbon_reference_count = 0, ribbon_command_handler_reference_count = 0;
HBITMAP hBitmap = NULL, hBitmap_old = NULL, hBitmap_menu = NULL, hBitmap_old_menu = NULL;
WNDPROC tdi_original = NULL, menu_bar_original = NULL;
HDC tdi_memory_hdc = NULL, menu_memory_hdc = NULL;

HRESULT STDMETHODCALLTYPE IUICommandHandler_QueryInterface(IUICommandHandler *This, REFIID riid, void **ppvObject) {
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IUICommandHandler)) {
        *ppvObject = This;
        This->lpVtbl->AddRef(This);
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE IUICommandHandler_AddRef(IUICommandHandler *This) {
    UNREFERENCED_PARAMETER(This);
    return ++ribbon_command_handler_reference_count;
}

ULONG STDMETHODCALLTYPE IUICommandHandler_Release(IUICommandHandler *This) {
    UNREFERENCED_PARAMETER(This);
    return --ribbon_command_handler_reference_count;
}

HRESULT STDMETHODCALLTYPE IUICommandHandler_Execute(IUICommandHandler *This, UINT32 commandId, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *currentValue, IUISimplePropertySet *commandExecutionProperties) {
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(verb);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(currentValue);
    UNREFERENCED_PARAMETER(commandExecutionProperties);
    
    switch (commandId) {
        case ID_MAIN_WINDOW_MENU_FILE_OPEN_RIBBON:
            MAIN_WINDOW_open_file();
            break;

        case ID_MAIN_WINDOW_MENU_FILE_SAVE_RIBBON:
            MAIN_WINDOW_save_file();
            break;

        case ID_MAIN_WINDOW_MENU_FILE_EXIT_RIBBON:
            MAIN_WINDOW_exit();
            break;

        default:
            return E_NOTIMPL;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE IUICommandHandler_UpdateProperty(IUICommandHandler *This, UINT32 commandId, REFPROPERTYKEY key, const PROPVARIANT *currentValue, PROPVARIANT *newValue) {
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(commandId);
    (void)key;
    UNREFERENCED_PARAMETER(currentValue);
    UNREFERENCED_PARAMETER(newValue);
    return E_NOTIMPL;
}

IUICommandHandlerVtbl ribbon_command_handler_table = {
    IUICommandHandler_QueryInterface,
    IUICommandHandler_AddRef,
    IUICommandHandler_Release,
    IUICommandHandler_Execute,
    IUICommandHandler_UpdateProperty
};

HRESULT STDMETHODCALLTYPE IUIApplication_QueryInterface(IUIApplication *This, REFIID riid, void **ppvObject) {
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IUIApplication)) {
        *ppvObject = This;
        This->lpVtbl->AddRef(This);
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE IUIApplication_AddRef(IUIApplication *This) {
    UNREFERENCED_PARAMETER(This);
    return ++ribbon_reference_count;
}

ULONG STDMETHODCALLTYPE IUIApplication_Release(IUIApplication *This) {
    UNREFERENCED_PARAMETER(This);
    return --ribbon_reference_count;
}

HRESULT STDMETHODCALLTYPE IUIApplication_OnViewChanged(IUIApplication *This, UINT32 viewId, UI_VIEWTYPE typeId, IUnknown *view, UI_VIEWVERB verb, INT32 uReasonCode) {
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(viewId);
    UNREFERENCED_PARAMETER(uReasonCode);

    if (typeId == UI_VIEWTYPE_RIBBON && verb == UI_VIEWVERB_SIZE) {
        IUIRibbon *pRibbon = NULL;

        view->lpVtbl->QueryInterface(view, &IID_IUIRibbon, (void **)&pRibbon);
        if (!pRibbon) {
            return S_FALSE;
        }

        pRibbon->lpVtbl->GetHeight(pRibbon, &main_window.ribbon_height);
        pRibbon->lpVtbl->Release(pRibbon);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IUIApplication_OnCreateUICommand(IUIApplication *This, UINT32 commandId, UI_COMMANDTYPE typeId, IUICommandHandler **commandHandler) {
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(commandId);
    UNREFERENCED_PARAMETER(typeId);

    if (!main_window.pCommand_handler) {
        main_window.pCommand_handler = malloc(sizeof(IUICommandHandler));
        if (!main_window.pCommand_handler) {
            return E_FAIL;
        }

        main_window.pCommand_handler->lpVtbl = &ribbon_command_handler_table;
    }
    *commandHandler = main_window.pCommand_handler;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IUIApplication_OnDestroyUICommand(IUIApplication *This, UINT32 commandId, UI_COMMANDTYPE typeId, IUICommandHandler *commandHandler) {
    UNREFERENCED_PARAMETER(This);
    UNREFERENCED_PARAMETER(commandId);
    UNREFERENCED_PARAMETER(typeId);
    UNREFERENCED_PARAMETER(commandHandler);

    return S_OK;
}

IUIApplicationVtbl ribbon_table = {
    IUIApplication_QueryInterface,
    IUIApplication_AddRef,
    IUIApplication_Release,
    IUIApplication_OnViewChanged,
    IUIApplication_OnCreateUICommand,
    IUIApplication_OnDestroyUICommand
};

static TEXTBOX *MAIN_WINDOW_create_mdi_child(void) {
    TEXTBOX *pTextbox = TEXTBOX_create(main_window.hwnd, main_window.size, 0, &main_window.font_file, &main_window.font_name);

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

LRESULT CALLBACK MAIN_WINDOW_MDI_DefWindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_ERASEBKGND:
            if (WINDOWS_HELPER_style != AERO) {
                break;
            }

            const HDC hdc = (HDC)wParam;
            RECT rect = { 0 };
            GetClientRect(hwnd, &rect);

            const HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            return TRUE;
    }

    return CallWindowProcA((WNDPROC)main_window.mdi_callback, hwnd, uMsg, wParam, lParam);
}

static void send_message_to_sdi(const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    if (main_window.pSdi) {
        SendMessage(main_window.pSdi->hwnd, uMsg, wParam, lParam);
    }
}

static void send_message_to_mdi(const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    if (main_window.mdi) {
        SendMessage(main_window.mdi, uMsg, wParam, lParam);
    }
}

static void MAIN_WINDOW_add_tdi_entry(const LPTSTR name, const int index) {
    TCITEM tab_item = { 0 };
    tab_item.mask = TCIF_TEXT;
    tab_item.pszText = name;
    tab_item.cchTextMax = STRING_UTILITIES_characters(name);
    tab_item.iImage = -1;
    TabCtrl_InsertItem(main_window.tdi, index, &tab_item);
}

static void MAIN_WINDOW_update_tdi_strip_height(void) {
    MAIN_WINDOW_add_tdi_entry(TEXT(""), 0);

    RECT size = { 0 };
    TabCtrl_GetItemRect(main_window.tdi, 0, &size);
    main_window.tdi_strip_height = size.bottom - size.top;

    TabCtrl_DeleteItem(main_window.tdi, 0);
}

static void MAIN_WINDOW_tdi_select(const int index) {
    TabCtrl_SetCurSel(main_window.tdi, index);
    TEXTBOX_tdi_select(index);
    main_window.pSdi = TEXTBOX_tdi_find(index);
}

static void MAIN_WINDOW_create_tdi_child(const LPTSTR name) {
    const int index = TEXTBOX_tdi_size();
    MAIN_WINDOW_add_tdi_entry(name, index);

    TEXTBOX_create(main_window.tdi, main_window.tdi_size, main_window.tdi_strip_height, &main_window.font_file, &main_window.font_name);
    MAIN_WINDOW_tdi_select(index);
}

HWND MAIN_WINDOW_create_toolbar(const HWND parent, const BOOL rebar, const TBBUTTON *pButtons) {
    DWORD style = WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE;
    if (rebar) {
        style = WS_CHILD | TBSTYLE_TRANSPARENT | CCS_NODIVIDER | CCS_NORESIZE;
    }

    const HWND toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                                        style,
                                        0, 0, 0, 0, parent, NULL, NULL, NULL);
    if (toolbar) {
        const TBADDBITMAP bitmap = { HINST_COMMCTRL, IDB_STD_SMALL_COLOR };
        SendMessage(toolbar, TB_ADDBITMAP, 2, (LPARAM)&bitmap);
        SendMessage(toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
        SendMessage(toolbar, TB_ADDBUTTONS, (WPARAM)2, (LPARAM)pButtons);
        SendMessage(toolbar, TB_AUTOSIZE, 0, 0);
    }

    return toolbar;
}

LRESULT CALLBACK MAIN_WINDOW_tdi_DefWindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            HTHEME theme = OpenThemeData_saved(hwnd, L"REBAR");
            if (!theme) {
                break;
            }

            /*
                1. Paint full client.
                2. Paint theme in tab strip.
                3. Paint tabs.
            */

            RECT full_rect = { 0 };
            GetClientRect(hwnd, &full_rect);

            RECT full_normal_rect = { 0 };
            full_normal_rect.right = full_rect.right - full_rect.left;
            full_normal_rect.bottom = full_rect.bottom - full_rect.top;

            if (!tdi_memory_hdc) {
                const HDC hdc_display = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
                tdi_memory_hdc = CreateCompatibleDC(hdc_display);

                hBitmap = CreateCompatibleBitmap(hdc_display, full_normal_rect.right, full_normal_rect.bottom);
                hBitmap_old = SelectObject(tdi_memory_hdc, hBitmap);
                DeleteDC(hdc_display);
            }

            PAINTSTRUCT paint_struct = { 0 };
            HDC hdc = BeginPaint(hwnd, &paint_struct);

            SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)tdi_memory_hdc, PRF_CLIENT);

            HRGN tab_region = CreateRectRgn(0, 0, 0, 0);
            RECT tab_rect = { 0 };
            for (int i = 0; i < TabCtrl_GetItemCount(hwnd); ++i) {
                TabCtrl_GetItemRect(hwnd, i, &tab_rect);
                const HRGN single_tab_region = CreateRectRgn(tab_rect.left, tab_rect.top, tab_rect.right, tab_rect.bottom);
                CombineRgn(tab_region, tab_region, single_tab_region, RGN_OR);
                DeleteObject(single_tab_region);
            }

            GetRgnBox(tab_region, &tab_rect);
            DeleteObject(tab_region);

            RECT toolbar_rect = { 0 };
            if (main_window.toolbar) {
                GetClientRect(main_window.toolbar, &toolbar_rect);
            }

            RECT to_color_rect = { 0 };
            to_color_rect.left = tab_rect.right - tab_rect.left + 2;
            to_color_rect.right = full_normal_rect.right;
            to_color_rect.top = -toolbar_rect.bottom;
            to_color_rect.bottom = tab_rect.bottom - tab_rect.top + 2;

            DrawThemeBackground_saved(theme, hdc, RP_BACKGROUND, 0, &to_color_rect, NULL);
            CloseThemeData_saved(theme);

            BitBlt(hdc, full_rect.left, full_rect.top, tab_rect.right + 2, tab_rect.bottom, tdi_memory_hdc, 0, 0, SRCCOPY);
            EndPaint(hwnd, &paint_struct);
            return 0;
        }

        case WM_DESTROY:
            DeleteObject(hBitmap);
            hBitmap = NULL;
            SelectObject(tdi_memory_hdc, hBitmap_old);
            DeleteDC(tdi_memory_hdc);
            hBitmap_old = NULL;
            tdi_memory_hdc = NULL;
            break;
    }

    return CallWindowProc(tdi_original, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK MAIN_WINDOW_menu_bar_DefWindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        /*
        case WM_PAINT: {
            HTHEME theme = OpenThemeData_saved(hwnd, L"REBAR");
            if (!theme) {
                break;
            }

            RECT rect = { 0 };
            GetClientRect(hwnd, &rect);
            RECT normal_rect = { 0 };
            normal_rect.right = rect.right - rect.left;
            normal_rect.bottom = rect.bottom - rect.top;

            if (!menu_memory_hdc) {
                const HDC hdc_display = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
                menu_memory_hdc = CreateCompatibleDC(hdc_display);

                hBitmap_menu = CreateCompatibleBitmap(hdc_display, normal_rect.right, normal_rect.bottom);
                hBitmap_old_menu = SelectObject(menu_memory_hdc, hBitmap_menu);
                DeleteDC(hdc_display);
            }

            PAINTSTRUCT paint_struct = { 0 };
            HDC hdc = BeginPaint(hwnd, &paint_struct);

            SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)menu_memory_hdc, PRF_CLIENT);
            BitBlt(hdc, rect.left, rect.top, rect.right, rect.bottom, menu_memory_hdc, 0, 0, SRCCOPY);

            const int buttons = SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0);
            const DWORD size = SendMessage(hwnd, TB_GETBUTTONSIZE, 0, 0);
            rect.left += LOWORD(size) * buttons;

            FillRect(hdc, &rect, CreateSolidBrush(RGB(255, 0, 0)));
            //DrawThemeBackground_saved(theme, hdc, RP_BACKGROUND, 0, &rect, NULL);
            CloseThemeData_saved(theme);

            EndPaint(hwnd, &paint_struct);
            return 0;
        }

        case WM_DESTROY:
            DeleteObject(hBitmap_menu);
            hBitmap_menu = NULL;
            SelectObject(menu_memory_hdc, hBitmap_old_menu);
            DeleteDC(menu_memory_hdc);
            hBitmap_old_menu = NULL;
            menu_memory_hdc = NULL;
            break;
            */
    }

    return CallWindowProc(menu_bar_original, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK MAIN_WINDOW_DefWindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            main_window.hwnd = hwnd;

            const HINSTANCE instance = GetModuleHandle(NULL);
            BOOL ribbon_exists = FALSE;
#ifdef USE_RIBBON
            CoCreateInstance(&CLSID_UIRibbonFramework,
                NULL,
                CLSCTX_INPROC_SERVER,
                &IID_IUIFramework,
                (void**)&main_window.pFramework);
            if (!main_window.pFramework) {
                goto after_ribbon;
            }

            main_window.pApplication = malloc(sizeof(IUIApplication));
            if (!main_window.pApplication) {
                goto after_ribbon;
            }
            main_window.pApplication->lpVtbl = &ribbon_table;
            main_window.pApplication->lpVtbl->QueryInterface(main_window.pApplication, &IID_IUIApplication, (void**)&main_window.pApplication);

            main_window.pFramework->lpVtbl->Initialize(main_window.pFramework, hwnd, main_window.pApplication);
            main_window.pFramework->lpVtbl->LoadUI(main_window.pFramework, GetModuleHandle(NULL), L"APPLICATION_RIBBON");
            ribbon_exists = TRUE;
#endif

        after_ribbon:
            if (!ribbon_exists) {
                const TBBUTTON pButtons[2] = {
                    { MAKELONG(STD_FILEOPEN, 0), ID_MAIN_WINDOW_MENU_FILE_OPEN, TBSTATE_ENABLED, BTNS_AUTOSIZE, { 0 }, 0, (INT_PTR)TEXT("Open") },
                    { MAKELONG(STD_FILESAVE, 0), ID_MAIN_WINDOW_MENU_FILE_SAVE, TBSTATE_ENABLED, BTNS_AUTOSIZE, { 0 }, 0, (INT_PTR)TEXT("Save") }
                };

                const HWND rebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
                                                  WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_BANDBORDERS | RBS_VARHEIGHT | CCS_NODIVIDER,
                                                  0, 0, 0, 0, hwnd, NULL, NULL, NULL);
                if (rebar) {
                    main_window.toolbar = rebar;

                    const HWND menu = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                                                     WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT | CCS_NODIVIDER | CCS_NORESIZE,
                                                     0, 0, 0, 0, rebar, NULL, NULL, NULL);
                    if (menu) {
                        const TBBUTTON pFile[1] = { { I_IMAGENONE, ID_MAIN_WINDOW_MENU_FILE, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_DROPDOWN, { 0 }, 0, (INT_PTR)TEXT("File") } };

                        SendMessage(menu, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
                        SendMessage(menu, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&pFile);
                        SendMessage(menu, TB_AUTOSIZE, 0, 0);
                    }

                    const HWND toolbar = MAIN_WINDOW_create_toolbar(rebar, TRUE, pButtons);

                    DWORD size = 0;
                    REBARBANDINFO band = { 0 };
                    band.cbSize = REBARBANDINFO_V3_SIZE;
                    band.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE;
                    band.hwndChild = menu;

                    if (menu) {
                        size = (DWORD)SendMessage(menu, TB_GETBUTTONSIZE, 0, 0);
                        band.cxMinChild = LOWORD(size);
                        band.cyMinChild = HIWORD(size);
                        band.cyChild = HIWORD(size);
                        SendMessage(rebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&band);
                    }

                    if (toolbar) {
                        size = (DWORD)SendMessage(toolbar, TB_GETBUTTONSIZE, 0, 0);
                        band.hwndChild = toolbar;
                        band.fStyle = RBBS_BREAK;
                        band.cxMinChild = LOWORD(size) * 2;
                        band.cyMinChild = HIWORD(size);
                        band.cyChild = HIWORD(size);
                        SendMessage(rebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&band);
                    }
                } else {
                    const HMENU traditional_menu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MAIN_WINDOW_MENU));
                    if (traditional_menu) {
                        SetMenu(hwnd, traditional_menu);
                    }

                    main_window.toolbar = MAIN_WINDOW_create_toolbar(hwnd, FALSE, pButtons);
                }
            }

            if (WINDOWS_HELPER_document_type == MDI) {
                CLIENTCREATESTRUCT client_create_struct = { 0 };
                main_window.mdi = CreateWindow(TEXT("MDICLIENT"), NULL,
                                               WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                                               0, 0, main_window.size.x, main_window.size.y,
                                               hwnd, NULL, NULL, &client_create_struct);

                main_window.mdi_callback = SetWindowLongPtr(main_window.mdi, GWLP_WNDPROC, (LONG)MAIN_WINDOW_MDI_DefWindowProc);
                MAIN_WINDOW_create_mdi_child();
            } else if (WINDOWS_HELPER_document_type == SDI) {
                main_window.pSdi = TEXTBOX_create(hwnd, main_window.size, 0, &main_window.font_file, &main_window.font_name);
            } else {
                RECT parent_size = { 0 }, menu_size = { 0 };
                GetClientRect(hwnd, &parent_size);
                if (ribbon_exists) {
                    //
                } else {
                    GetClientRect(main_window.toolbar, &menu_size);
                }
                parent_size.bottom -= menu_size.bottom;

                main_window.tdi = CreateWindow(WC_TABCONTROL, TEXT(""), WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                               0, menu_size.bottom, parent_size.right, parent_size.bottom,
                                               hwnd, NULL, instance, NULL);

                MAIN_WINDOW_update_tdi_strip_height();
                XY tdi_size = { 0 };
                tdi_size.x = parent_size.right;
                tdi_size.y = parent_size.bottom - main_window.tdi_strip_height;
                main_window.tdi_size = tdi_size;

                WIDE_STRING untitled = WIDE_STRING_create_w(STRINGS_UNTITLED());
                LPCTSTR pUntitled_t = WIDE_STRING_get_t_string(&untitled);
                MAIN_WINDOW_create_tdi_child((const LPTSTR)pUntitled_t);
                MEMORY_HELPER_free((void **)&pUntitled_t);
                WIDE_STRING_destroy(&untitled);

                tdi_original = (WNDPROC)SetWindowLongPtr(main_window.tdi, GWLP_WNDPROC, (LONG)MAIN_WINDOW_tdi_DefWindowProc);
            }
            return 0;
        }

        case WM_SETFOCUS:
            send_message_to_sdi(uMsg, 0, 0);
            send_message_to_mdi(uMsg, 0, 0);
            return 0;

        case WM_KILLFOCUS:
            send_message_to_sdi(uMsg, 0, 0);
            send_message_to_mdi(uMsg, 0, 0);
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_MAIN_WINDOW_MENU_FILE_OPEN:
                    MAIN_WINDOW_open_file();
                    return 0;

                case ID_MAIN_WINDOW_MENU_FILE_SAVE:
                    MAIN_WINDOW_save_file();
                    return 0;

                case ID_MAIN_WINDOW_MENU_FILE_EXIT:
                    MAIN_WINDOW_exit();
                    return 0;
            }
            break;

        case WM_IME_COMPOSITION:
            send_message_to_sdi(uMsg, wParam, lParam);
            return 0;

        case WM_CHAR:
            send_message_to_sdi(uMsg, wParam, lParam);
            return 0;

        case WM_KEYDOWN:
            send_message_to_sdi(uMsg, wParam, lParam);
            return 0;

        case WM_SIZE: {
            main_window.size.x = LOWORD(lParam);
            main_window.size.y = HIWORD(lParam);

            int toolbar_height = 0;
            if (main_window.toolbar) {
                RECT toolbar_rect = { 0 };
                GetWindowRect(main_window.toolbar, &toolbar_rect);
                toolbar_height = toolbar_rect.bottom - toolbar_rect.top;

                MoveWindow(main_window.toolbar, 0, 0, main_window.size.x, toolbar_height, TRUE);
            }

            HWND window = main_window.mdi;
            if (main_window.pSdi) {
                window = main_window.pSdi->hwnd;
            }
            if (main_window.tdi) {
                window = main_window.tdi;
            }
            if (!window) {
                break;
            }

            int top_height = main_window.ribbon_height + toolbar_height, compensated_height = main_window.size.y - top_height;
            if (!MoveWindow(window,
                            0, top_height,
                            main_window.size.x, compensated_height,
                            FALSE)) {
                WINDOWS_HELPER_warning(TEXT("Failed to resize."));
            }

            InvalidateRect(window, NULL, TRUE);
            if (main_window.mdi) {
                TEXTBOX_mdi_redraw();
            }
            if (main_window.tdi) {
                MAIN_WINDOW_update_tdi_strip_height();
                top_height += main_window.tdi_strip_height;
                compensated_height -= main_window.tdi_strip_height;
                SetWindowPos(main_window.pSdi->hwnd, NULL, 0, main_window.tdi_strip_height, main_window.size.x, compensated_height, SWP_DRAWFRAME);
                InvalidateRect(main_window.pSdi->hwnd, NULL, TRUE);
            }
            return 0;
        }

        case WM_SYSCOLORCHANGE:
            if (main_window.toolbar) {
                SendMessage(main_window.toolbar, WM_SYSCOLORCHANGE, 0, 0);
            }
            break;

        case WM_NOTIFY: {
            const LPNMHDR pNmhdr = (LPNMHDR)lParam;

            if (pNmhdr->hwndFrom == main_window.tdi) {
                if (pNmhdr->code == TCN_SELCHANGING) {
                    MAIN_WINDOW_tdi_select(TEXTBOX_tdi_size() - TabCtrl_GetCurSel(main_window.tdi) - 1);
                    return FALSE;
                }
            } else if (pNmhdr->hwndFrom == main_window.toolbar && pNmhdr->code == RBN_ENDDRAG) {
                SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(main_window.size.x, main_window.size.y));
            }

            if (pNmhdr->code == TBN_DROPDOWN) {
                const LPNMTOOLBAR pNmtoolbar = (LPNMTOOLBAR)pNmhdr;
                if (pNmtoolbar->iItem == ID_MAIN_WINDOW_MENU_FILE) {
                    HMENU popup = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MAIN_WINDOW_MENU));
                    popup = GetSubMenu(popup, 0);

                    RECT rect = { 0 };
                    SendMessage(pNmhdr->hwndFrom, TB_GETRECT, pNmtoolbar->iItem, (LPARAM)&rect);
                    MapWindowPoints(pNmhdr->hwndFrom, HWND_DESKTOP, (LPPOINT)&rect, 2);
                    TrackPopupMenu(popup, TPM_LEFTALIGN | TPM_LEFTBUTTON, rect.left, rect.bottom, 0, hwnd, NULL);
                    DestroyMenu(popup);
                    return TBDDRET_DEFAULT;
                }
            }
            break;
        }

        case WM_CLOSE: {
            WIDE_STRING text = WIDE_STRING_create_w(L"정말로 종료하시겠습니까?"),
                        caption = WIDE_STRING_create_w(L"Are you sure you want to exit?");
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

            if (main_window.pFramework) {
                main_window.pFramework->lpVtbl->Release(main_window.pFramework);
            }
            MEMORY_HELPER_free((void **)&main_window.pApplication);
            MEMORY_HELPER_free((void **)&main_window.pCommand_handler);
            PostQuitMessage(0);
            return 0;
    }

    if (WINDOWS_HELPER_document_type == MDI) {
        return DefFrameProc(hwnd, main_window.mdi, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MAIN_WINDOW_initialize(void) {
    main_window.hwnd = NULL;
    main_window.tdi = NULL;

    main_window.mdi_callback = (LONG_PTR)NULL;

    XY size = { 0 };
    main_window.size = size;
    main_window.tdi_size = size;

    main_window.pSdi = NULL;
    main_window.font_file = WIDE_STRING_create_w(L"unifont-15.1.05.otf");
    main_window.font_name = WIDE_STRING_create_w(L"Unifont");

    main_window.pFramework = NULL;
    main_window.pApplication = NULL;
    main_window.pCommand_handler = NULL;
    main_window.ribbon_height = 0;
    main_window.tdi_strip_height = 0;

    ATOM_WRAPPER main_window_class = { 0 };
    WIDE_STRING main_window_class_name = WIDE_STRING_create_w(L"Text Editor"),
                main_window_title = WIDE_STRING_create_w(STRINGS_MAIN_WINDOW_TITLE());

    ATOM_WRAPPER_initialize(&main_window_class, &main_window_class_name, 0, MAIN_WINDOW_DefWindowProc);
    
    LPCTSTR pMain_window_class_name = WIDE_STRING_get_t_string(&main_window_class_name),
            pMain_window_title = WIDE_STRING_get_t_string(&main_window_title);

    main_window.hwnd = CreateWindow(pMain_window_class_name,
                                    pMain_window_title,
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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

void MAIN_WINDOW_open_file(void) {
    WIDE_STRING file_name = { 0 };
    HANDLE file = WINDOWS_HELPER_file_dialog(main_window.hwnd, TRUE, &file_name);
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
        text_file.name = file_name;
        text_file.encoding = STRING_UTILITIES_detect_encoding(pBytes, bytes_read_number);
        if (text_file.encoding == ANSI) {
            text_file.text = WIDE_STRING_create_a((LPCSTR)pBytes);
        } else if (text_file.encoding == WIDE) {
            pBytes[bytes_read_number + 1] = '\0';
            text_file.text = WIDE_STRING_create_w((LPCWSTR)(pBytes + 2));
        } else {
            LPWSTR pWide = STRING_UTILITIES_UTF8_to_w(
                (LPCSTR)(pBytes + (text_file.encoding == UTF8_WITH_BOM ? 3 : 0))
            );
            text_file.text = WIDE_STRING_create_w(pWide);

            MEMORY_HELPER_free((void **)&pWide);
        }

        TEXTBOX *pTextbox = main_window.pSdi;
        if (WINDOWS_HELPER_document_type == MDI) {
            pTextbox = MAIN_WINDOW_create_mdi_child();
        } else if (WINDOWS_HELPER_document_type == TDI) {
            MAIN_WINDOW_create_tdi_child(TEXT("NEW WINDOW"));
            pTextbox = main_window.pSdi;
        }
        TEXTBOX_set_file(pTextbox, text_file);

    clean_up_open:
        MEMORY_HELPER_free((void **)&pBytes);
        CloseHandle(file);
    }
}

void MAIN_WINDOW_save_file(void) {
    TEXTBOX *pTextbox = main_window.pSdi;
    if (WINDOWS_HELPER_document_type == MDI) {
        pTextbox = TEXTBOX_find_by_HWND((HWND)SendMessage(main_window.mdi, WM_MDIGETACTIVE, 0, 0));
    }

    WIDE_STRING file_name = { 0 };
    HANDLE file = WINDOWS_HELPER_file_dialog(main_window.hwnd, FALSE, &file_name);
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
}

void MAIN_WINDOW_exit(void) {
    SendMessage(main_window.hwnd, WM_CLOSE, 0, 0);
}