#include <leak_checker.h>

#include <string_utilities.h>

#include <memory_helper.h>

#include <stdint.h>
#include <string.h>
#include <wchar.h>

namespace TextEditor {

namespace {

void ClampSlice(size_t total_length, size_t *start, size_t *slice_length) {
    if (*start > total_length) {
        *start = total_length;
    }

    const size_t available = total_length - *start;
    if (*slice_length > available) {
        *slice_length = available;
    }
}

bool IsUtf8Follower(BYTE byte) {
    return (byte & 0xC0) == 0x80;
}

bool DecodeUtf8Character(const BYTE *bytes, size_t size, size_t index, uint32_t *character, size_t *consumed) {
    if (index >= size) {
        *character = 0;
        *consumed = 0;
        return false;
    }

    const BYTE lead = bytes[index];
    if ((lead & 0x80) == 0x00) {
        *character = lead;
        *consumed = 1;
        return true;
    }

    if (lead >= 0xC2 &&
        lead <= 0xDF &&
        (index + 1) < size &&
        IsUtf8Follower(bytes[index + 1])) {
        *character = static_cast<uint32_t>(lead & 0x1F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 1] & 0x3F);
        *consumed = 2;
        return true;
    }

    if (lead == 0xE0 &&
        (index + 2) < size &&
        bytes[index + 1] >= 0xA0 &&
        bytes[index + 1] <= 0xBF &&
        IsUtf8Follower(bytes[index + 2])) {
        *character = static_cast<uint32_t>(lead & 0x0F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 1] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 2] & 0x3F);
        *consumed = 3;
        return true;
    }

    if (((lead >= 0xE1 && lead <= 0xEC) || (lead >= 0xEE && lead <= 0xEF)) &&
        (index + 2) < size &&
        IsUtf8Follower(bytes[index + 1]) &&
        IsUtf8Follower(bytes[index + 2])) {
        *character = static_cast<uint32_t>(lead & 0x0F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 1] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 2] & 0x3F);
        *consumed = 3;
        return true;
    }

    if (lead == 0xED &&
        (index + 2) < size &&
        bytes[index + 1] >= 0x80 &&
        bytes[index + 1] <= 0x9F &&
        IsUtf8Follower(bytes[index + 2])) {
        *character = static_cast<uint32_t>(lead & 0x0F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 1] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 2] & 0x3F);
        *consumed = 3;
        return true;
    }

    if (lead == 0xF0 &&
        (index + 3) < size &&
        bytes[index + 1] >= 0x90 &&
        bytes[index + 1] <= 0xBF &&
        IsUtf8Follower(bytes[index + 2]) &&
        IsUtf8Follower(bytes[index + 3])) {
        *character = static_cast<uint32_t>(lead & 0x07);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 1] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 2] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 3] & 0x3F);
        *consumed = 4;
        return true;
    }

    if (lead >= 0xF1 &&
        lead <= 0xF3 &&
        (index + 3) < size &&
        IsUtf8Follower(bytes[index + 1]) &&
        IsUtf8Follower(bytes[index + 2]) &&
        IsUtf8Follower(bytes[index + 3])) {
        *character = static_cast<uint32_t>(lead & 0x07);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 1] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 2] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 3] & 0x3F);
        *consumed = 4;
        return true;
    }

    if (lead == 0xF4 &&
        (index + 3) < size &&
        bytes[index + 1] >= 0x80 &&
        bytes[index + 1] <= 0x8F &&
        IsUtf8Follower(bytes[index + 2]) &&
        IsUtf8Follower(bytes[index + 3])) {
        *character = static_cast<uint32_t>(lead & 0x07);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 1] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 2] & 0x3F);
        *character = (*character << 6) | static_cast<uint32_t>(bytes[index + 3] & 0x3F);
        *consumed = 4;
        return true;
    }

    *character = 0;
    *consumed = 1;
    return false;
}

