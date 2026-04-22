#include <leak_checker.h>

#include <main_window_document_host.h>

#include <main_window.h>

namespace TextEditor {

namespace {

class SdiDocumentHost : public MainWindowDocumentHost {
public:
    SdiDocumentHost()
        : text_box_(NULL) {
    }

    virtual void UpdateLayout(int top_height) {
        if (text_box_) {
            MoveWindow(text_box_->hwnd(), 0, top_height, window_->size().x, window_->size().y - top_height, FALSE);
        }
    }

    virtual TextBox *CurrentTextBox() const {
        return text_box_;
    }

    virtual TextBox *CreateTextBoxForOpen() {
        return text_box_;
    }

private:
    virtual bool OnInitialize() {
        text_box_ = new TextBox(window_->size(), &window_->font_file(), &window_->font_name());
        if (!text_box_->CreateChildWindow(window_->hwnd(), 0)) {
            delete text_box_;
            text_box_ = NULL;
            return false;
        }

        return true;
    }

    TextBox *text_box_;
};

}  // namespace

MainWindowDocumentHost *CreateMainWindowSdiDocumentHost() {
    return new SdiDocumentHost();
}

}  // namespace TextEditor
