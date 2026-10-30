#include "native_telemetry.h"

#include <appmodel.h>
#include <winmeta.h>
#include <TraceLoggingProvider.h>

#include <limits>
#include <vector>

namespace initialization_telemetry {

TRACELOGGING_DEFINE_PROVIDER(
        provider,
        "Microsoft.Gaming.Middleware",
        (0xd13e8219, 0x218a, 0x51fc, 0x83, 0x62, 0x8c, 0x90, 0xcf, 0x60, 0x2d, 0x01),
        TraceLoggingOptionWindowsCoreTelemetry());

namespace {
bool registered = false;
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

PackageIdentity windows_package_identity(LONG &r_status) {
    UINT32 length = 0;
    r_status = GetCurrentPackageFamilyName(&length, nullptr);
    if (r_status == APPMODEL_ERROR_NO_PACKAGE) {
        return PackageIdentity::unpackaged;
    }
    if (r_status != ERROR_INSUFFICIENT_BUFFER || length == 0 || length > PACKAGE_FAMILY_NAME_MAX_LENGTH + 1) {
        return PackageIdentity::unknown;
    }
    std::vector<wchar_t> buffer(length);
    r_status = GetCurrentPackageFamilyName(&length, buffer.data());
    if (r_status == APPMODEL_ERROR_NO_PACKAGE) {
        return PackageIdentity::unpackaged;
    }
    return r_status == ERROR_SUCCESS && buffer[0] != L'\0'
            ? PackageIdentity::packaged
            : PackageIdentity::unknown;
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
    if (p_name.empty() || p_publisher.empty() ||
            !valid_value(p_name, PACKAGE_NAME_MAX_LENGTH) || !valid_value(p_publisher, 8192)) {
        return ERROR_INVALID_PARAMETER;
    }
    std::wstring name(p_name);
    std::wstring publisher(p_publisher);
    PACKAGE_ID identity{};
    identity.processorArchitecture = PROCESSOR_ARCHITECTURE_NEUTRAL;
    identity.name = name.data();
    identity.publisher = publisher.data();
    UINT32 length = 0;
    LONG status = PackageFamilyNameFromId(&identity, &length, nullptr);
    if (status != ERROR_INSUFFICIENT_BUFFER || length == 0 || length > PACKAGE_FAMILY_NAME_MAX_LENGTH + 1) {
        return status == ERROR_SUCCESS ? ERROR_INVALID_DATA : status;
    }
    std::vector<wchar_t> family(length);
    status = PackageFamilyNameFromId(&identity, &length, family.data());
    if (status != ERROR_SUCCESS) {
        return status;
    }

    UINT32 name_length = 0;
    UINT32 publisher_length = 0;
    status = PackageNameAndPublisherIdFromFamilyName(family.data(), &name_length, nullptr, &publisher_length, nullptr);
    if (status != ERROR_INSUFFICIENT_BUFFER || name_length == 0 || publisher_length == 0 ||
            name_length > PACKAGE_NAME_MAX_LENGTH + 1 || publisher_length > PACKAGE_PUBLISHERID_MAX_LENGTH + 1) {
        return status == ERROR_SUCCESS ? ERROR_INVALID_DATA : status;
    }
    std::vector<wchar_t> package_name(name_length);
    std::vector<wchar_t> publisher_id(publisher_length);
    status = PackageNameAndPublisherIdFromFamilyName(family.data(), &name_length, package_name.data(),
            &publisher_length, publisher_id.data());
    if (status == ERROR_SUCCESS) {
        r_family = family.data();
        r_publisher_id = publisher_id.data();
    }
    return status;
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
            !valid_value(p.configured_title_package_family_name, PACKAGE_FAMILY_NAME_MAX_LENGTH) ||
            !valid_value(p.package_publisher_id, PACKAGE_PUBLISHERID_MAX_LENGTH) ||
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
