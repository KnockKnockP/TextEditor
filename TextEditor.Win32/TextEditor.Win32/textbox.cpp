#include <leak_checker.h>

#include <textbox.h>

#include <commctrl.h>
#include <memory_helper.h>
#include <resource.h>
#include <strings.h>

namespace TextEditor {

WindowClass TextBox::window_class_;
bool TextBox::window_class_registered_ = false;
Vector<TextBox *> TextBox::instances_;

TextFile::TextFile()
    : encoding(kTextEncodingWide) {
}

TextBox::TextBox(const Point &size, const WideString *font_file, const WideString *font_name)
    : hwnd_(NULL),
      status_bar_hwnd_(NULL),
      size_(size),
      caret_(0, 0),
      caret_pixels_(0, 0),
      focus_(false) {
    file_.name = WideString(Strings::Untitled());
    file_.text = WideString();
    file_.encoding = kTextEncodingWide;

    font_.file = font_file;
    font_.name = font_name;

#ifndef UNICODE
    ResetMultibyteBuffer();
#endif
}

TextBox::~TextBox() {
    if (font_.handle) {
        DeleteObject(font_.handle);
    }

    if (Win32Api::RemoveFontResourceEx() && font_.file_buffer.c_str()[0]) {
        Win32Api::RemoveFontResourceEx()(font_.file_buffer.c_str(), FR_PRIVATE, 0);
    }
}

bool TextBox::CreateChildWindow(HWND parent, int vertical_offset) {
    if (!EnsureWindowClass()) {
        return false;
    }

    hwnd_ = CreateWindow(MAKEINTATOM(window_class_.atom()),
                         NULL,
                         WS_CHILD | WS_VISIBLE,
                         0,
                         vertical_offset,
                         size_.x,
                         size_.y,
                         parent,
                         NULL,
                         NULL,
                         this);
    return hwnd_ != NULL;
}

bool TextBox::CreateMdiChild(HWND mdi_client, const Point &parent_size) {
    if (!EnsureWindowClass()) {
        return false;
    }

    size_ = parent_size;

    MDICREATESTRUCT create = { 0 };
    create.szClass = MAKEINTATOM(window_class_.atom());
    create.szTitle = TEXT("Text Box");
    create.x = CW_USEDEFAULT;
    create.y = CW_USEDEFAULT;
    create.cx = parent_size.x / 2;
    create.cy = parent_size.y / 2;
    create.lParam = reinterpret_cast<LPARAM>(this);

    hwnd_ = reinterpret_cast<HWND>(SendMessage(mdi_client, WM_MDICREATE, 0, reinterpret_cast<LPARAM>(&create)));
    return hwnd_ != NULL;
}

HWND TextBox::hwnd() const {
    return hwnd_;
}

HWND TextBox::status_bar() const {
    return status_bar_hwnd_;
}

const TextFile &TextBox::file() const {
    return file_;
}

void TextBox::SetFile(const TextFile &text_file) {
    file_ = text_file;

    const WideString encoding = EncodingLabel();
    const TStringBuffer encoding_text = encoding.ToTString();
    SendMessage(status_bar_hwnd_, SB_SETTEXT, MAKEWPARAM(0, SBT_OWNERDRAW), reinterpret_cast<LPARAM>(encoding_text.c_str()));

    UpdateTitle();
    RequestRedraw();
}

void TextBox::RequestRedraw() const {
    if (hwnd_) {
        InvalidateRect(hwnd_, NULL, FALSE);
    }
}

void TextBox::SetCaretPosition(int x, int y) {
    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        y = 0;
    }

    if (y >= static_cast<int>(file_.text.line_count())) {
        y = static_cast<int>(file_.text.line_count()) - 1;
    }

    const size_t characters = file_.text.line_length(y);
    if (x > static_cast<int>(characters)) {
        x = static_cast<int>(characters);
    }

    caret_.x = x;
    caret_.y = y;
}

TextBox *TextBox::Find(HWND hwnd) {
    for (size_t i = 0; i < instances_.size(); ++i) {
        if (instances_[i] && instances_[i]->hwnd_ == hwnd) {
            return instances_[i];
        }
    }

    return NULL;
}

