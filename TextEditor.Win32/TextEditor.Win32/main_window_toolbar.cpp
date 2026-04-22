#include <leak_checker.h>

#include <main_window.h>

#include <CommCtrl.h>
#include <resource.h>

namespace TextEditor {

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

}  // namespace TextEditor
