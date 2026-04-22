#include <leak_checker.h>

#include <main_window.h>

#include <CommCtrl.h>
#include <main_window_document_host.h>
#include <resource.h>
#include <ribbon.h>
#include <strings.h>

namespace TextEditor {

WindowClass MainWindow::window_class_;
bool MainWindow::window_class_registered_ = false;

MainWindow::MainWindow()
    : hwnd_(NULL),
      toolbar_(NULL),
      size_(0, 0),
      font_file_(L"unifont-15.1.05.otf"),
      font_name_(L"Unifont"),
      document_host_(NULL) {
}

MainWindow::~MainWindow() {
    delete document_host_;
}

bool MainWindow::Initialize() {
    if (!window_class_registered_) {
        window_class_registered_ = window_class_.Register(WideString(Strings::MainWindowTitle()), 0, WindowProcedure);
    }

    const TStringBuffer title = WideString(Strings::MainWindowTitle()).ToTString();
    hwnd_ = CreateWindow(MAKEINTATOM(window_class_.atom()),
                         title.c_str(),
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
        case WM_KEYDOWN: {
            TextBox *text_box = window->CurrentTextBox();
            if (text_box) {
                SendMessage(text_box->hwnd(), message, w_param, l_param);
            }
            return 0;
        }

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

    if (window->document_host_) {
        return window->document_host_->DefaultWindowProcedure(hwnd, message, w_param, l_param);
    }

    return DefWindowProc(hwnd, message, w_param, l_param);
}

LRESULT MainWindow::OnCreate() {
    if (!ribbon_host_.Initialize(hwnd_, this)) {
        CreateMainToolbar();
    }

    document_host_ = CreateMainWindowDocumentHost(Win32Api::document_interface());
    if (!document_host_ || !document_host_->Initialize(this)) {
        delete document_host_;
        document_host_ = NULL;
        return -1;
    }

    UpdateLayout();
    return 0;
}

void MainWindow::OnSize(LPARAM l_param) {
    size_.x = LOWORD(l_param);
    size_.y = HIWORD(l_param);
    UpdateLayout();
}

void MainWindow::OnNotify(LPARAM l_param) {
    if (document_host_ && document_host_->HandleNotify(l_param)) {
        return;
    }

    NMHDR *header = reinterpret_cast<NMHDR *>(l_param);
    if (!header) {
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
    delete document_host_;
    document_host_ = NULL;
    PostQuitMessage(0);
}

void MainWindow::UpdateLayout() {
    int toolbar_height = 0;
    if (toolbar_) {
        RECT toolbar_rect = { 0 };
        GetWindowRect(toolbar_, &toolbar_rect);
        toolbar_height = toolbar_rect.bottom - toolbar_rect.top;
        MoveWindow(toolbar_, 0, 0, size_.x, toolbar_height, TRUE);
    }

    if (document_host_) {
        document_host_->UpdateLayout(static_cast<int>(ribbon_host_.height()) + toolbar_height);
    }
}

TextBox *MainWindow::CurrentTextBox() const {
    return document_host_ ? document_host_->CurrentTextBox() : NULL;
}

}  // namespace TextEditor
