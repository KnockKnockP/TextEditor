#ifndef TEXTEDITOR_MAIN_WINDOW_H
#define TEXTEDITOR_MAIN_WINDOW_H

#include <Windows.h>

#include <XY.h>
#include <atom_wrapper.h>
#include <ribbon_host.h>
#include <textbox.h>

namespace TextEditor {

class MainWindowDocumentHost;

class MainWindow : public RibbonHost::Delegate {
public:
    MainWindow();
    ~MainWindow();

    bool Initialize();
    void OpenFile();
    void SaveFile();
    void Exit();

    HWND hwnd() const {
        return hwnd_;
    }

    HWND toolbar() const {
        return toolbar_;
    }

    const Point &size() const {
        return size_;
    }

    const WideString &font_file() const {
        return font_file_;
    }

    const WideString &font_name() const {
        return font_name_;
    }

    virtual void OnRibbonCommand(UINT32 command_id);
    virtual void OnRibbonHeightChanged(UINT32 ribbon_height);

private:
    static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);

    LRESULT OnCreate();
    void OnSize(LPARAM l_param);
    void OnNotify(LPARAM l_param);
    void OnClose();
    void OnDestroy();

    bool CreateMainToolbar();
    HWND CreateToolbar(HWND parent, bool rebar, const TBBUTTON *buttons);
    void UpdateLayout();
    TextBox *CurrentTextBox() const;

    static WindowClass window_class_;
    static bool window_class_registered_;

    HWND hwnd_;
    HWND toolbar_;
    Point size_;
    WideString font_file_;
    WideString font_name_;
    RibbonHost ribbon_host_;
    MainWindowDocumentHost *document_host_;
};

}  // namespace TextEditor

#endif
