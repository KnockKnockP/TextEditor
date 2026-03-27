#include <leak_checker.h>

#include <main_window.h>

#include <CommCtrl.h>
#include <Uxtheme.h>
#include <memory_helper.h>
#include <resource.h>
#include <ribbon.h>
#include <strings.h>
#include <vssym32.h>

namespace TextEditor {

WindowClass MainWindow::window_class_;
bool MainWindow::window_class_registered_ = false;

MainWindow::MainWindow()
    : hwnd_(NULL),
      mdi_client_(NULL),
      toolbar_(NULL),
      tab_control_(NULL),
      mdi_original_proc_(NULL),
      tab_original_proc_(NULL),
      active_text_box_(NULL),
      font_file_(L"unifont-15.1.05.otf"),
      font_name_(L"Unifont"),
      tdi_strip_height_(0),
      tab_bitmap_(NULL),
      tab_bitmap_old_(NULL),
      tab_memory_hdc_(NULL) {
}

MainWindow::~MainWindow() {
    if (tab_memory_hdc_) {
        SelectObject(tab_memory_hdc_, tab_bitmap_old_);
        DeleteObject(tab_bitmap_);
        DeleteDC(tab_memory_hdc_);
    }
}

bool MainWindow::Initialize() {
    if (!window_class_registered_) {
        window_class_registered_ = window_class_.Register(WideString(Strings::MainWindowTitle()), 0, WindowProcedure);
    }

    hwnd_ = CreateWindow(MAKEINTATOM(window_class_.atom()),
                         Strings::MainWindowTitle(),
                         WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         NULL,
                         this);
    return hwnd_ != NULL;
}

void MainWindow::OpenFile() {
    WideString file_name;
    HANDLE file = Win32Api::ShowFileDialog(hwnd_, true, &file_name);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    const DWORD file_size = GetFileSize(file, NULL);
    BYTE *bytes = reinterpret_cast<BYTE *>(TextEditor::memory::AllocateBytes(file_size + sizeof(WCHAR)));
    DWORD bytes_read = 0;

    TextFile text_file;
    text_file.name = file_name;

    if (!ReadFile(file, bytes, file_size, &bytes_read, NULL)) {
        TextEditor::memory::Free(bytes);
        CloseHandle(file);
        Win32Api::Error(TEXT("Failed to read file."));
        return;
    }

    bytes[bytes_read] = '\0';
    text_file.encoding = DetectEncoding(bytes, bytes_read);
    if (text_file.encoding == kTextEncodingAnsi) {
        text_file.text = WideString(reinterpret_cast<const char *>(bytes));
    } else if (text_file.encoding == kTextEncodingWide) {
        bytes[bytes_read + 1] = '\0';
        text_file.text = WideString(reinterpret_cast<const wchar_t *>(bytes + 2));
    } else {
        const char *utf8_text = reinterpret_cast<const char *>(bytes + (text_file.encoding == kTextEncodingUtf8WithBom ? 3 : 0));
        text_file.text = WideString::FromUtf8(utf8_text);
    }

    TextBox *text_box = active_text_box_;
    if (Win32Api::document_interface() == Win32Api::kDocumentMdi) {
        text_box = CreateMdiChild();
    } else if (Win32Api::document_interface() == Win32Api::kDocumentTdi) {
        CreateTdiChild(TEXT("NEW WINDOW"));
        text_box = active_text_box_;
    }

    if (text_box) {
        text_box->SetFile(text_file);
    }

    TextEditor::memory::Free(bytes);
    CloseHandle(file);
}

void MainWindow::SaveFile() {
    TextBox *text_box = CurrentTextBox();
    if (!text_box) {
        return;
    }

    WideString file_name;
    HANDLE file = Win32Api::ShowFileDialog(hwnd_, false, &file_name);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    const BYTE bom[2] = { 0xFF, 0xFE };
    DWORD bytes_written = 0;
    const wchar_t *text = text_box->file().text.c_str();

    if (!WriteFile(file, bom, 2, &bytes_written, NULL) ||
        !WriteFile(file, text, static_cast<DWORD>(wcslen(text) * sizeof(WCHAR)), &bytes_written, NULL)) {
        Win32Api::Warning(TEXT("Failed to save file."));
    }

    CloseHandle(file);
}

void MainWindow::Exit() {
    SendMessage(hwnd_, WM_CLOSE, 0, 0);
}

void MainWindow::OnRibbonCommand(UINT32 command_id) {
    switch (command_id) {
        case ID_MAIN_WINDOW_MENU_FILE_OPEN_RIBBON:
            OpenFile();
            break;

        case ID_MAIN_WINDOW_MENU_FILE_SAVE_RIBBON:
            SaveFile();
            break;

        case ID_MAIN_WINDOW_MENU_FILE_EXIT_RIBBON:
            Exit();
            break;
    }
}

void MainWindow::OnRibbonHeightChanged(UINT32 ribbon_height) {
    UNREFERENCED_PARAMETER(ribbon_height);
    UpdateLayout();
}

LRESULT CALLBACK MainWindow::WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
    if (message == WM_CREATE) {
        CREATESTRUCT *create = reinterpret_cast<CREATESTRUCT *>(l_param);
        MainWindow *window = reinterpret_cast<MainWindow *>(create->lpCreateParams);
        window->hwnd_ = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        return window->OnCreate();
    }

