// Doctest coverage for playfab_internal:: HRESULT formatting helpers — the
// byte-identical sibling of gdk_internal:: (see test_gdk_result_codes.cpp).
// Every assertion here is intentionally identical to the GDK file modulo the
// namespace; if the two ever desync, both files will surface it.
//
// Same scope limitation as the GDK suite: only `format_hresult_hex` (the
// pure char-buffer formatter) is testable from a standalone doctest exe.
// The `godot::String`-returning helpers crash without a loaded engine
// because godot-cpp's String constructors dispatch through the
// GDExtension function-pointer table that is only populated when the addon
// is loaded by the editor or game runtime. See the GDK suite header
// comment for the full rationale.
#include "../third_party/doctest/doctest.h"

#include "playfab_result_codes_internal.h"

#include <cstring>

using playfab_internal::HRESULT_HEX_BUFFER_SIZE;
using playfab_internal::format_hresult_hex;

static_assert(noexcept(format_hresult_hex(HRESULT{}, static_cast<char *>(nullptr), std::size_t{0})),
        "playfab_internal::format_hresult_hex must remain noexcept");

static_assert(HRESULT_HEX_BUFFER_SIZE == 11,
        "playfab_internal::HRESULT_HEX_BUFFER_SIZE must remain 11 to match the "
        "fixed-width 0xXXXXXXXX format used everywhere else in the codebase");

TEST_CASE("playfab_internal::format_hresult_hex formats canonical HRESULTs") {
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
        CHECK(large[11] == 'X');
    }
}

TEST_CASE("playfab_internal::format_hresult_hex undersized buffer is documented-safe") {
    char buffer[HRESULT_HEX_BUFFER_SIZE - 1];
    std::memset(buffer, 'X', sizeof(buffer));

    char *result = format_hresult_hex(static_cast<HRESULT>(0x80004005L), buffer, sizeof(buffer));
    CHECK(result == buffer);
    CHECK(buffer[0] == '\0');
}

TEST_CASE("playfab_internal::format_hresult_hex zero-size buffer returns pointer untouched") {
    char buffer[HRESULT_HEX_BUFFER_SIZE];
    std::memset(buffer, 'X', sizeof(buffer));

    char *result = format_hresult_hex(static_cast<HRESULT>(0x80004005L), buffer, 0);
    CHECK(result == buffer);
    CHECK(buffer[0] == 'X');
}

TEST_CASE("playfab_internal::format_hresult_hex nullptr buffer returns nullptr") {
    char *result = format_hresult_hex(static_cast<HRESULT>(0x80004005L), nullptr, HRESULT_HEX_BUFFER_SIZE);
    CHECK(result == nullptr);
}