void AppendCodePoint(WideString *wide_string, uint32_t character) {
    if (character <= 0xFFFF) {
        wide_string->AppendChar(static_cast<wchar_t>(character));
        return;
    }

    character -= 0x10000;
    wide_string->AppendChar(static_cast<wchar_t>(0xD800 + (character >> 10)));
    wide_string->AppendChar(static_cast<wchar_t>(0xDC00 + (character & 0x3FF)));
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
    : value_(NULL),
      length_(0),
      capacity_(0) {
    EnsureCapacity(1);
    value_[0] = L'\0';
    UpdateLines();
}

WideString::WideString(const char *ansi_string)
    : value_(NULL),
      length_(0),
      capacity_(0) {
    AssignWide(FromAnsi(ansi_string ? ansi_string : ""));
}

WideString::WideString(const wchar_t *wide_string)
    : value_(NULL),
      length_(0),
      capacity_(0) {
    AssignWide(CloneWide(wide_string ? wide_string : L""));
}

WideString::WideString(const WideString &other)
    : value_(NULL),
      length_(0),
      capacity_(0) {
    AssignWide(CloneWide(other.c_str(), other.length()));
}

WideString::~WideString() {
    ClearLines();
    delete[] value_;
}

WideString &WideString::operator=(const WideString &other) {
    if (this != &other) {
        AssignWide(CloneWide(other.c_str(), other.length()));
    }

    return *this;
}

const wchar_t *WideString::c_str() const {
    return value_ ? value_ : L"";
}

size_t WideString::length() const {
    return length_;
}

bool WideString::empty() const {
    return length_ == 0;
}

TStringBuffer WideString::ToTString() const {
#ifdef UNICODE
    wchar_t *clone = CloneWide(c_str(), length_);
    return TStringBuffer(clone);
#else
    return TStringBuffer(ToAnsi(c_str()));
#endif
}

void WideString::AppendChar(wchar_t character) {
    InsertChar(length_, character);
}

void WideString::InsertChar(size_t position, wchar_t character) {
    wchar_t text[2] = { character, L'\0' };
    InsertWide(position, text);
}

void WideString::AppendAnsi(const char *text) {
    InsertAnsi(length_, text);
}

void WideString::InsertAnsi(size_t position, const char *text) {
    wchar_t *wide = FromAnsi(text ? text : "");
    InsertWide(position, wide);
    delete[] wide;
}

void WideString::AppendWide(const wchar_t *text) {
    InsertWide(length_, text);
}

void WideString::InsertWide(size_t position, const wchar_t *text) {
    if (!text || !text[0]) {
        return;
    }

    const size_t additional_length = wcslen(text);
    if (position > length_) {
        position = length_;
    }

    EnsureCapacity(length_ + additional_length + 1);
    for (size_t i = length_ + 1; i > position; --i) {
        value_[i + additional_length - 1] = value_[i - 1];
    }

    memcpy(value_ + position, text, additional_length * sizeof(wchar_t));
    length_ += additional_length;
    value_[length_] = L'\0';
    UpdateLines();
}

void WideString::Append(const WideString &text) {
    if (this == &text) {
        WideString copy(text);
        AppendWide(copy.c_str());
        return;
    }

    AppendWide(text.c_str());
}

void WideString::RemoveLastCharacter() {
    if (!length_) {
        return;
    }

    RemoveCharacterAt(length_ - 1);
}

void WideString::RemoveCharacterAt(size_t position) {
    if (position >= length_) {
        return;
    }

    for (size_t i = position; i < length_; ++i) {
        value_[i] = value_[i + 1];
    }

    --length_;
    UpdateLines();
}

void WideString::InsertCharAtLine(size_t line_index, size_t position, wchar_t character) {
    if (line_index >= lines_.size()) {
        return;
    }

    if (position > line_length(line_index)) {
        position = line_length(line_index);
    }

    InsertChar(line_start(line_index) + position, character);
}

void WideString::InsertAnsiAtLine(size_t line_index, size_t position, const char *text) {
    if (line_index >= lines_.size()) {
        return;
    }

    if (position > line_length(line_index)) {
        position = line_length(line_index);
    }

    InsertAnsi(line_start(line_index) + position, text);
}

void WideString::InsertWideAtLine(size_t line_index, size_t position, const wchar_t *text) {
    if (line_index >= lines_.size()) {
        return;
    }

    if (position > line_length(line_index)) {
        position = line_length(line_index);
    }

    InsertWide(line_start(line_index) + position, text);
}

void WideString::RemoveCharacterAtLine(size_t line_index, size_t position) {
    if (line_index >= lines_.size() || position >= line_length(line_index)) {
        return;
    }

    RemoveCharacterAt(line_start(line_index) + position);
}

size_t WideString::line_count() const {
    return lines_.size();
}

size_t WideString::line_length(size_t index) const {
    if (index >= lines_.size()) {
        return 0;
    }

    size_t start = lines_[index].start;
    size_t slice_length = lines_[index].length;
    ClampSlice(length_, &start, &slice_length);
    return slice_length;
}

size_t WideString::line_start(size_t index) const {
    if (index >= lines_.size()) {
        return length_;
    }

    size_t start = lines_[index].start;
    size_t slice_length = lines_[index].length;
    ClampSlice(length_, &start, &slice_length);
    return start;
}

WideString::LineView WideString::line_view(size_t index) const {
    LineView view;
    if (index >= lines_.size()) {
        view.text = c_str() + length_;
        view.length = 0;
        return view;
    }

    size_t start = lines_[index].start;
    size_t slice_length = lines_[index].length;
    ClampSlice(length_, &start, &slice_length);

    view.text = c_str() + start;
    view.length = slice_length;
    return view;
}

WideString WideString::line_string(size_t index) const {
    const LineView view = line_view(index);
    WideString line;
    line.AssignWide(CloneWide(view.text, view.length));
    return line;
}

void WideString::ExtractFileNameFromPath() {
    size_t last_separator = 0;
    for (size_t i = 0; i < length_; ++i) {
        if (value_[i] == L'\\' || value_[i] == L'/') {
            last_separator = i + 1;
        }
    }

    if (!last_separator) {
        return;
    }

    const size_t file_name_length = length_ - last_separator;
    memmove(value_, value_ + last_separator, (file_name_length + 1) * sizeof(wchar_t));
    length_ = file_name_length;
    UpdateLines();
}

WideString WideString::FromUtf8(const char *utf8_string) {
    WideString wide_string;
    if (!utf8_string) {
        return wide_string;
    }

    const BYTE *bytes = reinterpret_cast<const BYTE *>(utf8_string);
    const size_t size = strlen(utf8_string);
    size_t index = 0;

    while (index < size && bytes[index]) {
        uint32_t character = 0;
        size_t consumed = 0;
        if (!DecodeUtf8Character(bytes, size, index, &character, &consumed)) {
            character = '?';
        }

        AppendCodePoint(&wide_string, character);
        index += consumed;
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

    if (!wide_string) {
        value_ = NULL;
        length_ = 0;
        capacity_ = 0;
        EnsureCapacity(1);
        value_[0] = L'\0';
    } else {
        value_ = wide_string;
        length_ = wcslen(wide_string);
        capacity_ = length_ + 1;
    }

    UpdateLines();
}

void WideString::EnsureCapacity(size_t requested_capacity) {
    if (requested_capacity <= capacity_) {
        return;
    }

    size_t new_capacity = capacity_ ? capacity_ * 2 : 8;
    while (new_capacity < requested_capacity) {
        new_capacity *= 2;
    }

    wchar_t *buffer = new wchar_t[new_capacity];
    if (value_) {
        memcpy(buffer, value_, (length_ + 1) * sizeof(wchar_t));
    } else {
        buffer[0] = L'\0';
    }

    delete[] value_;
    value_ = buffer;
    capacity_ = new_capacity;
}

void WideString::UpdateLines() {
    ClearLines();

    size_t line_start_index = 0;
    for (size_t i = 0; i <= length_; ++i) {
        if (i == length_ || value_[i] == L'\n') {
            LineInfo line;
            line.start = line_start_index;
            line.length = i - line_start_index;
            lines_.push_back(line);
            line_start_index = i + 1;
        }
    }
}

void WideString::ClearLines() {
    lines_.clear();
}

wchar_t *WideString::CloneWide(const wchar_t *text) {
    if (!text) {
        return CloneWide(L"", 0);
    }

    return CloneWide(text, wcslen(text));
}

wchar_t *WideString::CloneWide(const wchar_t *text, size_t characters) {
    if (!text) {
        characters = 0;
    }

    wchar_t *clone = new wchar_t[characters + 1];
    if (characters && text) {
        memcpy(clone, text, characters * sizeof(wchar_t));
    }
    clone[characters] = L'\0';
    return clone;
}

wchar_t *WideString::FromAnsi(const char *ansi_string) {
    const char *text = ansi_string ? ansi_string : "";
    const int characters = MultiByteToWideChar(CP_ACP, 0, text, -1, NULL, 0);
    wchar_t *wide = new wchar_t[characters];
    MultiByteToWideChar(CP_ACP, 0, text, -1, wide, characters);
    return wide;
}

char *WideString::ToAnsi(const wchar_t *wide_string) {
    const wchar_t *text = wide_string ? wide_string : L"";
    const int characters = WideCharToMultiByte(CP_ACP, 0, text, -1, NULL, 0, NULL, NULL);
    char *ansi = new char[characters];
    WideCharToMultiByte(CP_ACP, 0, text, -1, ansi, characters, NULL, NULL);
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
        uint32_t character = 0;
        size_t consumed = 0;
        if (!DecodeUtf8Character(bytes, size, index, &character, &consumed)) {
            return kTextEncodingAnsi;
        }

        UNREFERENCED_PARAMETER(character);
        index += consumed;
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