    MainWindow *window = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!window) {
        return DefWindowProc(hwnd, message, w_param, l_param);
    }

    switch (message) {
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        case WM_IME_COMPOSITION:
        case WM_CHAR:
        case WM_KEYDOWN:
            if (window->active_text_box_) {
                SendMessage(window->active_text_box_->hwnd(), message, w_param, l_param);
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(w_param)) {
                case ID_MAIN_WINDOW_MENU_FILE_OPEN:
                    window->OpenFile();
                    return 0;

                case ID_MAIN_WINDOW_MENU_FILE_SAVE:
                    window->SaveFile();
                    return 0;

                case ID_MAIN_WINDOW_MENU_FILE_EXIT:
                    window->Exit();
                    return 0;
            }
            break;

        case WM_SIZE:
            window->OnSize(l_param);
            return 0;

        case WM_NOTIFY:
            window->OnNotify(l_param);
            return 0;

        case WM_SYSCOLORCHANGE:
            if (window->toolbar_) {
                SendMessage(window->toolbar_, WM_SYSCOLORCHANGE, 0, 0);
            }
            return 0;

        case WM_CLOSE:
            window->OnClose();
            return 0;

        case WM_DESTROY:
            window->OnDestroy();
            return 0;
    }

    if (Win32Api::document_interface() == Win32Api::kDocumentMdi) {
        return DefFrameProc(hwnd, window->mdi_client_, message, w_param, l_param);
    }

    return DefWindowProc(hwnd, message, w_param, l_param);
}

LRESULT CALLBACK MainWindow::MdiClientProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
    MainWindow *window = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (window && message == WM_ERASEBKGND && Win32Api::style() == Win32Api::kStyleAero) {
        HDC hdc = reinterpret_cast<HDC>(w_param);
        RECT rect = { 0 };
        GetClientRect(hwnd, &rect);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        return TRUE;
    }

    return CallWindowProc(window ? window->mdi_original_proc_ : DefWindowProc, hwnd, message, w_param, l_param);
}

