// stub_string.cpp — GDExtension String and StringName interface stubs.
//
// String blob (8 bytes): stores a heap-allocated std::u32string*.
//   Construction: allocate new std::u32string, store pointer.
//   Destruction:  delete the std::u32string, store nullptr.
//   Copy:         clone the std::u32string.
//
// StringName blob (8 bytes): stores a pointer into a global interned pool
//   (std::unordered_set<std::string>).  All StringNames with the same
//   bytes share the same pool node, so equality is pointer equality.

#include "stub_types.h"

#include <cstring>
#include <unordered_set>

// ─── Internal helpers ─────────────────────────────────────────────────────

// Simple UTF-8 decode to char32_t.  Handles ASCII fast-path; multi-byte sequences
// are decoded per RFC 3629.  Invalid bytes are replaced with U+FFFD.
static char32_t utf8_next(const char **pp, const char *end) {
    const unsigned char *p = reinterpret_cast<const unsigned char *>(*pp);
    if (p >= reinterpret_cast<const unsigned char *>(end)) return 0;
    uint8_t b = *p++;
    char32_t cp;
    int extra = 0;
    if (b < 0x80)        { cp = b;           extra = 0; }
    else if (b < 0xC0)   { cp = 0xFFFD;      extra = 0; }
    else if (b < 0xE0)   { cp = b & 0x1F;    extra = 1; }
    else if (b < 0xF0)   { cp = b & 0x0F;    extra = 2; }
    else                 { cp = b & 0x07;     extra = 3; }
    while (extra-- > 0) {
        if (p >= reinterpret_cast<const unsigned char *>(end)) { cp = 0xFFFD; break; }
        uint8_t c = *p++;
        if ((c & 0xC0) != 0x80) { cp = 0xFFFD; break; }
        cp = (cp << 6) | (c & 0x3F);
    }
    *pp = reinterpret_cast<const char *>(p);
    return cp;
}

std::u32string *stub_str_from_utf8(const char *p, ptrdiff_t len) {
    auto *s = new std::u32string();
    if (!p) return s;
    const char *end = (len < 0) ? p + ::strlen(p) : p + len;
    while (p < end) {
        char32_t cp = utf8_next(&p, end);
        if (cp == 0) break;
        s->push_back(cp);
    }
    return s;
}

std::u32string *stub_str_from_latin1(const char *p, ptrdiff_t len) {
    auto *s = new std::u32string();
    if (!p) return s;
    ptrdiff_t n = (len < 0) ? (ptrdiff_t)::strlen(p) : len;
    s->reserve((size_t)n);
    for (ptrdiff_t i = 0; i < n; ++i) {
        s->push_back((unsigned char)p[i]);
    }
    return s;
}

ptrdiff_t stub_str_to_utf8(const std::u32string *s, char *out, ptrdiff_t out_len) {
    if (!s) { if (out && out_len > 0) out[0] = '\0'; return 0; }
    ptrdiff_t needed = 0;
    for (char32_t cp : *s) {
        int bytes;
        if      (cp < 0x80)    bytes = 1;
        else if (cp < 0x800)   bytes = 2;
        else if (cp < 0x10000) bytes = 3;
        else                   bytes = 4;
        // godot-cpp's String::utf8() passes out_len == content byte length and
        // writes its own NUL at str[length]; write every byte that fits in the
        // buffer (<=, not <) so the final content byte is never dropped.
        if (out && needed + bytes <= out_len) {
            if (bytes == 1) { out[needed] = (char)cp; }
            else if (bytes == 2) {
                out[needed]   = (char)(0xC0 | (cp >> 6));
                out[needed+1] = (char)(0x80 | (cp & 0x3F));
            } else if (bytes == 3) {
                out[needed]   = (char)(0xE0 | (cp >> 12));
                out[needed+1] = (char)(0x80 | ((cp >> 6) & 0x3F));
                out[needed+2] = (char)(0x80 | (cp & 0x3F));
            } else {
                out[needed]   = (char)(0xF0 | (cp >> 18));
                out[needed+1] = (char)(0x80 | ((cp >> 12) & 0x3F));
                out[needed+2] = (char)(0x80 | ((cp >> 6) & 0x3F));
                out[needed+3] = (char)(0x80 | (cp & 0x3F));
            }
        }
        needed += bytes;
    }
    // Only NUL-terminate when a spare slot exists beyond the content.
    if (out && needed < out_len) out[needed] = '\0';
    return needed;
}

