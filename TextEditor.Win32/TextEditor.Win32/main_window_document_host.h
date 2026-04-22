#ifndef TEXTEDITOR_MAIN_WINDOW_DOCUMENT_HOST_H
#define TEXTEDITOR_MAIN_WINDOW_DOCUMENT_HOST_H

#include <Windows.h>

#include <windows_helper.h>

namespace TextEditor {

class MainWindow;
class TextBox;

class MainWindowDocumentHost {
public:
    MainWindowDocumentHost();
    virtual ~MainWindowDocumentHost();

    bool Initialize(MainWindow *window);
    virtual bool HandleNotify(LPARAM l_param);
    virtual LRESULT DefaultWindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) const;
    virtual void UpdateLayout(int top_height) = 0;
    virtual TextBox *CurrentTextBox() const = 0;
    virtual TextBox *CreateTextBoxForOpen() = 0;

protected:
    MainWindow *window_;

private:
    virtual bool OnInitialize() = 0;
};

MainWindowDocumentHost *CreateMainWindowDocumentHost(Win32Api::DocumentInterface document_interface);
MainWindowDocumentHost *CreateMainWindowSdiDocumentHost();
MainWindowDocumentHost *CreateMainWindowMdiDocumentHost();
MainWindowDocumentHost *CreateMainWindowTdiDocumentHost();

}  // namespace TextEditor

#endif
