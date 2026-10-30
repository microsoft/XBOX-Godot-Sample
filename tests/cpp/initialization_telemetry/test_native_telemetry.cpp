#include "../../../addons/initialization_telemetry/native_telemetry.h"
#include "doctest.h"

#include <vector>

using namespace initialization_telemetry;

TEST_CASE("telemetry session survives repeated setup and counts only explicit successes") {
    ImageSession session;
    CHECK_FALSE(session.record_success());
    REQUIRE(SUCCEEDED(session.initialize()));
    const GUID original = session.get_id();
    CHECK(session.get_sequence() == 0);
    REQUIRE(session.record_success());
    CHECK(session.get_sequence() == 1);
    REQUIRE(SUCCEEDED(session.initialize()));
    CHECK(IsEqualGUID(original, session.get_id()));
    REQUIRE(session.record_success());
    CHECK(session.get_sequence() == 2);
    ImageSession other_image;
    REQUIRE(SUCCEEDED(other_image.initialize()));
    CHECK_FALSE(IsEqualGUID(session.get_id(), other_image.get_id()));
    CHECK(other_image.get_sequence() == 0);
}

TEST_CASE("telemetry suppression reads the process environment without Boolean parsing") {
    const DWORD size = GetEnvironmentVariableW(L"GODOT_ADDON_SUPPRESS_TELEMETRY", nullptr, 0);
    std::vector<wchar_t> saved(size == 0 ? 1 : size);
    if (size != 0) {
        REQUIRE(GetEnvironmentVariableW(L"GODOT_ADDON_SUPPRESS_TELEMETRY", saved.data(), size) != 0);
    }
    struct Restore {
        const wchar_t *value;
        ~Restore() { SetEnvironmentVariableW(L"GODOT_ADDON_SUPPRESS_TELEMETRY", value); }
    } restore{size == 0 ? nullptr : saved.data()};
    REQUIRE(SetEnvironmentVariableW(L"GODOT_ADDON_SUPPRESS_TELEMETRY", nullptr));
    CHECK(process_suppression() == EnvironmentSuppression::absent_or_empty);
    for (const wchar_t *value : {L"0", L"false", L" ", L"\t", L"1"}) {
        REQUIRE(SetEnvironmentVariableW(L"GODOT_ADDON_SUPPRESS_TELEMETRY", value));
        CHECK(process_suppression() == EnvironmentSuppression::suppressed);
    }
}

TEST_CASE("configured identity derives Windows family and publisher IDs without an installation token") {
    std::wstring family;
    std::wstring publisher;
    REQUIRE(configured_package_identity(L"Telemetry.Synthetic.One", L"CN=Telemetry Synthetic",
                    family, publisher) == ERROR_SUCCESS);
    CHECK_FALSE(family.empty());
    CHECK(publisher.size() == 13);
    CHECK(family == L"Telemetry.Synthetic.One_" + publisher);
    std::wstring second_family;
    std::wstring second_publisher;
    REQUIRE(configured_package_identity(L"Telemetry.Synthetic.Two", L"CN=Telemetry Synthetic",
                    second_family, second_publisher) == ERROR_SUCCESS);
    CHECK(family != second_family);
    CHECK(publisher == second_publisher);
    REQUIRE(configured_package_identity(L"Telemetry.Synthetic.Two", L"CN=Different Synthetic",
                    second_family, second_publisher) == ERROR_SUCCESS);
    CHECK(publisher != second_publisher);
    CHECK(configured_package_identity(L"", L"", family, publisher) == ERROR_INVALID_PARAMETER);
    CHECK(family.empty());
    CHECK(publisher.empty());
}

TEST_CASE("telemetry payload rejects oversized, malformed Unicode and noncanonical identity values") {
    CHECK(valid_value(std::wstring(VERSION_LIMIT, L'v'), VERSION_LIMIT));
    CHECK_FALSE(valid_value(std::wstring(VERSION_LIMIT + 1, L'v'), VERSION_LIMIT));
    CHECK_FALSE(valid_value(std::wstring_view(L"a\0b", 3), IDENTIFIER_LIMIT));
    CHECK_FALSE(valid_value(L"line\nbreak", IDENTIFIER_LIMIT));
    CHECK_FALSE(valid_value(std::wstring(1, wchar_t(0xd800)), IDENTIFIER_LIMIT));
    CHECK(valid_value(std::wstring{wchar_t(0xd83d), wchar_t(0xde00)}, IDENTIFIER_LIMIT));
    CHECK_FALSE(valid_value(std::wstring(1, wchar_t(0xdc00)), IDENTIFIER_LIMIT));
    EventPayload payload;
    payload.component_id = L"godot_gdk";
    payload.component_version = L"0.3.0";
    payload.engine_version = L"4.6.2.stable.official";
    payload.development_context = DevelopmentContext::development_runtime;
    REQUIRE(valid_payload(payload));
    payload.xbox_title_id = L"1234ABCD";
    CHECK(valid_payload(payload));
    payload.xbox_title_id = L"1234abcd";
    CHECK_FALSE(valid_payload(payload));
    payload.xbox_title_id.clear();
    payload.component_version.assign(VERSION_LIMIT + 1, L'v');
    CHECK_FALSE(valid_payload(payload));
}

TEST_CASE("telemetry accepts every SDK device model and future device type values") {
    EventPayload payload;
    payload.component_id = L"godot_gdk";
    payload.component_version = L"0.3.0";
    payload.development_context = DevelopmentContext::development_runtime;
    for (const uint32_t device : {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 0xdeadbeefu, 0xffffffffu}) {
        payload.device_type = device;
        CHECK(valid_payload(payload));
    }
}

TEST_CASE("local telemetry provider registration is idempotent and can be safely torn down") {
    CHECK(INITIALIZATION_KEYWORD == 0x0000800000000000ULL);
    CHECK(INITIALIZATION_EVENT_TAGS == 0x00080000u);
    REQUIRE(register_provider() == ERROR_SUCCESS);
    CHECK(register_provider() == ERROR_SUCCESS);
    unregister_provider();
    unregister_provider();
    CHECK_FALSE(provider_enabled());
}
