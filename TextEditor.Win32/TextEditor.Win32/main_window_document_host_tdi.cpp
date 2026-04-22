#include <leak_checker.h>

#include <main_window_document_host.h>

#include <CommCtrl.h>
#include <Uxtheme.h>
#include <main_window.h>
#include <strings.h>
#include <vssym32.h>
#include <windows_helper.h>

namespace TextEditor {

namespace {

class TdiDocumentHost : public MainWindowDocumentHost {
public:
    TdiDocumentHost()
        : tab_control_(NULL),
          original_proc_(NULL),
          tdi_size_(0, 0),
          active_text_box_(NULL),
          strip_height_(0),
          tab_bitmap_(NULL),
          tab_bitmap_old_(NULL),
          tab_memory_hdc_(NULL) {
    }

    virtual ~TdiDocumentHost() {
        ReleaseTabBuffer();
    }

    virtual bool HandleNotify(LPARAM l_param) {
        NMHDR *header = reinterpret_cast<NMHDR *>(l_param);
        if (header && header->hwndFrom == tab_control_ && header->code == TCN_SELCHANGING) {
            SelectTdi(TextBox::TdiSize() - TabCtrl_GetCurSel(tab_control_) - 1);
            return true;
        }

        return false;
    }

    virtual void UpdateLayout(int top_height) {
        if (!tab_control_) {
            return;
        }

        MoveWindow(tab_control_, 0, top_height, window_->size().x, window_->size().y - top_height, FALSE);
        UpdateTdiStripHeight();
        if (active_text_box_) {
            SetWindowPos(active_text_box_->hwnd(),
                         NULL,
                         0,
                         strip_height_,
                         window_->size().x,
                         window_->size().y - top_height - strip_height_,
                         SWP_DRAWFRAME);
        }
    }

    virtual TextBox *CurrentTextBox() const {
        return active_text_box_;
    }

    virtual TextBox *CreateTextBoxForOpen() {
        CreateTdiChild(TEXT("NEW WINDOW"));
        return active_text_box_;
    }

private:
    static LRESULT CALLBACK TabControlProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
        TdiDocumentHost *host = reinterpret_cast<TdiDocumentHost *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (!host) {
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

                if (!host->tab_memory_hdc_) {
                    HDC display = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
                    host->tab_memory_hdc_ = CreateCompatibleDC(display);
                    host->tab_bitmap_ = CreateCompatibleBitmap(display, full_normal_rect.right, full_normal_rect.bottom);
                    host->tab_bitmap_old_ = reinterpret_cast<HBITMAP>(SelectObject(host->tab_memory_hdc_, host->tab_bitmap_));
                    DeleteDC(display);
                }

                PAINTSTRUCT paint = { 0 };
                HDC hdc = BeginPaint(hwnd, &paint);
                SendMessage(hwnd, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(host->tab_memory_hdc_), PRF_CLIENT);

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
                if (host->window_->toolbar()) {
                    GetClientRect(host->window_->toolbar(), &toolbar_rect);
                }

                RECT theme_rect = { 0 };
                theme_rect.left = tab_rect.right - tab_rect.left + 2;
                theme_rect.right = full_normal_rect.right;
                theme_rect.top = -toolbar_rect.bottom;
                theme_rect.bottom = tab_rect.bottom - tab_rect.top + 2;

                Win32Api::DrawThemeBackground()(theme, hdc, RP_BACKGROUND, 0, &theme_rect, NULL);
                Win32Api::CloseThemeData()(theme);

                BitBlt(hdc, 0, 0, tab_rect.right + 2, tab_rect.bottom, host->tab_memory_hdc_, 0, 0, SRCCOPY);
                EndPaint(hwnd, &paint);
                return 0;
            }
        } else if (message == WM_DESTROY) {
            host->ReleaseTabBuffer();
        }

        return CallWindowProc(host->original_proc_, hwnd, message, w_param, l_param);
    }

    virtual bool OnInitialize() {
        tab_control_ = CreateWindow(WC_TABCONTROL,
                                    TEXT(""),
                                    WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                    0,
                                    0,
                                    window_->size().x,
                                    window_->size().y,
                                    window_->hwnd(),
                                    NULL,
                                    GetModuleHandle(NULL),
                                    NULL);
        if (!tab_control_) {
            return false;
        }

        SetWindowLongPtr(tab_control_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        original_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtr(tab_control_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(TabControlProcedure)));

        UpdateTdiStripHeight();
        const TStringBuffer untitled = WideString(Strings::Untitled()).ToTString();
        CreateTdiChild(untitled.c_str());
        return active_text_box_ != NULL;
    }

    void ReleaseTabBuffer() {
        if (tab_memory_hdc_) {
            SelectObject(tab_memory_hdc_, tab_bitmap_old_);
            DeleteObject(tab_bitmap_);
            DeleteDC(tab_memory_hdc_);
            tab_memory_hdc_ = NULL;
            tab_bitmap_ = NULL;
            tab_bitmap_old_ = NULL;
        }
    }

    void CreateTdiChild(LPCTSTR title) {
        const int index = TextBox::TdiSize();
        AddTdiEntry(title, index);

        TextBox *text_box = new TextBox(tdi_size_, &window_->font_file(), &window_->font_name());
        if (!text_box->CreateChildWindow(tab_control_, static_cast<int>(strip_height_))) {
            delete text_box;
            TabCtrl_DeleteItem(tab_control_, index);
            return;
        }

        active_text_box_ = text_box;
        SelectTdi(index);
    }

    void AddTdiEntry(LPCTSTR title, int index) {
        TCITEM item = { 0 };
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<LPTSTR>(title);
        item.cchTextMax = static_cast<int>(CountCharacters(title));
        item.iImage = -1;
        TabCtrl_InsertItem(tab_control_, index, &item);
    }

    void UpdateTdiStripHeight() {
        AddTdiEntry(TEXT(""), 0);

        RECT rect = { 0 };
        TabCtrl_GetItemRect(tab_control_, 0, &rect);
        strip_height_ = rect.bottom - rect.top;
        TabCtrl_DeleteItem(tab_control_, 0);

        tdi_size_ = Point(window_->size().x, window_->size().y - static_cast<int>(strip_height_));
    }

    void SelectTdi(int index) {
        if (!tab_control_ || index < 0 || index >= TextBox::TdiSize()) {
            return;
        }

        TabCtrl_SetCurSel(tab_control_, index);
        TextBox::SelectTdi(static_cast<size_t>(index));
        active_text_box_ = TextBox::FindTdi(static_cast<size_t>(index));
    }

    HWND tab_control_;
    WNDPROC original_proc_;
    Point tdi_size_;
    TextBox *active_text_box_;
    UINT32 strip_height_;
    HBITMAP tab_bitmap_;
    HBITMAP tab_bitmap_old_;
    HDC tab_memory_hdc_;
};

}  // namespace

MainWindowDocumentHost *CreateMainWindowTdiDocumentHost() {
    return new TdiDocumentHost();
}

}  // namespace TextEditor
