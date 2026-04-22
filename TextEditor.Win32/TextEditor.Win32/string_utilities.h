#ifndef TEXTEDITOR_STRING_UTILITIES_H
#define TEXTEDITOR_STRING_UTILITIES_H

#include <Windows.h>

#include <vector.h>

namespace TextEditor {

enum TextEncoding {
    kTextEncodingNone = 0,
    kTextEncodingAnsi = 1,
    kTextEncodingWide = 2,
    kTextEncodingUtf8 = 3,
    kTextEncodingUtf8WithBom = 4
};

class TStringBuffer {
public:
    TStringBuffer();
    explicit TStringBuffer(TCHAR *value);
    TStringBuffer(const TStringBuffer &other);
    ~TStringBuffer();

    TStringBuffer &operator=(const TStringBuffer &other);

    const TCHAR *c_str() const;
    operator LPCTSTR() const;

private:
    void Assign(TCHAR *value);
    static TCHAR *Clone(LPCTSTR value);

    TCHAR *value_;
};

class WideString {
public:
    struct LineView {
        const wchar_t *text;
        size_t length;
    };

    WideString();
    explicit WideString(const char *ansi_string);
    explicit WideString(const wchar_t *wide_string);
    WideString(const WideString &other);
    ~WideString();

    WideString &operator=(const WideString &other);

    const wchar_t *c_str() const;
    size_t length() const;
    bool empty() const;

    TStringBuffer ToTString() const;

    void AppendChar(wchar_t character);
    void InsertChar(size_t position, wchar_t character);
    void AppendAnsi(const char *text);
    void InsertAnsi(size_t position, const char *text);
    void AppendWide(const wchar_t *text);
    void InsertWide(size_t position, const wchar_t *text);
    void Append(const WideString &text);
    void RemoveLastCharacter();
    void RemoveCharacterAt(size_t position);

    void InsertCharAtLine(size_t line, size_t position, wchar_t character);
    void InsertAnsiAtLine(size_t line, size_t position, const char *text);
    void InsertWideAtLine(size_t line, size_t position, const wchar_t *text);
    void RemoveCharacterAtLine(size_t line, size_t position);

    size_t line_count() const;
    size_t line_length(size_t index) const;
    size_t line_start(size_t index) const;
    LineView line_view(size_t index) const;
    WideString line_string(size_t index) const;
    void ExtractFileNameFromPath();

    static WideString FromTString(LPCTSTR t_string);
    static WideString FromUtf8(const char *utf8_string);

private:
    struct LineInfo {
        size_t start;
        size_t length;
    };

    void AssignWide(wchar_t *wide_string);
    void EnsureCapacity(size_t requested_capacity);
    void UpdateLines();
    void ClearLines();
    static wchar_t *CloneWide(const wchar_t *text);
    static wchar_t *CloneWide(const wchar_t *text, size_t characters);
    static wchar_t *FromAnsi(const char *ansi_string);
    static char *ToAnsi(const wchar_t *wide_string);

    wchar_t *value_;
    size_t length_;
    size_t capacity_;
    Vector<LineInfo> lines_;
};

TextEncoding DetectEncoding(const BYTE *bytes, size_t size);
LPCTSTR EncodingName(TextEncoding encoding);
size_t CountCharacters(LPCTSTR text);

}  // namespace TextEditor

#endif