TextBox *TextBox::FindTdi(size_t index) {
    return instances_[index];
}

void TextBox::SelectTdi(size_t index) {
    for (size_t i = 0; i < instances_.size(); ++i) {
        ShowWindow(instances_[i]->hwnd_, SW_HIDE);
    }

    ShowWindow(instances_[index]->hwnd_, SW_SHOW);
}

int TextBox::TdiSize() {
    return static_cast<int>(instances_.size());
}

void TextBox::RedrawMdi() {
    for (size_t i = 0; i < instances_.size(); ++i) {
        SetWindowPos(instances_[i]->hwnd_, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);
    }
}

bool TextBox::EnsureWindowClass() {
    if (!window_class_registered_) {
        window_class_registered_ = window_class_.Register(WideString(L"Text Box"), CS_HREDRAW | CS_VREDRAW, WindowProcedure);
    }

    return window_class_registered_;
}

LRESULT CALLBACK TextBox::WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
    if (message == WM_CREATE) {
        CREATESTRUCT *create = reinterpret_cast<CREATESTRUCT *>(l_param);
        TextBox *text_box = reinterpret_cast<TextBox *>(create->lpCreateParams);
        text_box->hwnd_ = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(text_box));
        instances_.push_back(text_box);
        return text_box->OnCreate();
    }

    TextBox *text_box = FromWindow(hwnd);
    if (!text_box) {
        return DefWindowProc(hwnd, message, w_param, l_param);
    }

    switch (message) {
        case WM_LBUTTONDOWN:
            SetFocus(hwnd);
            return 0;

        case WM_SETFOCUS:
            text_box->OnSetFocus();
            return 0;

        case WM_KILLFOCUS:
            text_box->OnKillFocus();
            return 0;

        case WM_SIZE:
            text_box->OnSize(l_param);
            return 0;

        case WM_IME_COMPOSITION:
            text_box->OnImeComposition(l_param);
            return 0;

        case WM_CHAR:
            text_box->OnChar(w_param);
            return 0;

        case WM_KEYDOWN:
            text_box->OnKeyDown(w_param);
            return 0;

        case WM_PAINT:
            text_box->OnPaint();
            return 0;

        case WM_DESTROY:
            text_box->OnDestroy();
            return 0;

        case WM_NCDESTROY:
            text_box->OnNonClientDestroy();
            return 0;
    }

    if (Win32Api::document_interface() == Win32Api::kDocumentMdi) {
        return DefMDIChildProc(hwnd, message, w_param, l_param);
    }

    return DefWindowProc(hwnd, message, w_param, l_param);
}