LRESULT CALLBACK MainWindow::TabControlProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
    MainWindow *window = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!window) {
        return DefWindowProc(hwnd, message, w_param, l_param);
    }

    if (message == WM_PAINT) {
        HTHEME theme = Win32Api::OpenThemeData() ? Win32Api::OpenThemeData()(hwnd, L"REBAR") : NULL;
        if (theme) {
            RECT full_rect = { 0 };
            GetClientRect(hwnd, &full_rect);

            RECT full_normal_rect = { 0 };
            full_normal_rect.right = full_rect.right - full_rect.left;
            full_normal_rect.bottom = full_rect.bottom - full_rect.top;

            if (!window->tab_memory_hdc_) {
                HDC display = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
                window->tab_memory_hdc_ = CreateCompatibleDC(display);
                window->tab_bitmap_ = CreateCompatibleBitmap(display, full_normal_rect.right, full_normal_rect.bottom);
                window->tab_bitmap_old_ = reinterpret_cast<HBITMAP>(SelectObject(window->tab_memory_hdc_, window->tab_bitmap_));
                DeleteDC(display);
            }

            PAINTSTRUCT paint = { 0 };
            HDC hdc = BeginPaint(hwnd, &paint);
            SendMessage(hwnd, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(window->tab_memory_hdc_), PRF_CLIENT);

            HRGN tab_region = CreateRectRgn(0, 0, 0, 0);
            RECT tab_rect = { 0 };
            for (int i = 0; i < TabCtrl_GetItemCount(hwnd); ++i) {
                TabCtrl_GetItemRect(hwnd, i, &tab_rect);
                HRGN single_region = CreateRectRgn(tab_rect.left, tab_rect.top, tab_rect.right, tab_rect.bottom);
                CombineRgn(tab_region, tab_region, single_region, RGN_OR);
                DeleteObject(single_region);
            }

            GetRgnBox(tab_region, &tab_rect);
            DeleteObject(tab_region);

            RECT toolbar_rect = { 0 };
            if (window->toolbar_) {
                GetClientRect(window->toolbar_, &toolbar_rect);
            }

            RECT theme_rect = { 0 };
            theme_rect.left = tab_rect.right - tab_rect.left + 2;
            theme_rect.right = full_normal_rect.right;
            theme_rect.top = -toolbar_rect.bottom;
            theme_rect.bottom = tab_rect.bottom - tab_rect.top + 2;

            Win32Api::DrawThemeBackground()(theme, hdc, RP_BACKGROUND, 0, &theme_rect, NULL);
            Win32Api::CloseThemeData()(theme);

            BitBlt(hdc, 0, 0, tab_rect.right + 2, tab_rect.bottom, window->tab_memory_hdc_, 0, 0, SRCCOPY);
            EndPaint(hwnd, &paint);
            return 0;
        }
    } else if (message == WM_DESTROY && window->tab_memory_hdc_) {
        SelectObject(window->tab_memory_hdc_, window->tab_bitmap_old_);
        DeleteObject(window->tab_bitmap_);
        DeleteDC(window->tab_memory_hdc_);
        window->tab_memory_hdc_ = NULL;
        window->tab_bitmap_ = NULL;
        window->tab_bitmap_old_ = NULL;
    }

    return CallWindowProc(window->tab_original_proc_, hwnd, message, w_param, l_param);
}

LRESULT MainWindow::OnCreate() {
    if (!ribbon_host_.Initialize(hwnd_, this)) {
        CreateMainToolbar();
    }

    if (Win32Api::document_interface() == Win32Api::kDocumentMdi) {
        CLIENTCREATESTRUCT client = { 0 };
        mdi_client_ = CreateWindow(TEXT("MDICLIENT"),
                                   NULL,
                                   WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                                   0,
                                   0,
                                   size_.x,
                                   size_.y,
                                   hwnd_,
                                   NULL,
                                   NULL,
                                   &client);

        SetWindowLongPtr(mdi_client_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        mdi_original_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtr(mdi_client_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MdiClientProcedure)));
        active_text_box_ = CreateMdiChild();
    } else if (Win32Api::document_interface() == Win32Api::kDocumentSdi) {
        active_text_box_ = new TextBox(size_, &font_file_, &font_name_);
        if (!active_text_box_->CreateChildWindow(hwnd_, 0)) {
            Win32Api::Error(TEXT("Failed to create text box."));
        }
    } else {
        RECT parent_size = { 0 };
        RECT menu_size = { 0 };
        GetClientRect(hwnd_, &parent_size);
        if (toolbar_) {
            GetClientRect(toolbar_, &menu_size);
        }

        tab_control_ = CreateWindow(WC_TABCONTROL,
                                    TEXT(""),
                                    WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                    0,
                                    menu_size.bottom,
                                    parent_size.right,
                                    parent_size.bottom - menu_size.bottom,
                                    hwnd_,
                                    NULL,
                                    GetModuleHandle(NULL),
                                    NULL);

        SetWindowLongPtr(tab_control_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        tab_original_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtr(tab_control_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(TabControlProcedure)));

        UpdateTdiStripHeight();
        CreateTdiChild(WideString(Strings::Untitled()).ToTString().c_str());
    }

    return 0;
}

void MainWindow::OnSize(LPARAM l_param) {
    size_.x = LOWORD(l_param);
    size_.y = HIWORD(l_param);
    UpdateLayout();
}

