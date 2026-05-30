#include "../third_party/doctest/doctest.h"

#include "playfab_owned_utf8.h"

#include <cstring>
#include <string>

TEST_CASE("playfab_internal::OwnedUtf8String survives transient request buffers until completion") {
    constexpr const char *expected = "save description \xE2\x98\x83 from a transient CharString-shaped buffer";

    playfab_internal::OwnedUtf8String owned;
    const char *owned_completion_ptr = nullptr;
    {
        std::string transient = expected;
        owned.set(transient.c_str());
        owned_completion_ptr = owned.c_str();
        CHECK(std::strcmp(owned_completion_ptr, expected) == 0);
    }

    CHECK(owned.c_str() == owned_completion_ptr);
    CHECK(std::strcmp(owned.c_str(), expected) == 0);
}

TEST_CASE("playfab_internal::OwnedUtf8String supports Party optional-null and auth string shapes") {
    playfab_internal::OwnedUtf8String empty;
    CHECK(std::strcmp(empty.c_str(), "") == 0);
    CHECK(empty.c_str_or_null() == nullptr);

    constexpr const char *invitation_id = "party-invite-lifetime-pin-123456789";
    playfab_internal::OwnedUtf8String auth_string(invitation_id);
    CHECK(auth_string.c_str_or_null() != nullptr);
    CHECK(std::strcmp(auth_string.c_str(), invitation_id) == 0);

    std::string overwrite_source = invitation_id;
    auth_string.set(overwrite_source.c_str());
    const char *completion_ptr = auth_string.c_str();
    overwrite_source.assign(4096, 'x');

    CHECK(auth_string.c_str() == completion_ptr);
    CHECK(std::strcmp(auth_string.c_str(), invitation_id) == 0);
}
