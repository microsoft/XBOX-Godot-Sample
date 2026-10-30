#ifndef INITIALIZATION_TELEMETRY_NATIVE_TELEMETRY_H
#define INITIALIZATION_TELEMETRY_NATIVE_TELEMETRY_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "development_policy.h"
#include "TraceLoggingConfig.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace initialization_telemetry {

inline constexpr uint64_t INITIALIZATION_KEYWORD = MICROSOFT_KEYWORD_CRITICAL_DATA;
inline constexpr uint32_t INITIALIZATION_EVENT_TAGS = MICROSOFT_EVENTTAG_CORE_DATA;
inline constexpr size_t VERSION_LIMIT = 128;
inline constexpr size_t IDENTIFIER_LIMIT = 256;

enum class EnvironmentSuppression { absent_or_empty, suppressed, unavailable };
enum class ComponentKind : uint32_t { native_sdk = 1, managed_sdk = 2, engine_addon = 3, language_wrapper = 4 };
enum class DevelopmentContext : uint32_t { unknown = 0, editor = 1, editor_runtime = 2, development_runtime = 3, tooling = 4 };

struct EventPayload {
    std::wstring component_id;
    std::wstring component_version;
    std::wstring engine_version;
    std::wstring gaming_runtime_version;
    std::wstring gdk_build_sdk_version;
    DevelopmentContext development_context = DevelopmentContext::unknown;
    std::wstring configured_title_package_family_name;
    std::wstring package_publisher_id;
    uint32_t device_type = 0;
    std::wstring xbox_title_id;
    std::wstring playfab_title_id;
};

class ImageSession {
    GUID id{};
    uint64_t sequence = 0;
    bool initialized = false;

public:
    HRESULT initialize();
    bool record_success();
    const GUID &get_id() const { return id; }
    uint64_t get_sequence() const { return sequence; }
};

EnvironmentSuppression process_suppression();
PackageIdentity windows_package_identity(LONG &r_status);
LONG configured_package_identity(std::wstring_view p_name, std::wstring_view p_publisher,
        std::wstring &r_family, std::wstring &r_publisher_id);
bool valid_value(std::wstring_view p_value, size_t p_limit);
bool valid_payload(const EventPayload &p_payload);
DevelopmentContext development_context(DevelopmentDecision p_decision);
ULONG register_provider();
void unregister_provider();
bool provider_enabled();
void write_initialized(const ImageSession &p_session, const EventPayload &p_payload);

} // namespace initialization_telemetry

#endif // INITIALIZATION_TELEMETRY_NATIVE_TELEMETRY_H
