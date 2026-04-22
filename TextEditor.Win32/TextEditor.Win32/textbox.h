#ifndef TEXTEDITOR_TEXTBOX_H
#define TEXTEDITOR_TEXTBOX_H

#include <Windows.h>

#include <XY.h>
#include <atom_wrapper.h>
#include <string_utilities.h>
#include <vector.h>
#include <windows_helper.h>

namespace TextEditor {

class TextFile {
public:
    TextFile();

    WideString name;
    WideString text;
    TextEncoding encoding;
};

class TextBox {
public:
    TextBox(const Point &size, const WideString *font_file, const WideString *font_name);
    ~TextBox();

    bool CreateChildWindow(HWND parent, int vertical_offset);
    bool CreateMdiChild(HWND mdi_client, const Point &parent_size);

    HWND hwnd() const;
    HWND status_bar() const;
    const TextFile &file() const;

    void SetFile(const TextFile &text_file);
    void RequestRedraw() const;
    void SetCaretPosition(int x, int y);

    static TextBox *Find(HWND hwnd);
    static TextBox *FindTdi(size_t index);
    static void SelectTdi(size_t index);
    static int TdiSize();
    static void RedrawMdi();

private:
    struct FontState {
        FontState()
            : handle(NULL),
              size(6, 16),
              file(NULL),
              name(NULL) {
        }

        HFONT handle;
        Point size;
        const WideString *file;
        const WideString *name;
        TStringBuffer file_buffer;
        TStringBuffer name_buffer;
    };

    static bool EnsureWindowClass();
    static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
    static TextBox *FromWindow(HWND hwnd);

    LRESULT OnCreate();
    void OnDestroy();
    void OnNonClientDestroy();
    void OnSize(LPARAM l_param);
    void OnSetFocus();
    void OnKillFocus();
    void OnImeComposition(LPARAM l_param);
    void OnChar(WPARAM w_param);
    void OnKeyDown(WPARAM w_param);
    void OnPaint();

    void CreateCaretHandle();
    void UpdateTitle();
    void UpdateCaretPixels(HDC hdc);
    WideString EncodingLabel() const;
    void ResetMultibyteBuffer();

    static WindowClass window_class_;
    static bool window_class_registered_;
    static Vector<TextBox *> instances_;

    HWND hwnd_;
    HWND status_bar_hwnd_;
    Point size_;
    Point caret_;
    Point caret_pixels_;
    TextFile file_;
    FontState font_;
    bool focus_;
    WideString ime_text_;
#ifndef UNICODE
    char multibyte_buffer_[3];
#endif
};

}  // namespace TextEditor

#endif
