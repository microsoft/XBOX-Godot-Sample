// Doctest coverage for gdk_internal:: HRESULT formatting helpers extracted in
// Wave 2 (commit 668fa4c). Pins the formatting contract for the only surface
// that is actually exercisable from a standalone doctest exe: the pure
// char-buffer formatter `format_hresult_hex`.
//
// Why no coverage of `format_hresult_string` / `format_hresult_message` /
// `code_or_format_hresult` here:
//
//   Those three helpers return `godot::String`. Every `godot::String`
//   constructor (including `String(const char*)` and `String::utf8(...)`)
//   dispatches through `godot::gdextension_interface::string_new_with_*`
//   function pointers. That table is populated by the engine during the
//   GDExtension entry-point handshake. In a standalone test exe no engine
//   ever runs, the pointers stay null, and constructing any non-empty
//   `godot::String` segfaults. An earlier draft of this file exercised those
//   helpers and crashed at runtime — see the final-report deviation note for
//   this wave. The String-returning helpers are thin one-line forwarders and
//   are observed end-to-end through the GUT suites that drive
//   `GDKResult::format_hresult` / `hresult_error` from real Godot processes.
//
// The PlayFab addon ships a byte-identical helper set; see
// test_playfab_result_codes.cpp. If the two ever desync, both suites should
// surface the regression.
#include "../third_party/doctest/doctest.h"

#include "gdk_result_codes_internal.h"

#include <cstring>

using gdk_internal::HRESULT_HEX_BUFFER_SIZE;
using gdk_internal::format_hresult_hex;

// Compile-time pin: format_hresult_hex must remain noexcept (callable from
// finalizers, async completion paths, etc.).
static_assert(noexcept(format_hresult_hex(HRESULT{}, static_cast<char *>(nullptr), std::size_t{0})),
        "gdk_internal::format_hresult_hex must remain noexcept");

// Compile-time pin: the buffer-size constant must stay 11 ("0xFFFFFFFF" + NUL).
static_assert(HRESULT_HEX_BUFFER_SIZE == 11,
        "gdk_internal::HRESULT_HEX_BUFFER_SIZE must remain 11 to match the "
        "fixed-width 0xXXXXXXXX format used everywhere else in the codebase");

TEST_CASE("gdk_internal::format_hresult_hex formats canonical HRESULTs") {
    char buffer[HRESULT_HEX_BUFFER_SIZE] = {};

    SUBCASE("S_OK -> 0x00000000") {
        char *result = format_hresult_hex(static_cast<HRESULT>(0x00000000), buffer, sizeof(buffer));
        CHECK(result == buffer);
        CHECK(std::strcmp(buffer, "0x00000000") == 0);
        CHECK(std::strlen(buffer) == 10u);
    }

    SUBCASE("E_FAIL -> 0x80004005") {
        char *result = format_hresult_hex(static_cast<HRESULT>(0x80004005L), buffer, sizeof(buffer));
        CHECK(result == buffer);
        CHECK(std::strcmp(buffer, "0x80004005") == 0);
    }

    SUBCASE("0xFFFFFFFF -> uppercase fixed width") {
        char *result = format_hresult_hex(static_cast<HRESULT>(0xFFFFFFFF), buffer, sizeof(buffer));
        CHECK(result == buffer);
        CHECK(std::strcmp(buffer, "0xFFFFFFFF") == 0);
    }

    SUBCASE("low-bit value still pads to 8 hex digits") {
        char *result = format_hresult_hex(static_cast<HRESULT>(0x00000001), buffer, sizeof(buffer));
        CHECK(result == buffer);
        CHECK(std::strcmp(buffer, "0x00000001") == 0);
    }

    SUBCASE("oversized buffer is fine and writes the same 10 chars + NUL") {
        char large[64];
        std::memset(large, 'X', sizeof(large));
        char *result = format_hresult_hex(static_cast<HRESULT>(0x80004005L), large, sizeof(large));
        CHECK(result == large);
        CHECK(std::strcmp(large, "0x80004005") == 0);
        // Bytes past the NUL are not touched by snprintf.
        CHECK(large[11] == 'X');
    }
}

TEST_CASE("gdk_internal::format_hresult_hex undersized buffer is documented-safe") {
    // Header contract: "If buffer_size is too small the buffer is left empty
    // (out_buffer[0] == '\0') and out_buffer is still returned." Pin both
    // halves of that contract.
    char buffer[HRESULT_HEX_BUFFER_SIZE - 1];
    std::memset(buffer, 'X', sizeof(buffer));

    char *result = format_hresult_hex(static_cast<HRESULT>(0x80004005L), buffer, sizeof(buffer));
    CHECK(result == buffer);
    CHECK(buffer[0] == '\0');
}

TEST_CASE("gdk_internal::format_hresult_hex zero-size buffer returns pointer untouched") {
    char buffer[HRESULT_HEX_BUFFER_SIZE];
    std::memset(buffer, 'X', sizeof(buffer));

    char *result = format_hresult_hex(static_cast<HRESULT>(0x80004005L), buffer, 0);
    CHECK(result == buffer);
    // No write happened — sentinel survives.
    CHECK(buffer[0] == 'X');
}

TEST_CASE("gdk_internal::format_hresult_hex nullptr buffer returns nullptr") {
    char *result = format_hresult_hex(static_cast<HRESULT>(0x80004005L), nullptr, HRESULT_HEX_BUFFER_SIZE);
    CHECK(result == nullptr);
}
