#include <leak_checker.h>

#include <main_window_document_host.h>

namespace TextEditor {

MainWindowDocumentHost::MainWindowDocumentHost()
    : window_(NULL) {
}

MainWindowDocumentHost::~MainWindowDocumentHost() {
}

bool MainWindowDocumentHost::Initialize(MainWindow *window) {
    window_ = window;
    return OnInitialize();
}

bool MainWindowDocumentHost::HandleNotify(LPARAM l_param) {
    UNREFERENCED_PARAMETER(l_param);
    return false;
}

LRESULT MainWindowDocumentHost::DefaultWindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) const {
    return DefWindowProc(hwnd, message, w_param, l_param);
}

MainWindowDocumentHost *CreateMainWindowDocumentHost(Win32Api::DocumentInterface document_interface) {
    switch (document_interface) {
        case Win32Api::kDocumentMdi:
            return CreateMainWindowMdiDocumentHost();

        case Win32Api::kDocumentSdi:
            return CreateMainWindowSdiDocumentHost();

        case Win32Api::kDocumentTdi:
            return CreateMainWindowTdiDocumentHost();

        default:
            return NULL;
    }
}

}  // namespace TextEditor
