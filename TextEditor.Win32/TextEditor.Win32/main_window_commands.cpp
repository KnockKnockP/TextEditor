#include <leak_checker.h>

#include <main_window.h>

#include <main_window_document_host.h>
#include <memory_helper.h>
#include <windows_helper.h>

namespace TextEditor {

void MainWindow::OpenFile() {
    WideString file_name;
    HANDLE file = Win32Api::ShowFileDialog(hwnd_, true, &file_name);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    const DWORD file_size = GetFileSize(file, NULL);
    BYTE *bytes = reinterpret_cast<BYTE *>(TextEditor::memory::AllocateBytes(file_size + sizeof(WCHAR)));
    DWORD bytes_read = 0;

    TextFile text_file;
    text_file.name = file_name;

    if (!ReadFile(file, bytes, file_size, &bytes_read, NULL)) {
        TextEditor::memory::Free(bytes);
        CloseHandle(file);
        Win32Api::Error(TEXT("Failed to read file."));
        return;
    }

    bytes[bytes_read] = '\0';
    text_file.encoding = DetectEncoding(bytes, bytes_read);
    if (text_file.encoding == kTextEncodingAnsi) {
        text_file.text = WideString(reinterpret_cast<const char *>(bytes));
    } else if (text_file.encoding == kTextEncodingWide) {
        bytes[bytes_read + 1] = '\0';
        text_file.text = WideString(reinterpret_cast<const wchar_t *>(bytes + 2));
    } else {
        const char *utf8_text = reinterpret_cast<const char *>(bytes + (text_file.encoding == kTextEncodingUtf8WithBom ? 3 : 0));
        text_file.text = WideString::FromUtf8(utf8_text);
    }

    TextBox *text_box = document_host_ ? document_host_->CreateTextBoxForOpen() : NULL;
    if (text_box) {
        text_box->SetFile(text_file);
    }

    TextEditor::memory::Free(bytes);
    CloseHandle(file);
}

void MainWindow::SaveFile() {
    TextBox *text_box = CurrentTextBox();
    if (!text_box) {
        return;
    }

    WideString file_name;
    HANDLE file = Win32Api::ShowFileDialog(hwnd_, false, &file_name);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    const BYTE bom[2] = { 0xFF, 0xFE };
    DWORD bytes_written = 0;
    const wchar_t *text = text_box->file().text.c_str();

    if (!WriteFile(file, bom, 2, &bytes_written, NULL) ||
        !WriteFile(file, text, static_cast<DWORD>(wcslen(text) * sizeof(WCHAR)), &bytes_written, NULL)) {
        Win32Api::Warning(TEXT("Failed to save file."));
    }

    CloseHandle(file);
}

void MainWindow::Exit() {
    SendMessage(hwnd_, WM_CLOSE, 0, 0);
}

}  // namespace TextEditor