void MainWindow::OnNotify(LPARAM l_param) {
    NMHDR *header = reinterpret_cast<NMHDR *>(l_param);

    if (header->hwndFrom == tab_control_ && header->code == TCN_SELCHANGING) {
        SelectTdi(TextBox::TdiSize() - TabCtrl_GetCurSel(tab_control_) - 1);
        return;
    }

    if (header->hwndFrom == toolbar_ && header->code == RBN_ENDDRAG) {
        UpdateLayout();
        return;
    }

    if (header->code == TBN_DROPDOWN) {
        NMTOOLBAR *toolbar = reinterpret_cast<NMTOOLBAR *>(header);
        if (toolbar->iItem == ID_MAIN_WINDOW_MENU_FILE) {
            HMENU popup = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MAIN_WINDOW_MENU));
            popup = GetSubMenu(popup, 0);

            RECT rect = { 0 };
            SendMessage(header->hwndFrom, TB_GETRECT, toolbar->iItem, reinterpret_cast<LPARAM>(&rect));
            MapWindowPoints(header->hwndFrom, HWND_DESKTOP, reinterpret_cast<LPPOINT>(&rect), 2);
            TrackPopupMenu(popup, TPM_LEFTALIGN | TPM_LEFTBUTTON, rect.left, rect.bottom, 0, hwnd_, NULL);
            DestroyMenu(popup);
        }
    }
}

