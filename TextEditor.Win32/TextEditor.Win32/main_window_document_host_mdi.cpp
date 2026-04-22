#include <leak_checker.h>

#include <main_window_document_host.h>

#include <main_window.h>
#include <windows_helper.h>

namespace TextEditor {

namespace {

class MdiDocumentHost : public MainWindowDocumentHost {
public:
    MdiDocumentHost()
        : mdi_client_(NULL),
          original_proc_(NULL) {
    }

    virtual void UpdateLayout(int top_height) {
        if (mdi_client_) {
            MoveWindow(mdi_client_, 0, top_height, window_->size().x, window_->size().y - top_height, FALSE);
            TextBox::RedrawMdi();
        }
    }

    virtual TextBox *CurrentTextBox() const {
        if (!mdi_client_) {
            return NULL;
        }

        return TextBox::Find(reinterpret_cast<HWND>(SendMessage(mdi_client_, WM_MDIGETACTIVE, 0, 0)));
    }

    virtual TextBox *CreateTextBoxForOpen() {
        return CreateMdiChild();
    }

    virtual LRESULT DefaultWindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) const {
        return DefFrameProc(hwnd, mdi_client_, message, w_param, l_param);
    }

private:
    static LRESULT CALLBACK MdiClientProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
        MdiDocumentHost *host = reinterpret_cast<MdiDocumentHost *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (host && message == WM_ERASEBKGND && Win32Api::style() == Win32Api::kStyleAero) {
            HDC hdc = reinterpret_cast<HDC>(w_param);
            RECT rect = { 0 };
            GetClientRect(hwnd, &rect);
            HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            return TRUE;
        }

        return CallWindowProc(host ? host->original_proc_ : DefWindowProc, hwnd, message, w_param, l_param);
    }

    virtual bool OnInitialize() {
        CLIENTCREATESTRUCT client = { 0 };
        mdi_client_ = CreateWindow(TEXT("MDICLIENT"),
                                   NULL,
                                   WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                                   0,
                                   0,
                                   window_->size().x,
                                   window_->size().y,
                                   window_->hwnd(),
                                   NULL,
                                   NULL,
                                   &client);
        if (!mdi_client_) {
            return false;
        }

        SetWindowLongPtr(mdi_client_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        original_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtr(mdi_client_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MdiClientProcedure)));
        return CreateTextBoxForOpen() != NULL;
    }

    TextBox *CreateMdiChild() {
        TextBox *text_box = new TextBox(window_->size(), &window_->font_file(), &window_->font_name());

        RECT rect = { 0 };
        GetWindowRect(window_->hwnd(), &rect);
        if (!text_box->CreateMdiChild(mdi_client_, Point(rect.right / 2, rect.bottom / 2))) {
            delete text_box;
            return NULL;
        }

        return text_box;
    }

    HWND mdi_client_;
    WNDPROC original_proc_;
};

}  // namespace

MainWindowDocumentHost *CreateMainWindowMdiDocumentHost() {
    return new MdiDocumentHost();
}

}  // namespace TextEditor
