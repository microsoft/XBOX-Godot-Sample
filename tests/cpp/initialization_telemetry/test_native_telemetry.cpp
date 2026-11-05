#include "../../../addons/initialization_telemetry/native_telemetry.h"
#include "doctest.h"

#include <appmodel.h>
#include <vector>

using namespace initialization_telemetry;

TEST_CASE("telemetry counter counts only explicit successes independently for each image") {
    InitializationCounter counter;
    CHECK(counter.get_sequence() == 0);
    REQUIRE(counter.record_success());
    CHECK(counter.get_sequence() == 1);
    REQUIRE(counter.record_success());
    CHECK(counter.get_sequence() == 2);
    InitializationCounter other_image;
    CHECK(other_image.get_sequence() == 0);
    REQUIRE(other_image.record_success());
    CHECK(other_image.get_sequence() == 1);
    CHECK(counter.get_sequence() == 2);
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

TEST_CASE("manifest identity hashing matches known publisher IDs and the desktop package API") {
    struct Identity {
        const wchar_t *name;
        const wchar_t *publisher;
        const wchar_t *publisher_id;
    };
    for (const auto &value : {
                 Identity{L"Microsoft.WindowsCalculator",
                         L"CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond, S=Washington, C=US",
                         L"8wekyb3d8bbwe"},
                 Identity{L"Microsoft.Windows.ShellExperienceHost",
                         L"CN=Microsoft Windows, O=Microsoft Corporation, L=Redmond, S=Washington, C=US",
                         L"cw5n1h2txyewy"}}) {
        std::wstring family;
        std::wstring publisher_id;
        REQUIRE(configured_package_identity(value.name, value.publisher, family, publisher_id) == ERROR_SUCCESS);
        CHECK(publisher_id == value.publisher_id);
        CHECK(family == std::wstring(value.name) + L"_" + value.publisher_id);
    }
    for (const std::wstring &publisher : {
                 std::wstring(L"CN=Telemetry Synthetic"), std::wstring(L"CN=Different Synthetic"),
                 std::wstring(L"CN=T\u00e9l\u00e9m\u00e9trie \u4e2d\u6587"),
                 std::wstring(L"CN=Supplementary \U0001f600")}) {
        for (const std::wstring &name : {std::wstring(L"Telemetry.Synthetic.One"),
                     std::wstring(L"Abc"), std::wstring(PACKAGE_NAME_MAX_LENGTH, L'A')}) {
            std::wstring family;
            std::wstring publisher_id;
            REQUIRE(configured_package_identity(name, publisher, family, publisher_id) == ERROR_SUCCESS);
            PACKAGE_ID identity{};
            identity.processorArchitecture = PROCESSOR_ARCHITECTURE_NEUTRAL;
            identity.name = const_cast<wchar_t *>(name.c_str());
            identity.publisher = const_cast<wchar_t *>(publisher.c_str());
            UINT32 length = 0;
            REQUIRE(PackageFamilyNameFromId(&identity, &length, nullptr) == ERROR_INSUFFICIENT_BUFFER);
            std::vector<wchar_t> expected(length);
            REQUIRE(PackageFamilyNameFromId(&identity, &length, expected.data()) == ERROR_SUCCESS);
            CHECK(family == expected.data());
            CHECK(publisher_id.size() == PACKAGE_PUBLISHERID_MAX_LENGTH);
        }
    }
}

TEST_CASE("manifest identity hashing preserves input and payload limits") {
    std::wstring family = L"stale";
    std::wstring publisher = L"stale";
    for (const auto &name : {std::wstring(L""), std::wstring(L"ab"), std::wstring(L"has_space"),
                 std::wstring(L"has space"), std::wstring(L"invalid\u00e9"),
                 std::wstring(PACKAGE_NAME_MAX_LENGTH + 1, L'A')}) {
        CHECK(configured_package_identity(name, L"CN=Telemetry Synthetic", family, publisher) == ERROR_INVALID_PARAMETER);
        CHECK(family.empty());
        CHECK(publisher.empty());
    }
    for (const auto &value : {std::wstring(), std::wstring(8193, L'P'),
                 std::wstring(L"CN=line\nbreak"), std::wstring(L"CN=a\0b", 6),
                 std::wstring(1, wchar_t(0xd800))}) {
        CHECK(configured_package_identity(L"Telemetry.Synthetic", value, family, publisher) == ERROR_INVALID_PARAMETER);
        CHECK(family.empty());
        CHECK(publisher.empty());
    }
    REQUIRE(configured_package_identity(std::wstring(PACKAGE_NAME_MAX_LENGTH, L'A'),
                    std::wstring(8192, L'P'), family, publisher) == ERROR_SUCCESS);
    CHECK(family.size() == PACKAGE_FAMILY_NAME_MAX_LENGTH);
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