ptrdiff_t stub_str_to_latin1(const std::u32string *s, char *out, ptrdiff_t out_len) {
    if (!s) { if (out && out_len > 0) out[0] = '\0'; return 0; }
    ptrdiff_t n = (ptrdiff_t)s->size();
    if (out) {
        // godot-cpp's String::ascii() passes out_len == content length and
        // writes its own NUL at str[length]; only fill a spare slot here so the
        // final content byte is never clobbered.
        ptrdiff_t write = n < out_len ? n : out_len;
        for (ptrdiff_t i = 0; i < write; ++i)
            out[i] = (char)((*s)[i] & 0xFF);
        if (write < out_len) out[write] = '\0';
    }
    return n;
}

// ─── Global interned StringName pool ─────────────────────────────────────

static std::unordered_set<std::string> &sn_pool() {
    static std::unordered_set<std::string> pool;
    return pool;
}

std::string *sn_intern(const char *p, ptrdiff_t len) {
    std::string key = (len < 0) ? std::string(p ? p : "") : std::string(p ? p : "", (size_t)len);
    auto [it, _] = sn_pool().insert(std::move(key));
    return const_cast<std::string *>(&*it);
}

// ─── GDExtension C interface functions ───────────────────────────────────

extern "C" {

// ── String new helpers ────────────────────────────────────────────────────

static void s_string_new_with_latin1_chars(void *r_dest, const char *p_contents) {
    delete str_blob_get(r_dest);
    str_blob_set(r_dest, stub_str_from_latin1(p_contents));
}

static void s_string_new_with_latin1_chars_and_len(void *r_dest, const char *p_contents, int64_t p_size) {
    delete str_blob_get(r_dest);
    str_blob_set(r_dest, stub_str_from_latin1(p_contents, (ptrdiff_t)p_size));
}

static void s_string_new_with_utf8_chars(void *r_dest, const char *p_contents) {
    delete str_blob_get(r_dest);
    str_blob_set(r_dest, stub_str_from_utf8(p_contents));
}

static void s_string_new_with_utf8_chars_and_len(void *r_dest, const char *p_contents, int64_t p_size) {
    delete str_blob_get(r_dest);
    str_blob_set(r_dest, stub_str_from_utf8(p_contents, (ptrdiff_t)p_size));
}

// Returns an error code (0 = OK).  Same semantics as parse_utf8.
static int64_t s_string_new_with_utf8_chars_and_len2(void *r_dest, const char *p_contents, int64_t p_size) {
    delete str_blob_get(r_dest);
    str_blob_set(r_dest, stub_str_from_utf8(p_contents, (ptrdiff_t)p_size));
    return 0; // OK
}

static void s_string_new_with_utf32_chars(void *r_dest, const char32_t *p_contents) {
    delete str_blob_get(r_dest);
    str_blob_set(r_dest, p_contents ? new std::u32string(p_contents) : new std::u32string());
}

static void s_string_new_with_utf32_chars_and_len(void *r_dest, const char32_t *p_contents, int64_t p_size) {
    delete str_blob_get(r_dest);
    str_blob_set(r_dest, p_contents && p_size > 0
                         ? new std::u32string(p_contents, (size_t)p_size)
                         : new std::u32string());
}

static void s_string_new_with_wide_chars(void *r_dest, const wchar_t *p_contents) {
    delete str_blob_get(r_dest);
    auto *s = new std::u32string();
    if (p_contents) {
        for (const wchar_t *p = p_contents; *p; ++p)
            s->push_back((char32_t)*p);
    }
    str_blob_set(r_dest, s);
}

static void s_string_new_with_wide_chars_and_len(void *r_dest, const wchar_t *p_contents, int64_t p_size) {
    delete str_blob_get(r_dest);
    auto *s = new std::u32string();
    if (p_contents && p_size > 0) {
        for (int64_t i = 0; i < p_size; ++i)
            s->push_back((char32_t)p_contents[i]);
    }
    str_blob_set(r_dest, s);
}

// UTF-16: simplistic (BMP only; surrogates are passed through as-is)
static void s_string_new_with_utf16_chars(void *r_dest, const char16_t *p_contents) {
    delete str_blob_get(r_dest);
    auto *s = new std::u32string();
    if (p_contents) {
        for (const char16_t *p = p_contents; *p; ++p)
            s->push_back((char32_t)*p);
    }
    str_blob_set(r_dest, s);
}

static void s_string_new_with_utf16_chars_and_len(void *r_dest, const char16_t *p_contents, int64_t p_size) {
    delete str_blob_get(r_dest);
    auto *s = new std::u32string();
    if (p_contents && p_size > 0) {
        for (int64_t i = 0; i < p_size; ++i)
            s->push_back((char32_t)p_contents[i]);
    }
    str_blob_set(r_dest, s);
}

static int64_t s_string_new_with_utf16_chars_and_len2(void *r_dest, const char16_t *p_contents, int64_t p_size, uint8_t /*p_default_little_endian*/) {
    s_string_new_with_utf16_chars_and_len(r_dest, p_contents, p_size);
    return 0;
}

// ── String operator helpers ───────────────────────────────────────────────

static void s_string_operator_plus_eq_string(void *p_self, const void *p_b) {
    std::u32string *a = str_blob_get(p_self);
    const std::u32string *b = str_blob_get(p_b);
    if (a && b) *a += *b;
}

static void s_string_operator_plus_eq_cstr(void *p_self, const char *p_b) {
    std::u32string *a = str_blob_get(p_self);
    if (a && p_b) {
        for (const char *p = p_b; *p; ++p)
            a->push_back((unsigned char)*p);
    }
}

static void s_string_operator_plus_eq_wcstr(void *p_self, const wchar_t *p_b) {
    std::u32string *a = str_blob_get(p_self);
    if (a && p_b) {
        for (const wchar_t *p = p_b; *p; ++p)
            a->push_back((char32_t)*p);
    }
}

static void s_string_operator_plus_eq_c32str(void *p_self, const char32_t *p_b) {
    std::u32string *a = str_blob_get(p_self);
    if (a && p_b) {
        for (const char32_t *p = p_b; *p; ++p)
            a->push_back(*p);
    }
}

static void s_string_operator_plus_eq_char(void *p_self, int32_t p_b) {
    std::u32string *a = str_blob_get(p_self);
    if (a) a->push_back((char32_t)p_b);
}

static char32_t *s_string_operator_index(void *p_self, int64_t p_index) {
    std::u32string *s = str_blob_get(p_self);
    if (!s || p_index < 0 || (size_t)p_index >= s->size()) return nullptr;
    return reinterpret_cast<char32_t *>(&(*s)[(size_t)p_index]);
}

static const char32_t *s_string_operator_index_const(const void *p_self, int64_t p_index) {
    const std::u32string *s = str_blob_get(p_self);
    if (!s || p_index < 0 || (size_t)p_index >= s->size()) return nullptr;
    return reinterpret_cast<const char32_t *>(&(*s)[(size_t)p_index]);
}

// ── String to / resize ───────────────────────────────────────────────────

static int64_t s_string_to_latin1_chars(const void *p_self, char *r_chars, int64_t p_size) {
    const std::u32string *s = str_blob_get(p_self);
    if (!s) { if (r_chars && p_size > 0) r_chars[0] = '\0'; return 0; }
    return (int64_t)stub_str_to_latin1(s, r_chars, (ptrdiff_t)p_size);
}

static int64_t s_string_to_utf8_chars(const void *p_self, char *r_chars, int64_t p_size) {
    const std::u32string *s = str_blob_get(p_self);
    if (!s) { if (r_chars && p_size > 0) r_chars[0] = '\0'; return 0; }
    return (int64_t)stub_str_to_utf8(s, r_chars, (ptrdiff_t)p_size);
}

static int64_t s_string_to_utf16_chars(const void *p_self, char16_t *r_chars, int64_t p_size) {
    const std::u32string *s = str_blob_get(p_self);
    if (!s) return 0;
    int64_t n = (int64_t)s->size();
    if (r_chars && p_size > 0) {
        for (int64_t i = 0; i < n && i < p_size - 1; ++i)
            r_chars[i] = (char16_t)((*s)[(size_t)i] & 0xFFFF);
        r_chars[n < p_size ? n : p_size - 1] = 0;
    }
    return n;
}

static int64_t s_string_to_utf32_chars(const void *p_self, char32_t *r_chars, int64_t p_size) {
    const std::u32string *s = str_blob_get(p_self);
    if (!s) return 0;
    int64_t n = (int64_t)s->size();
    if (r_chars && p_size > 0) {
        int64_t copy = n < p_size - 1 ? n : p_size - 1;
        ::memcpy(r_chars, s->data(), (size_t)copy * sizeof(char32_t));
        r_chars[copy] = 0;
    }
    return n;
}

static int64_t s_string_to_wide_chars(const void *p_self, wchar_t *r_chars, int64_t p_size) {
    const std::u32string *s = str_blob_get(p_self);
    if (!s) return 0;
    int64_t n = (int64_t)s->size();
    if (r_chars && p_size > 0) {
        for (int64_t i = 0; i < n && i < p_size - 1; ++i)
            r_chars[i] = (wchar_t)((*s)[(size_t)i]);
        r_chars[n < p_size ? n : p_size - 1] = 0;
    }
    return n;
}

static int64_t s_string_resize(void *p_self, int64_t p_length) {
    std::u32string *s = str_blob_get(p_self);
    if (!s) return -1; // error
    s->resize((size_t)p_length, U'\0');
    return 0; // OK
}

// ── StringName new helpers ────────────────────────────────────────────────

static void s_string_name_new_with_latin1_chars(void *r_dest, const char *p_contents, uint8_t /*p_is_static*/) {
    sn_blob_set(r_dest, sn_intern(p_contents));
}

static void s_string_name_new_with_utf8_chars(void *r_dest, const char *p_contents) {
    sn_blob_set(r_dest, sn_intern(p_contents));
}

static void s_string_name_new_with_utf8_chars_and_len(void *r_dest, const char *p_contents, int64_t p_size) {
    sn_blob_set(r_dest, sn_intern(p_contents, (ptrdiff_t)p_size));
}

} // extern "C"