void MainWindow::OnClose() {
    const WideString message(L"Are you sure you want to exit?");
    if (MessageBox(hwnd_, message.ToTString().c_str(), message.ToTString().c_str(), MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
        DestroyWindow(hwnd_);
    }
}

void MainWindow::OnDestroy() {
    ribbon_host_.Destroy();
    PostQuitMessage(0);
}

bool MainWindow::CreateMainToolbar() {
    const TBBUTTON buttons[2] = {
        { MAKELONG(STD_FILEOPEN, 0), ID_MAIN_WINDOW_MENU_FILE_OPEN, TBSTATE_ENABLED, BTNS_AUTOSIZE, { 0 }, 0, reinterpret_cast<INT_PTR>(TEXT("Open")) },
        { MAKELONG(STD_FILESAVE, 0), ID_MAIN_WINDOW_MENU_FILE_SAVE, TBSTATE_ENABLED, BTNS_AUTOSIZE, { 0 }, 0, reinterpret_cast<INT_PTR>(TEXT("Save")) }
    };

    const HWND rebar = CreateWindowEx(WS_EX_TOOLWINDOW,
                                      REBARCLASSNAME,
                                      NULL,
                                      WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_BANDBORDERS | RBS_VARHEIGHT | CCS_NODIVIDER,
                                      0,
                                      0,
                                      0,
                                      0,
                                      hwnd_,
                                      NULL,
                                      NULL,
                                      NULL);

    if (!rebar) {
        toolbar_ = CreateToolbar(hwnd_, false, buttons);
        return toolbar_ != NULL;
    }

    toolbar_ = rebar;

    const HWND menu = CreateWindowEx(0,
                                     TOOLBARCLASSNAME,
                                     NULL,
                                     WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT | CCS_NODIVIDER | CCS_NORESIZE,
                                     0,
                                     0,
                                     0,
                                     0,
                                     rebar,
                                     NULL,
                                     NULL,
                                     NULL);
    if (menu) {
        const TBBUTTON file_button[1] = {
            { I_IMAGENONE, ID_MAIN_WINDOW_MENU_FILE, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_DROPDOWN, { 0 }, 0, reinterpret_cast<INT_PTR>(TEXT("File")) }
        };

        SendMessage(menu, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        SendMessage(menu, TB_ADDBUTTONS, 1, reinterpret_cast<LPARAM>(file_button));
        SendMessage(menu, TB_AUTOSIZE, 0, 0);
    }

    const HWND toolbar = CreateToolbar(rebar, true, buttons);

    REBARBANDINFO band = { 0 };
    band.cbSize = REBARBANDINFO_V3_SIZE;
    band.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE;

    if (menu) {
        const DWORD size = static_cast<DWORD>(SendMessage(menu, TB_GETBUTTONSIZE, 0, 0));
        band.hwndChild = menu;
        band.cxMinChild = LOWORD(size);
        band.cyMinChild = HIWORD(size);
        band.cyChild = HIWORD(size);
        SendMessage(rebar, RB_INSERTBAND, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(&band));
    }

    if (toolbar) {
        const DWORD size = static_cast<DWORD>(SendMessage(toolbar, TB_GETBUTTONSIZE, 0, 0));
        band.hwndChild = toolbar;
        band.fStyle = RBBS_BREAK;
        band.cxMinChild = LOWORD(size) * 2;
        band.cyMinChild = HIWORD(size);
        band.cyChild = HIWORD(size);
        SendMessage(rebar, RB_INSERTBAND, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(&band));
    }

    return true;
}

HWND MainWindow::CreateToolbar(HWND parent, bool rebar, const TBBUTTON *buttons) {
    DWORD style = WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE;
    if (rebar) {
        style = WS_CHILD | TBSTYLE_TRANSPARENT | CCS_NODIVIDER | CCS_NORESIZE;
    }

    HWND toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, style, 0, 0, 0, 0, parent, NULL, NULL, NULL);
    if (toolbar) {
        const TBADDBITMAP bitmap = { HINST_COMMCTRL, IDB_STD_SMALL_COLOR };
        SendMessage(toolbar, TB_ADDBITMAP, 2, reinterpret_cast<LPARAM>(&bitmap));
        SendMessage(toolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        SendMessage(toolbar, TB_ADDBUTTONS, 2, reinterpret_cast<LPARAM>(buttons));
        SendMessage(toolbar, TB_AUTOSIZE, 0, 0);
    }

    return toolbar;
}

TextBox *MainWindow::CreateMdiChild() {
    TextBox *text_box = new TextBox(size_, &font_file_, &font_name_);
    RECT rect = { 0 };
    GetWindowRect(hwnd_, &rect);
    text_box->CreateMdiChild(mdi_client_, Point(rect.right / 2, rect.bottom / 2));
    return text_box;
}

void MainWindow::CreateTdiChild(LPCTSTR title) {
    const int index = TextBox::TdiSize();
    AddTdiEntry(title, index);

    active_text_box_ = new TextBox(tdi_size_, &font_file_, &font_name_);
    active_text_box_->CreateChildWindow(tab_control_, tdi_strip_height_);
    SelectTdi(index);
}

void MainWindow::AddTdiEntry(LPCTSTR title, int index) {
    TCITEM item = { 0 };
    item.mask = TCIF_TEXT;
    item.pszText = const_cast<LPTSTR>(title);
    item.cchTextMax = static_cast<int>(CountCharacters(title));
    item.iImage = -1;
    TabCtrl_InsertItem(tab_control_, index, &item);
}

void MainWindow::UpdateTdiStripHeight() {
    AddTdiEntry(TEXT(""), 0);
    RECT rect = { 0 };
    TabCtrl_GetItemRect(tab_control_, 0, &rect);
    tdi_strip_height_ = rect.bottom - rect.top;
    TabCtrl_DeleteItem(tab_control_, 0);

    tdi_size_ = Point(size_.x, size_.y - tdi_strip_height_);
}

void MainWindow::SelectTdi(int index) {
    TabCtrl_SetCurSel(tab_control_, index);
    TextBox::SelectTdi(index);
    active_text_box_ = TextBox::FindTdi(index);
}

void MainWindow::UpdateLayout() {
    int toolbar_height = 0;
    if (toolbar_) {
        RECT toolbar_rect = { 0 };
        GetWindowRect(toolbar_, &toolbar_rect);
        toolbar_height = toolbar_rect.bottom - toolbar_rect.top;
        MoveWindow(toolbar_, 0, 0, size_.x, toolbar_height, TRUE);
    }

    int top_height = static_cast<int>(ribbon_host_.height()) + toolbar_height;

    if (mdi_client_) {
        MoveWindow(mdi_client_, 0, top_height, size_.x, size_.y - top_height, FALSE);
        TextBox::RedrawMdi();
    } else if (tab_control_) {
        MoveWindow(tab_control_, 0, top_height, size_.x, size_.y - top_height, FALSE);
        UpdateTdiStripHeight();
        if (active_text_box_) {
            SetWindowPos(active_text_box_->hwnd(), NULL, 0, tdi_strip_height_, size_.x, size_.y - top_height - tdi_strip_height_, SWP_DRAWFRAME);
        }
    } else if (active_text_box_) {
        MoveWindow(active_text_box_->hwnd(), 0, top_height, size_.x, size_.y - top_height, FALSE);
    }
}

TextBox *MainWindow::CurrentTextBox() {
    if (Win32Api::document_interface() == Win32Api::kDocumentMdi && mdi_client_) {
        return TextBox::Find(reinterpret_cast<HWND>(SendMessage(mdi_client_, WM_MDIGETACTIVE, 0, 0)));
    }

    return active_text_box_;
}

}  // namespace TextEditor
