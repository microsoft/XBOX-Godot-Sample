// Doctest coverage for playfab_internal::match_request_key.
//
// This helper is extracted from the get_request_value key-selection logic in
// playfab_api_helpers.cpp. The pure form uses strcmp on a flat key array and
// has no Godot or GDK dependencies; production code uses godot::StringName
// hash lookups for O(1) performance (see the comment in playfab_api_helpers.cpp).
//
// Test matrix covers the documented return contract:
//   0  — primary matched
//   1  — fallback matched
//  -1  — neither matched (or degenerate input)

#include "doctest.h"

#include "playfab_request_key.h"

using playfab_internal::match_request_key;

TEST_CASE("match_request_key: degenerate inputs return -1") {
    SUBCASE("null keys pointer") {
        CHECK(match_request_key(nullptr, 3, "snake", "Pascal") == -1);
    }

    SUBCASE("zero key count") {
        const char *keys[] = { "snake", "Pascal" };
        CHECK(match_request_key(keys, 0, "snake", "Pascal") == -1);
    }

    SUBCASE("both primary and fallback null") {
        const char *keys[] = { "something" };
        CHECK(match_request_key(keys, 1, nullptr, nullptr) == -1);
    }

    SUBCASE("null primary and null fallback with populated key list") {
        const char *keys[] = { "a", "b", "c" };
        CHECK(match_request_key(keys, 3, nullptr, nullptr) == -1);
    }
}

TEST_CASE("match_request_key: no matching key returns -1") {
    const char *keys[] = { "alpha", "beta", "gamma" };

    SUBCASE("neither primary nor fallback present") {
        CHECK(match_request_key(keys, 3, "missing_snake", "MissingPascal") == -1);
    }

    SUBCASE("empty key array but non-null primary/fallback") {
        CHECK(match_request_key(nullptr, 0, "some_key", "SomeKey") == -1);
    }
}

TEST_CASE("match_request_key: primary (snake_case) wins when present") {
    SUBCASE("only primary present") {
        const char *keys[] = { "email_address" };
        CHECK(match_request_key(keys, 1, "email_address", "EmailAddress") == 0);
    }

    SUBCASE("both primary and fallback present — primary takes precedence") {
        const char *keys[] = { "email_address", "EmailAddress" };
        CHECK(match_request_key(keys, 2, "email_address", "EmailAddress") == 0);
    }

    SUBCASE("primary present in multi-key array") {
        const char *keys[] = { "unrelated", "email_address", "also_unrelated" };
        CHECK(match_request_key(keys, 3, "email_address", "EmailAddress") == 0);
    }

    SUBCASE("null fallback — primary still matches") {
        const char *keys[] = { "email_address" };
        CHECK(match_request_key(keys, 1, "email_address", nullptr) == 0);
    }
}

TEST_CASE("match_request_key: fallback (PascalCase) used when primary absent") {
    SUBCASE("only fallback present") {
        const char *keys[] = { "EmailAddress" };
        CHECK(match_request_key(keys, 1, "email_address", "EmailAddress") == 1);
    }

    SUBCASE("null primary — fallback used") {
        const char *keys[] = { "EmailAddress" };
        CHECK(match_request_key(keys, 1, nullptr, "EmailAddress") == 1);
    }

    SUBCASE("fallback present in multi-key array") {
        const char *keys[] = { "other_field", "EmailAddress", "yet_another" };
        CHECK(match_request_key(keys, 3, "email_address", "EmailAddress") == 1);
    }
}

TEST_CASE("match_request_key: identical primary and fallback strings") {
    // When primary == fallback (same literal value), match_request_key
    // should return 0 if that key is present (primary takes precedence).
    const char *keys[] = { "SameKey" };
    CHECK(match_request_key(keys, 1, "SameKey", "SameKey") == 0);
}

TEST_CASE("match_request_key: null entries in the key array are skipped") {
    const char *keys[] = { nullptr, "email_address", nullptr };
    CHECK(match_request_key(keys, 3, "email_address", "EmailAddress") == 0);
}

TEST_CASE("match_request_key: case-sensitive comparison") {
    // snake_case != SNAKE_CASE
    const char *keys[] = { "EMAIL_ADDRESS" };
    CHECK(match_request_key(keys, 1, "email_address", "EmailAddress") == -1);
}

TEST_CASE("match_request_key: single-char key names") {
    const char *keys[] = { "a", "B" };

    SUBCASE("primary single char matches") {
        CHECK(match_request_key(keys, 2, "a", "B") == 0);
    }

    SUBCASE("fallback single char matches") {
        CHECK(match_request_key(keys, 2, "x", "B") == 1);
    }
}
