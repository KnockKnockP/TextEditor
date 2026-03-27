#ifndef TEXTEDITOR_MAIN_WINDOW_H
#define TEXTEDITOR_MAIN_WINDOW_H

#include <Windows.h>

#include <XY.h>
#include <atom_wrapper.h>
#include <ribbon_host.h>
#include <textbox.h>

namespace TextEditor {

class MainWindow : public RibbonHost::Delegate {
public:
    MainWindow();
    ~MainWindow();

    bool Initialize();
    void OpenFile();
    void SaveFile();
    void Exit();

    virtual void OnRibbonCommand(UINT32 command_id);
    virtual void OnRibbonHeightChanged(UINT32 ribbon_height);

private:
    static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK MdiClientProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK TabControlProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);

    LRESULT OnCreate();
    void OnSize(LPARAM l_param);
    void OnNotify(LPARAM l_param);
    void OnClose();
    void OnDestroy();

    bool CreateMainToolbar();
    HWND CreateToolbar(HWND parent, bool rebar, const TBBUTTON *buttons);
    TextBox *CreateMdiChild();
    void CreateTdiChild(LPCTSTR title);
    void AddTdiEntry(LPCTSTR title, int index);
    void UpdateTdiStripHeight();
    void SelectTdi(int index);
    void UpdateLayout();
    TextBox *CurrentTextBox();

    static WindowClass window_class_;
    static bool window_class_registered_;

    HWND hwnd_;
    HWND mdi_client_;
    HWND toolbar_;
    HWND tab_control_;
    WNDPROC mdi_original_proc_;
    WNDPROC tab_original_proc_;
    Point size_;
    Point tdi_size_;
    TextBox *active_text_box_;
    WideString font_file_;
    WideString font_name_;
    RibbonHost ribbon_host_;
    UINT32 tdi_strip_height_;

    HBITMAP tab_bitmap_;
    HBITMAP tab_bitmap_old_;
    HDC tab_memory_hdc_;
};

}  // namespace TextEditor

#endif