TextBox *TextBox::FromWindow(HWND hwnd) {
    return reinterpret_cast<TextBox *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

LRESULT TextBox::OnCreate() {
    UpdateTitle();

    if (Win32Api::AddFontResourceEx()) {
        font_.file_buffer = font_.file->ToTString();
        if (Win32Api::AddFontResourceEx()(font_.file_buffer.c_str(), FR_PRIVATE, 0)) {
            font_.size = Point(16, 16);
        }

        font_.name_buffer = font_.name->ToTString();
        font_.handle = CreateFont(font_.size.y,
                                  font_.size.x,
                                  0,
                                  0,
                                  0,
                                  FALSE,
                                  FALSE,
                                  FALSE,
                                  DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS,
                                  CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY,
                                  FF_DONTCARE,
                                  font_.name_buffer.c_str());
    }

    const WideString encoding = EncodingLabel();
    const TStringBuffer encoding_text = encoding.ToTString();
    status_bar_hwnd_ = CreateStatusWindow(WS_CHILD | WS_VISIBLE, encoding_text.c_str(), hwnd_, IDR_TEXTBOX_STATUS_BAR);
    return 0;
}

void TextBox::OnDestroy() {
    HideCaret(hwnd_);
    DestroyCaret();
}

void TextBox::OnNonClientDestroy() {
    for (size_t i = 0; i < instances_.size(); ++i) {
        if (instances_[i] == this) {
            instances_.remove_at(i);
            break;
        }
    }

    SetWindowLongPtr(hwnd_, GWLP_USERDATA, 0);
    delete this;
}

void TextBox::OnSize(LPARAM l_param) {
    size_.x = LOWORD(l_param);
    size_.y = HIWORD(l_param);
    SendMessage(status_bar_hwnd_, WM_SIZE, 0, 0);
}

void TextBox::OnSetFocus() {
    focus_ = true;
    CreateCaretHandle();
}

void TextBox::OnKillFocus() {
    focus_ = false;
    HideCaret(hwnd_);
    DestroyCaret();
}

void TextBox::OnImeComposition(LPARAM l_param) {
    if (!Win32Api::ImmGetContext() || !Win32Api::ImmGetCompositionString() || !Win32Api::ImmReleaseContext()) {
        return;
    }

    const HIMC input_context = Win32Api::ImmGetContext()(hwnd_);
    const bool has_composition = (l_param & GCS_COMPSTR) != 0;
    const bool has_result = (l_param & GCS_RESULTSTR) != 0;

    if (has_composition || has_result) {
        const DWORD flag = has_result ? GCS_RESULTSTR : GCS_COMPSTR;
        const LONG bytes = Win32Api::ImmGetCompositionString()(input_context, flag, NULL, 0);

        TCHAR *buffer = new TCHAR[(bytes / sizeof(TCHAR)) + 1];
        Win32Api::ImmGetCompositionString()(input_context, flag, buffer, bytes);
        buffer[bytes / sizeof(TCHAR)] = TEXT('\0');
        ime_text_ = WideString::FromTString(buffer);
        delete[] buffer;

        if (has_result) {
            file_.text.InsertWideAtLine(static_cast<size_t>(caret_.y), static_cast<size_t>(caret_.x), ime_text_.c_str());
            SetCaretPosition(caret_.x + static_cast<int>(ime_text_.length()), caret_.y);
            ime_text_ = WideString();
        }
    }

    Win32Api::ImmReleaseContext()(hwnd_, input_context);
    RequestRedraw();
}

void TextBox::OnChar(WPARAM w_param) {
    if (w_param == VK_BACK) {
        SetCaretPosition(caret_.x - 1, caret_.y);

        const size_t line_characters = file_.text.line_length(caret_.y);
        if (!caret_.x && !line_characters && caret_.y) {
            file_.text.RemoveCharacterAt(file_.text.line_start(caret_.y) - 1);
            SetCaretPosition(static_cast<int>(file_.text.line_length(caret_.y - 1)), caret_.y - 1);
        } else if (line_characters) {
            file_.text.RemoveCharacterAtLine(caret_.y, caret_.x);
        }
    } else {
#ifndef UNICODE
        if (IsDBCSLeadByte(static_cast<BYTE>(w_param))) {
            if (!multibyte_buffer_[0]) {
                multibyte_buffer_[0] = static_cast<char>(w_param);
            } else {
                multibyte_buffer_[1] = static_cast<char>(w_param);
                file_.text.InsertAnsiAtLine(caret_.y, caret_.x, multibyte_buffer_);
                ResetMultibyteBuffer();
                SetCaretPosition(caret_.x + 1, caret_.y);
            }
            RequestRedraw();
            return;
        }
#endif

        wchar_t character = static_cast<wchar_t>(w_param);
        if (character == L'\r') {
            character = L'\n';
            const int previous_line = caret_.y;
            SetCaretPosition(0, caret_.y);
            file_.text.InsertCharAtLine(previous_line, file_.text.line_length(previous_line), character);
            SetCaretPosition(0, previous_line + 1);
        } else {
            file_.text.InsertCharAtLine(caret_.y, caret_.x, character);
            SetCaretPosition(caret_.x + 1, caret_.y);
        }
    }

    RequestRedraw();
}

void TextBox::OnKeyDown(WPARAM w_param) {
    if (w_param == VK_LEFT) {
        SetCaretPosition(caret_.x - 1, caret_.y);
    } else if (w_param == VK_RIGHT) {
        SetCaretPosition(caret_.x + 1, caret_.y);
    } else if (w_param == VK_UP) {
        SetCaretPosition(caret_.x, caret_.y - 1);
    } else if (w_param == VK_DOWN) {
        SetCaretPosition(caret_.x, caret_.y + 1);
    }

    RequestRedraw();
}

void TextBox::OnPaint() {
    PAINTSTRUCT paint = { 0 };
    HDC hdc = BeginPaint(hwnd_, &paint);

    if (font_.handle) {
        SelectObject(hdc, font_.handle);
    }

    COLORREF background = RGB(255, 255, 255);
    COLORREF text = RGB(0, 0, 0);
    COLORREF text_background = background;

    if (Win32Api::document_interface() == Win32Api::kDocumentSdi) {
        if (Win32Api::style() == Win32Api::kStyleAero) {
            background = RGB(0, 0, 0);
            text = RGB(255, 255, 255);
            text_background = TRANSPARENT;
        } else if (Win32Api::style() == Win32Api::kStyleMetro && Win32Api::DwmGetColorizationColor()) {
            DWORD color = 0;
            BOOL opaque = FALSE;
            if (SUCCEEDED(Win32Api::DwmGetColorizationColor()(&color, &opaque))) {
                text = RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
            }
        }
    }

    HBRUSH brush = CreateSolidBrush(background);
    FillRect(hdc, &paint.rcPaint, brush);
    DeleteObject(brush);

    UpdateCaretPixels(hdc);

    SetTextColor(hdc, text);
    SetBkColor(hdc, text_background);

    RECT text_rect = paint.rcPaint;
    for (size_t i = 0; i < file_.text.line_count(); ++i) {
        WideString line_text(file_.text.line_string(i));
        if (!ime_text_.empty() && static_cast<int>(i) == caret_.y) {
            WideString composed;
            const WideString::LineView line = file_.text.line_view(i);
            for (int j = 0; j < caret_.x && j < static_cast<int>(line.length); ++j) {
                composed.AppendChar(line.text[j]);
            }
            composed.Append(ime_text_);
            for (size_t j = static_cast<size_t>(caret_.x); j < line.length; ++j) {
                composed.AppendChar(line.text[j]);
            }
            line_text = composed;
        }

        const TStringBuffer draw_text = line_text.ToTString();
        DrawText(hdc, draw_text.c_str(), -1, &text_rect, DT_EXPANDTABS);
        text_rect.top += font_.size.y;
    }

    if (focus_) {
        SetCaretPos(caret_pixels_.x, caret_pixels_.y);
    }

    EndPaint(hwnd_, &paint);
}

void TextBox::CreateCaretHandle() {
    CreateCaret(hwnd_, NULL, 2, font_.size.y);
    ShowCaret(hwnd_);
}

void TextBox::UpdateTitle() {
    const TStringBuffer title = file_.name.ToTString();
    SetWindowText(hwnd_, title.c_str());
}

void TextBox::UpdateCaretPixels(HDC hdc) {
    size_t prefix_bytes = sizeof(wchar_t) * static_cast<size_t>(caret_.x);
    size_t ime_bytes = sizeof(wchar_t) * ime_text_.length();
    const WideString::LineView line = file_.text.line_view(caret_.y);

    wchar_t *caret_line = reinterpret_cast<wchar_t *>(TextEditor::memory::AllocateBytes(prefix_bytes + ime_bytes + sizeof(wchar_t)));
    memcpy(caret_line, line.text, prefix_bytes);

    size_t end = static_cast<size_t>(caret_.x);
    if (!ime_text_.empty()) {
        memcpy(caret_line + caret_.x, ime_text_.c_str(), ime_bytes);
        end += ime_text_.length();
    }
    caret_line[end] = L'\0';

    const TStringBuffer line_buffer(WideString(caret_line).ToTString());
    RECT text_size = { 0 };
    DrawText(hdc, line_buffer.c_str(), -1, &text_size, DT_CALCRECT | DT_EXPANDTABS);

    caret_pixels_.x = text_size.right;
    caret_pixels_.y = font_.size.y * caret_.y;

    TextEditor::memory::Free(caret_line);
}

WideString TextBox::EncodingLabel() const {
    WideString encoding(Strings::Encoding());
    encoding.AppendWide(L": ");
    encoding.Append(WideString::FromTString(EncodingName(file_.encoding)));
    return encoding;
}

void TextBox::ResetMultibyteBuffer() {
#ifndef UNICODE
    memset(multibyte_buffer_, 0, sizeof(multibyte_buffer_));
#endif
}

}  // namespace TextEditor
