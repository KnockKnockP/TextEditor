#include <leak_checker.h>

#include <string_utilities.h>

#include <memory_helper.h>

#include <stdint.h>
#include <string.h>
#include <wchar.h>

namespace TextEditor {

namespace {

bool IsUtf8Boundary(size_t size, size_t index, size_t cluster_size) {
    return size >= cluster_size && (index + cluster_size) <= size;
}

bool IsUtf8Follower(const BYTE *bytes, size_t index) {
    return (bytes[index] & 0xC0) == 0x80;
}

bool IsUtf8Cluster1(BYTE byte) {
    return (byte & 0x80) == 0x00;
}

bool IsUtf8Cluster2(const BYTE *bytes, size_t size, size_t index) {
    return IsUtf8Boundary(size, index, 2) &&
           ((bytes[index] & 0xE0) == 0xC0) &&
           IsUtf8Follower(bytes, index + 1);
}

bool IsUtf8Cluster3(const BYTE *bytes, size_t size, size_t index) {
    return IsUtf8Boundary(size, index, 3) &&
           ((bytes[index] & 0xF0) == 0xE0) &&
           IsUtf8Follower(bytes, index + 1) &&
           IsUtf8Follower(bytes, index + 2);
}

bool IsUtf8Cluster4(const BYTE *bytes, size_t size, size_t index) {
    return IsUtf8Boundary(size, index, 4) &&
           ((bytes[index] & 0xF8) == 0xF0) &&
           IsUtf8Follower(bytes, index + 1) &&
           IsUtf8Follower(bytes, index + 2) &&
           IsUtf8Follower(bytes, index + 3);
}

}  // namespace

TStringBuffer::TStringBuffer()
    : value_(NULL) {
}

TStringBuffer::TStringBuffer(TCHAR *value)
    : value_(NULL) {
    Assign(value);
}

TStringBuffer::TStringBuffer(const TStringBuffer &other)
    : value_(NULL) {
    Assign(Clone(other.c_str()));
}

TStringBuffer::~TStringBuffer() {
    delete[] value_;
}

TStringBuffer &TStringBuffer::operator=(const TStringBuffer &other) {
    if (this != &other) {
        Assign(Clone(other.c_str()));
    }

    return *this;
}

const TCHAR *TStringBuffer::c_str() const {
    return value_ ? value_ : TEXT("");
}

TStringBuffer::operator LPCTSTR() const {
    return c_str();
}

void TStringBuffer::Assign(TCHAR *value) {
    delete[] value_;
    value_ = value;
}

TCHAR *TStringBuffer::Clone(LPCTSTR value) {
    const size_t characters = CountCharacters(value) + 1;
    TCHAR *clone = new TCHAR[characters];

#ifdef UNICODE
    wcscpy(clone, value);
#else
    strcpy(clone, value);
#endif

    return clone;
}

WideString::WideString()
    : value_(new wchar_t[1]) {
    value_[0] = L'\0';
    UpdateLines();
}

WideString::WideString(const char *ansi_string)
    : value_(NULL) {
    AssignWide(FromAnsi(ansi_string));
}

WideString::WideString(const wchar_t *wide_string)
    : value_(NULL) {
    AssignWide(CloneWide(wide_string));
}

WideString::WideString(const WideString &other)
    : value_(NULL) {
    AssignWide(CloneWide(other.c_str()));
}

WideString::~WideString() {
    ClearLines();
    delete[] value_;
}

WideString &WideString::operator=(const WideString &other) {
    if (this != &other) {
        AssignWide(CloneWide(other.c_str()));
    }

    return *this;
}

const wchar_t *WideString::c_str() const {
    return value_ ? value_ : L"";
}

size_t WideString::length() const {
    return wcslen(c_str());
}

bool WideString::empty() const {
    return length() == 0;
}

TStringBuffer WideString::ToTString() const {
#ifdef UNICODE
    const size_t characters = length() + 1;
    wchar_t *clone = new wchar_t[characters];
    wcscpy(clone, c_str());
    return TStringBuffer(clone);
#else
    return TStringBuffer(ToAnsi(c_str()));
#endif
}

void WideString::AppendChar(wchar_t character) {
    InsertChar(length(), character);
}

void WideString::InsertChar(size_t position, wchar_t character) {
    const size_t old_length = length();
    wchar_t *text = new wchar_t[old_length + 2];

    if (position > old_length) {
        position = old_length;
    }

    for (size_t i = 0; i < position; ++i) {
        text[i] = value_[i];
    }

    text[position] = character;

    for (size_t i = position; i < old_length; ++i) {
        text[i + 1] = value_[i];
    }

    text[old_length + 1] = L'\0';
    AssignWide(text);
}

void WideString::AppendAnsi(const char *text) {
    InsertAnsi(length(), text);
}

void WideString::InsertAnsi(size_t position, const char *text) {
    wchar_t *wide = FromAnsi(text);
    InsertWide(position, wide);
    delete[] wide;
}

void WideString::AppendWide(const wchar_t *text) {
    InsertWide(length(), text);
}

void WideString::InsertWide(size_t position, const wchar_t *text) {
    const size_t current_length = length();
    const size_t additional_length = wcslen(text);
    wchar_t *combined = new wchar_t[current_length + additional_length + 1];

    if (position > current_length) {
        position = current_length;
    }

    size_t write_index = 0;
    for (; write_index < position; ++write_index) {
        combined[write_index] = value_[write_index];
    }

    for (size_t i = 0; i < additional_length; ++i, ++write_index) {
        combined[write_index] = text[i];
    }

    for (size_t i = position; i < current_length; ++i, ++write_index) {
        combined[write_index] = value_[i];
    }

    combined[write_index] = L'\0';
    AssignWide(combined);
}

void WideString::Append(const WideString &text) {
    AppendWide(text.c_str());
}

void WideString::RemoveLastCharacter() {
    if (!length()) {
        return;
    }

    RemoveCharacterAt(length() - 1);
}

void WideString::RemoveCharacterAt(size_t position) {
    const size_t characters = length();
    if (!characters || position >= characters) {
        return;
    }

    wchar_t *text = new wchar_t[characters];
    size_t write_index = 0;
    for (size_t i = 0; i < characters; ++i) {
        if (i != position) {
            text[write_index++] = value_[i];
        }
    }

    text[write_index] = L'\0';
    AssignWide(text);
}

void WideString::InsertCharAtLine(size_t line_index, size_t position, wchar_t character) {
    WideString current_line(line(line_index));
    current_line.InsertChar(position, character);

    delete[] lines_[line_index];
    lines_[line_index] = CloneWide(current_line.c_str());
    ConsolidateLines();
}

void WideString::InsertAnsiAtLine(size_t line_index, size_t position, const char *text) {
    WideString current_line(line(line_index));
    current_line.InsertAnsi(position, text);

    delete[] lines_[line_index];
    lines_[line_index] = CloneWide(current_line.c_str());
    ConsolidateLines();
}

void WideString::InsertWideAtLine(size_t line_index, size_t position, const wchar_t *text) {
    WideString current_line(line(line_index));
    current_line.InsertWide(position, text);

    delete[] lines_[line_index];
    lines_[line_index] = CloneWide(current_line.c_str());
    ConsolidateLines();
}

void WideString::RemoveCharacterAtLine(size_t line_index, size_t position) {
    WideString current_line(line(line_index));
    current_line.RemoveCharacterAt(position);

    delete[] lines_[line_index];
    lines_[line_index] = CloneWide(current_line.c_str());
    ConsolidateLines();
}

size_t WideString::line_count() const {
    return lines_.size();
}

const wchar_t *WideString::line(size_t index) const {
    return lines_[index];
}

void WideString::ConsolidateLines() {
    size_t total_characters = 1;
    for (size_t i = 0; i < lines_.size(); ++i) {
        total_characters += wcslen(lines_[i]);
        if (i + 1 != lines_.size()) {
            ++total_characters;
        }
    }

    wchar_t *text = new wchar_t[total_characters];
    text[0] = L'\0';

    for (size_t i = 0; i < lines_.size(); ++i) {
        wcscat(text, lines_[i]);
        if (i + 1 != lines_.size()) {
            wcscat(text, L"\n");
        }
    }

    AssignWide(text);
}

void WideString::ExtractFileNameFromPath() {
    const size_t characters = length();
    size_t last_separator = 0;

    for (size_t i = 0; i < characters; ++i) {
        if (value_[i] == L'\\') {
            last_separator = i + 1;
        }
    }

    const size_t file_name_characters = characters - last_separator;
    wchar_t *file_name = new wchar_t[file_name_characters + 1];
    for (size_t i = 0; i < file_name_characters; ++i) {
        file_name[i] = value_[last_separator + i];
    }
    file_name[file_name_characters] = L'\0';

    AssignWide(file_name);
}

WideString WideString::FromUtf8(const char *utf8_string) {
    WideString wide_string;
    const size_t size = strlen(utf8_string);
    int index = 0;

    while (utf8_string[index]) {
        uint32_t character = 0;

        if (IsUtf8Cluster1(static_cast<BYTE>(utf8_string[index]))) {
            character = static_cast<BYTE>(utf8_string[index++]);
        } else if (IsUtf8Cluster2(reinterpret_cast<const BYTE *>(utf8_string), size, index)) {
            character = static_cast<BYTE>(utf8_string[index++]) & 0x1F;
            character = (character << 6) | (static_cast<BYTE>(utf8_string[index++]) & 0x3F);
        } else if (IsUtf8Cluster3(reinterpret_cast<const BYTE *>(utf8_string), size, index)) {
            character = static_cast<BYTE>(utf8_string[index++]) & 0x0F;
            character = (character << 6) | (static_cast<BYTE>(utf8_string[index++]) & 0x3F);
            character = (character << 6) | (static_cast<BYTE>(utf8_string[index++]) & 0x3F);
        } else if (IsUtf8Cluster4(reinterpret_cast<const BYTE *>(utf8_string), size, index)) {
            character = static_cast<BYTE>(utf8_string[index++]) & 0x07;
            character = (character << 6) | (static_cast<BYTE>(utf8_string[index++]) & 0x3F);
            character = (character << 6) | (static_cast<BYTE>(utf8_string[index++]) & 0x3F);
            character = (character << 6) | (static_cast<BYTE>(utf8_string[index++]) & 0x3F);
        }

        wchar_t converted[3] = { 0, 0, 0 };
        converted[0] = static_cast<wchar_t>(character);

        if (character >= 0x10000) {
            character -= 0x10000;
            converted[0] = static_cast<wchar_t>(0xD800 + (character >> 10));
            converted[1] = static_cast<wchar_t>(0xDC00 + (character & 0x3FF));
        }

        wide_string.AppendWide(converted);
    }

    return wide_string;
}

WideString WideString::FromTString(LPCTSTR t_string) {
#ifdef UNICODE
    return WideString(t_string);
#else
    return WideString(static_cast<const char *>(t_string));
#endif
}

void WideString::AssignWide(wchar_t *wide_string) {
    ClearLines();
    delete[] value_;
    value_ = wide_string;
    UpdateLines();
}

void WideString::UpdateLines() {
    ClearLines();

    const size_t characters = length();
    size_t line_start = 0;

    for (size_t i = 0; i <= characters; ++i) {
        if (value_[i] == L'\n' || value_[i] == L'\0') {
            const size_t line_length = i - line_start;
            wchar_t *line = new wchar_t[line_length + 1];

            for (size_t j = 0; j < line_length; ++j) {
                line[j] = value_[line_start + j];
            }

            line[line_length] = L'\0';
            lines_.push_back(line);
            line_start = i + 1;
        }
    }

    if (lines_.empty()) {
        wchar_t *empty_line = new wchar_t[1];
        empty_line[0] = L'\0';
        lines_.push_back(empty_line);
    }
}

void WideString::ClearLines() {
    for (size_t i = 0; i < lines_.size(); ++i) {
        delete[] lines_[i];
    }
    lines_.clear();
}

wchar_t *WideString::CloneWide(const wchar_t *text) {
    const size_t characters = wcslen(text) + 1;
    wchar_t *clone = new wchar_t[characters];
    wcscpy(clone, text);
    return clone;
}

wchar_t *WideString::FromAnsi(const char *ansi_string) {
    const int characters = MultiByteToWideChar(CP_ACP, 0, ansi_string, -1, NULL, 0);
    wchar_t *wide = new wchar_t[characters];
    MultiByteToWideChar(CP_ACP, 0, ansi_string, -1, wide, characters);
    return wide;
}

char *WideString::ToAnsi(const wchar_t *wide_string) {
    const int characters = WideCharToMultiByte(CP_ACP, 0, wide_string, -1, NULL, 0, NULL, NULL);
    char *ansi = new char[characters];
    WideCharToMultiByte(CP_ACP, 0, wide_string, -1, ansi, characters, NULL, NULL);
    return ansi;
}

TextEncoding DetectEncoding(const BYTE *bytes, size_t size) {
    if (size >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE) {
        return kTextEncodingWide;
    }

    if (size >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
        return kTextEncodingUtf8WithBom;
    }

    size_t index = 0;
    while (index < size && bytes[index]) {
        if (IsUtf8Cluster1(bytes[index])) {
            ++index;
        } else if (IsUtf8Cluster2(bytes, size, index)) {
            index += 2;
        } else if (IsUtf8Cluster3(bytes, size, index)) {
            index += 3;
        } else if (IsUtf8Cluster4(bytes, size, index)) {
            index += 4;
        } else {
            return kTextEncodingAnsi;
        }
    }

    return kTextEncodingUtf8;
}

LPCTSTR EncodingName(TextEncoding encoding) {
    switch (encoding) {
        case kTextEncodingAnsi:
            return TEXT("ANSI");

        case kTextEncodingWide:
            return TEXT("UTF-16 LE BOM (Wide)");

        case kTextEncodingUtf8:
            return TEXT("UTF-8");

        case kTextEncodingUtf8WithBom:
            return TEXT("UTF-8 BOM");

        default:
            return TEXT("Unknown Encoding");
    }
}

size_t CountCharacters(LPCTSTR text) {
#ifdef UNICODE
    return wcslen(text);
#else
    return strlen(text);
#endif
}

}  // namespace TextEditor