// Exported function pointer table
void *stub_string_new_with_latin1_chars_ptr              = (void *)s_string_new_with_latin1_chars;
void *stub_string_new_with_latin1_chars_and_len_ptr      = (void *)s_string_new_with_latin1_chars_and_len;
void *stub_string_new_with_utf8_chars_ptr                = (void *)s_string_new_with_utf8_chars;
void *stub_string_new_with_utf8_chars_and_len_ptr        = (void *)s_string_new_with_utf8_chars_and_len;
void *stub_string_new_with_utf8_chars_and_len2_ptr       = (void *)s_string_new_with_utf8_chars_and_len2;
void *stub_string_new_with_utf32_chars_ptr               = (void *)s_string_new_with_utf32_chars;
void *stub_string_new_with_utf32_chars_and_len_ptr       = (void *)s_string_new_with_utf32_chars_and_len;
void *stub_string_new_with_wide_chars_ptr                = (void *)s_string_new_with_wide_chars;
void *stub_string_new_with_wide_chars_and_len_ptr        = (void *)s_string_new_with_wide_chars_and_len;
void *stub_string_new_with_utf16_chars_ptr               = (void *)s_string_new_with_utf16_chars;
void *stub_string_new_with_utf16_chars_and_len_ptr       = (void *)s_string_new_with_utf16_chars_and_len;
void *stub_string_new_with_utf16_chars_and_len2_ptr      = (void *)s_string_new_with_utf16_chars_and_len2;
void *stub_string_operator_plus_eq_string_ptr            = (void *)s_string_operator_plus_eq_string;
void *stub_string_operator_plus_eq_cstr_ptr              = (void *)s_string_operator_plus_eq_cstr;
void *stub_string_operator_plus_eq_wcstr_ptr             = (void *)s_string_operator_plus_eq_wcstr;
void *stub_string_operator_plus_eq_c32str_ptr            = (void *)s_string_operator_plus_eq_c32str;
void *stub_string_operator_plus_eq_char_ptr              = (void *)s_string_operator_plus_eq_char;
void *stub_string_operator_index_ptr                     = (void *)s_string_operator_index;
void *stub_string_operator_index_const_ptr               = (void *)s_string_operator_index_const;
void *stub_string_to_latin1_chars_ptr                    = (void *)s_string_to_latin1_chars;
void *stub_string_to_utf8_chars_ptr                      = (void *)s_string_to_utf8_chars;
void *stub_string_to_utf16_chars_ptr                     = (void *)s_string_to_utf16_chars;
void *stub_string_to_utf32_chars_ptr                     = (void *)s_string_to_utf32_chars;
void *stub_string_to_wide_chars_ptr                      = (void *)s_string_to_wide_chars;
void *stub_string_resize_ptr                             = (void *)s_string_resize;
void *stub_string_name_new_with_latin1_chars_ptr         = (void *)s_string_name_new_with_latin1_chars;
void *stub_string_name_new_with_utf8_chars_ptr           = (void *)s_string_name_new_with_utf8_chars;
void *stub_string_name_new_with_utf8_chars_and_len_ptr   = (void *)s_string_name_new_with_utf8_chars_and_len;
