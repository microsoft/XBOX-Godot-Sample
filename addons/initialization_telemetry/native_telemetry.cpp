#include "native_telemetry.h"

#include <bcrypt.h>
#include <winmeta.h>
#include <TraceLoggingProvider.h>

#include <limits>

namespace initialization_telemetry {

TRACELOGGING_DEFINE_PROVIDER(
        provider,
        "Microsoft.Gaming.Middleware",
        (0xab2f0be6, 0x763c, 0x4048, 0x81, 0x8b, 0x33, 0x99, 0x94, 0x58, 0xb2, 0x1c),
        TraceLoggingOptionWindowsCoreTelemetry());

namespace {
bool registered = false;
constexpr size_t PACKAGE_NAME_LIMIT = 50;
constexpr size_t PACKAGE_FAMILY_NAME_LIMIT = 64;
constexpr size_t PACKAGE_PUBLISHER_ID_LIMIT = 13;
}

bool InitializationCounter::record_success() {
    if (sequence == std::numeric_limits<uint64_t>::max()) {
        return false;
    }
    ++sequence;
    return true;
}

EnvironmentSuppression process_suppression() {
    wchar_t unused[1]{};
    SetLastError(ERROR_SUCCESS);
    const DWORD length = GetEnvironmentVariableW(L"GODOT_ADDON_SUPPRESS_TELEMETRY", unused, 1);
    if (length != 0) {
        return EnvironmentSuppression::suppressed;
    }
    const DWORD status = GetLastError();
    return status == ERROR_SUCCESS || status == ERROR_ENVVAR_NOT_FOUND
            ? EnvironmentSuppression::absent_or_empty
            : EnvironmentSuppression::unavailable;
}

bool valid_value(std::wstring_view p_value, size_t p_limit) {
    if (p_value.size() > p_limit) {
        return false;
    }
    for (size_t i = 0; i < p_value.size(); ++i) {
        const auto value = static_cast<uint16_t>(p_value[i]);
        if (value < 0x20 || value == 0x7f) {
            return false;
        }
        if (value >= 0xd800 && value <= 0xdbff) {
            if (++i == p_value.size() || p_value[i] < 0xdc00 || p_value[i] > 0xdfff) {
                return false;
            }
        } else if (value >= 0xdc00 && value <= 0xdfff) {
            return false;
        }
    }
    return true;
}

LONG configured_package_identity(std::wstring_view p_name, std::wstring_view p_publisher,
        std::wstring &r_family, std::wstring &r_publisher_id) {
    r_family.clear();
    r_publisher_id.clear();
    if (p_name.size() < 3 || p_publisher.empty() ||
            !valid_value(p_name, PACKAGE_NAME_LIMIT) || !valid_value(p_publisher, 8192)) {
        return ERROR_INVALID_PARAMETER;
    }
    for (const wchar_t value : p_name) {
        if (!(value >= L'a' && value <= L'z') && !(value >= L'A' && value <= L'Z') &&
                !(value >= L'0' && value <= L'9') && value != L'.' && value != L'-') {
            return ERROR_INVALID_PARAMETER;
        }
    }
    static_assert(sizeof(wchar_t) == 2, "Publisher hashing requires UTF-16LE on Windows.");
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (status < 0) {
        return status;
    }
    UCHAR digest[32]{};
    status = BCryptHash(algorithm, nullptr, 0,
            reinterpret_cast<PUCHAR>(const_cast<wchar_t *>(p_publisher.data())),
            static_cast<ULONG>(p_publisher.size() * sizeof(wchar_t)), digest, sizeof(digest));
    const NTSTATUS close_status = BCryptCloseAlgorithmProvider(algorithm, 0);
    if (status < 0 || close_status < 0) {
        return status < 0 ? status : close_status;
    }
    // Windows publisher IDs use lowercase Crockford base32, MSB first, with one trailing zero bit.
    constexpr wchar_t alphabet[] = L"0123456789abcdefghjkmnpqrstvwxyz";
    std::wstring publisher_id;
    publisher_id.reserve(PACKAGE_PUBLISHER_ID_LIMIT);
    for (size_t group = 0; group < PACKAGE_PUBLISHER_ID_LIMIT; ++group) {
        unsigned index = 0;
        for (size_t bit = group * 5; bit < group * 5 + 5; ++bit) {
            index = (index << 1) | (bit < 64 ? ((digest[bit / 8] >> (7 - bit % 8)) & 1) : 0);
        }
        publisher_id += alphabet[index];
    }
    r_family = std::wstring(p_name) + L"_" + publisher_id;
    r_publisher_id = publisher_id;
    return ERROR_SUCCESS;
}

DevelopmentContext development_context(DevelopmentDecision p_decision) {
    switch (p_decision) {
        case DevelopmentDecision::editor: return DevelopmentContext::editor;
        case DevelopmentDecision::editor_runtime: return DevelopmentContext::editor_runtime;
        case DevelopmentDecision::development_runtime: return DevelopmentContext::development_runtime;
        default: return DevelopmentContext::unknown;
    }
}

bool valid_payload(const EventPayload &p) {
    if (p.component_id.empty() || p.component_version.empty() ||
            !valid_value(p.component_id, IDENTIFIER_LIMIT) || !valid_value(p.component_version, VERSION_LIMIT) ||
            !valid_value(p.engine_version, VERSION_LIMIT) || !valid_value(p.gaming_runtime_version, VERSION_LIMIT) ||
            !valid_value(p.gdk_build_sdk_version, VERSION_LIMIT) ||
            !valid_value(p.configured_title_package_family_name, PACKAGE_FAMILY_NAME_LIMIT) ||
            !valid_value(p.package_publisher_id, PACKAGE_PUBLISHER_ID_LIMIT) ||
            !valid_value(p.playfab_title_id, IDENTIFIER_LIMIT) ||
            p.development_context < DevelopmentContext::editor ||
            p.development_context > DevelopmentContext::development_runtime) {
        return false;
    }
    if (!p.xbox_title_id.empty()) {
        if (p.xbox_title_id.size() != 8) {
            return false;
        }
        for (const wchar_t value : p.xbox_title_id) {
            if (!(value >= L'0' && value <= L'9') && !(value >= L'A' && value <= L'F')) {
                return false;
            }
        }
    }
    return true;
}

ULONG register_provider() {
    if (registered) {
        return ERROR_SUCCESS;
    }
    const ULONG status = TraceLoggingRegister(provider);
    registered = status == ERROR_SUCCESS;
    return status;
}

void unregister_provider() {
    if (registered) {
        TraceLoggingUnregister(provider);
        registered = false;
    }
}

bool provider_enabled() {
    return registered && TraceLoggingProviderEnabled(provider, WINEVENT_LEVEL_INFO, INITIALIZATION_KEYWORD);
}

void write_initialized(const InitializationCounter &p_counter, const EventPayload &p) {
    TraceLoggingWrite(provider, "SdkComponentInitialized",
            TraceLoggingLevel(WINEVENT_LEVEL_INFO),
            TraceLoggingKeyword(INITIALIZATION_KEYWORD),
            TraceLoggingEventTag(INITIALIZATION_EVENT_TAGS),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingUInt32(1, "SchemaVersion"),
            TraceLoggingWideString(p.component_id.c_str(), "ComponentId"),
            TraceLoggingWideString(p.component_version.c_str(), "ComponentVersion"),
            TraceLoggingUInt32(static_cast<uint32_t>(ComponentKind::engine_addon), "ComponentKind"),
            TraceLoggingUInt64(p_counter.get_sequence(), "InitializationSequence"),
            TraceLoggingWideString(L"Godot", "EngineName"),
            TraceLoggingWideString(p.engine_version.c_str(), "EngineVersion"),
            TraceLoggingWideString(L"", "ExecutionRuntimeName"),
            TraceLoggingWideString(L"", "ExecutionRuntimeVersion"),
            TraceLoggingWideString(p.gaming_runtime_version.c_str(), "GamingRuntimeVersion"),
            TraceLoggingWideString(p.gdk_build_sdk_version.c_str(), "GdkBuildSdkVersion"),
            TraceLoggingUInt32(static_cast<uint32_t>(p.development_context), "DevelopmentContext"),
            TraceLoggingWideString(p.configured_title_package_family_name.c_str(), "ConfiguredTitlePackageFamilyName"),
            TraceLoggingWideString(p.package_publisher_id.c_str(), "PackagePublisherId"),
            TraceLoggingUInt32(p.device_type, "DeviceType"),
            TraceLoggingWideString(p.xbox_title_id.c_str(), "XboxTitleId"),
            TraceLoggingWideString(p.playfab_title_id.c_str(), "PlayFabTitleId"));
}

} // namespace initialization_telemetry
