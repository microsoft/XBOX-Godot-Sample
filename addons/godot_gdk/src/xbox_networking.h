#ifndef XBOX_NETWORKING_H
#define XBOX_NETWORKING_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <memory>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/signal.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XNetworking.h>

namespace godot {

class Xbox;
class XboxResult;
class XboxRuntime;

// XboxNetworkingSecurityInformation
// --------------------------------
// RefCounted result of XboxNetworking::query_security_information_for_url_async().
// Wraps the Network Security Allow List (NSAL) entry the GDK returns for a URL
// registered in Partner Center: the enabled HTTP security protocol flags plus
// the certificate thumbprints to pin against.
//
// The native XNetworkingSecurityInformation is a header struct whose pointers
// point *into* the caller-supplied byte buffer that
// XNetworkingQuerySecurityInformationForUrlAsyncResult writes, so the buffer
// cannot be copied or relocated without invalidating them. This wrapper owns
// that buffer for its whole lifetime and parses it into Godot types up front.
// Retaining the raw buffer (rather than only the parsed copy) keeps a future
// native-interop entry point additive; see the XNetworkingVerifyServerCertificate
// note on XboxNetworking.
class XboxNetworkingSecurityInformation : public RefCounted {
    GDCLASS(XboxNetworkingSecurityInformation, RefCounted);

    std::unique_ptr<uint8_t[]> m_buffer;
    size_t m_buffer_size = 0;
    const XNetworkingSecurityInformation *m_native = nullptr;
    int64_t m_enabled_http_security_protocol_flags = 0;
    Array m_thumbprints;

protected:
    static void _bind_methods();

public:
    // Bitmask of the HTTP security protocols the NSAL entry enables for the URL.
    int64_t get_enabled_http_security_protocol_flags() const;

    // Array of Dictionaries, one per certificate thumbprint:
    //   { type: int (XboxNetworking.THUMBPRINT_TYPE_*), type_name: String,
    //     bytes: PackedByteArray, hex: String }
    Array get_thumbprints() const;

    // Whole record as a Dictionary:
    //   { enabled_http_security_protocol_flags: int, thumbprints: Array }
    Dictionary to_dictionary() const;

    // Internal: takes ownership of the result buffer and parses p_native, which
    // must point into that buffer. Called only by XboxNetworking.
    void set_native_internal(std::unique_ptr<uint8_t[]> p_buffer, size_t p_buffer_size, const XNetworkingSecurityInformation *p_native);
};

// XboxNetworking
// -------------
// Networking service. Exposed as GDK.networking. Wraps XNetworking.h: the
// preferred local UDP multiplayer port, connectivity hints, NSAL security
// information for title endpoints, and the TCP receive-buffer configuration and
// statistics surfaces.
//
// Availability: every XNetworking entry point below is exported by the PC GDK
// xgameruntime.lib and documented as "Windows, Xbox One family, Xbox Series".
// Three of them are documented no-ops on Windows and are wrapped anyway so the
// surface is identical on console-capable Godot forks:
//   * set_configuration_setting()   -- returns E_NOTIMPL on Windows.
//   * query_configuration_setting() -- always reports 0 on Windows.
//   * query_statistics()            -- succeeds but reports all zeros on Windows.
//
// Not wrapped: XNetworkingVerifyServerCertificate. It takes a WinHTTP
// HINTERNET request handle obtained from WinHttpOpenRequest and is meant to be
// called from inside a WINHTTP_CALLBACK_STATUS_SENDING_REQUEST callback. Godot
// has no WinHTTP handle to hand out (HTTPClient/HTTPRequest do not expose one),
// so there is no way to call it correctly from GDScript. Titles that need
// certificate pinning can read the thumbprints from
// XboxNetworkingSecurityInformation and validate in their own native code.
//
// Also not wrapped: XNetworkingQuerySecurityInformationForUrlUtf16Async. It
// differs from the UTF-8 entry point only in input encoding, and Godot has a
// single String type, so the two are indistinguishable from script.
class XboxNetworking : public RefCounted {
    GDCLASS(XboxNetworking, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;
    bool m_port_changed_registered = false;
    bool m_connectivity_changed_registered = false;
    XTaskQueueRegistrationToken m_port_changed_token = {};
    XTaskQueueRegistrationToken m_connectivity_changed_token = {};

    // Signatures match the XNetworking.h typedefs, which - unlike
    // XGameActivationCallback - do not carry the CALLBACK macro.
    static void _port_changed_callback(void *p_context, uint16_t p_port);
    static void _connectivity_hint_changed_callback(void *p_context, const XNetworkingConnectivityHint *p_hint);

    void handle_port_changed_internal(uint16_t p_port);
    void handle_connectivity_hint_changed_internal(const XNetworkingConnectivityHint *p_hint);

    XboxRuntime *_get_runtime() const;
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) const;

protected:
    static void _bind_methods();

public:
    // Maps to XNetworkingConnectivityLevelHint.
    enum ConnectivityLevelHint {
        CONNECTIVITY_LEVEL_UNKNOWN = static_cast<uint32_t>(XNetworkingConnectivityLevelHint::Unknown),
        CONNECTIVITY_LEVEL_NONE = static_cast<uint32_t>(XNetworkingConnectivityLevelHint::None),
        CONNECTIVITY_LEVEL_LOCAL_ACCESS = static_cast<uint32_t>(XNetworkingConnectivityLevelHint::LocalAccess),
        CONNECTIVITY_LEVEL_INTERNET_ACCESS = static_cast<uint32_t>(XNetworkingConnectivityLevelHint::InternetAccess),
        CONNECTIVITY_LEVEL_CONSTRAINED_INTERNET_ACCESS = static_cast<uint32_t>(XNetworkingConnectivityLevelHint::ConstrainedInternetAccess),
    };

    // Maps to XNetworkingConnectivityCostHint.
    enum ConnectivityCostHint {
        CONNECTIVITY_COST_UNKNOWN = static_cast<uint32_t>(XNetworkingConnectivityCostHint::Unknown),
        CONNECTIVITY_COST_UNRESTRICTED = static_cast<uint32_t>(XNetworkingConnectivityCostHint::Unrestricted),
        CONNECTIVITY_COST_FIXED = static_cast<uint32_t>(XNetworkingConnectivityCostHint::Fixed),
        CONNECTIVITY_COST_VARIABLE = static_cast<uint32_t>(XNetworkingConnectivityCostHint::Variable),
    };

    // Maps to XNetworkingThumbprintType.
    enum ThumbprintType {
        THUMBPRINT_TYPE_LEAF = static_cast<uint32_t>(XNetworkingThumbprintType::Leaf),
        THUMBPRINT_TYPE_ISSUER = static_cast<uint32_t>(XNetworkingThumbprintType::Issuer),
        THUMBPRINT_TYPE_ROOT = static_cast<uint32_t>(XNetworkingThumbprintType::Root),
    };

    // Maps to XNetworkingConfigurationSetting.
    enum ConfigurationSetting {
        CONFIGURATION_SETTING_MAX_TITLE_TCP_QUEUED_RECEIVE_BUFFER_SIZE = static_cast<uint32_t>(XNetworkingConfigurationSetting::MaxTitleTcpQueuedReceiveBufferSize),
        CONFIGURATION_SETTING_MAX_SYSTEM_TCP_QUEUED_RECEIVE_BUFFER_SIZE = static_cast<uint32_t>(XNetworkingConfigurationSetting::MaxSystemTcpQueuedReceiveBufferSize),
        CONFIGURATION_SETTING_MAX_TOOLS_TCP_QUEUED_RECEIVE_BUFFER_SIZE = static_cast<uint32_t>(XNetworkingConfigurationSetting::MaxToolsTcpQueuedReceiveBufferSize),
    };

    // Maps to XNetworkingStatisticsType.
    enum StatisticsType {
        STATISTICS_TYPE_TITLE_TCP_QUEUED_RECEIVED_BUFFER_USAGE = static_cast<uint32_t>(XNetworkingStatisticsType::TitleTcpQueuedReceivedBufferUsage),
        STATISTICS_TYPE_SYSTEM_TCP_QUEUED_RECEIVED_BUFFER_USAGE = static_cast<uint32_t>(XNetworkingStatisticsType::SystemTcpQueuedReceivedBufferUsage),
        STATISTICS_TYPE_TOOLS_TCP_QUEUED_RECEIVED_BUFFER_USAGE = static_cast<uint32_t>(XNetworkingStatisticsType::ToolsTcpQueuedReceivedBufferUsage),
    };

    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();

    // Preferred local UDP port for multiplayer traffic (default 3074), in host
    // byte order. Wraps XNetworkingQueryPreferredLocalUdpMultiplayerPort.
    //
    // The GDK documents this synchronous call as unsafe on time-sensitive
    // threads, which Godot's main thread is. Prefer
    // query_preferred_local_udp_multiplayer_port_async() from _ready()/_process()
    // and keep this overload for worker threads and startup code.
    //
    // Returns an XboxResult whose data is a Dictionary: { port: int }.
    Ref<XboxResult> query_preferred_local_udp_multiplayer_port();

    // Async form of the above, safe to start from the main thread. Wraps
    // XNetworkingQueryPreferredLocalUdpMultiplayerPortAsync /
    // ...AsyncResult. Completes with an XboxResult whose data is { port: int }.
    Signal query_preferred_local_udp_multiplayer_port_async();

    // Current connectivity hint. Wraps XNetworkingGetConnectivityHint; safe on
    // the main thread. Returns an XboxResult whose data is the hint Dictionary
    // described by make_connectivity_hint_dictionary().
    //
    // network_initialized is the only authoritative field; the rest are
    // best-effort device-wide hints and must not be treated as a reachability
    // test for any specific endpoint.
    Ref<XboxResult> get_connectivity_hint();

    // NSAL security information (certificate thumbprints) for a title endpoint
    // registered in Partner Center. Wraps
    // XNetworkingQuerySecurityInformationForUrlAsync / ...AsyncResultSize /
    // ...AsyncResult. Completes with an XboxResult whose data is a
    // Ref<XboxNetworkingSecurityInformation>.
    Signal query_security_information_for_url_async(const String &p_url);

    // Wraps XNetworkingQueryConfigurationSetting. Returns an XboxResult whose
    // data is a Dictionary: { value: int, unlimited: bool }. `unlimited` is true
    // when the native uint64 value is UINT64_MAX (reported as -1 in `value`,
    // which cannot represent it). Always reports 0 / false on Windows.
    Ref<XboxResult> query_configuration_setting(int64_t p_setting);

    // Wraps XNetworkingSetConfigurationSetting. p_value must be >= 0. Returns
    // E_NOTIMPL as an error XboxResult with code "not_supported_on_platform" on
    // Windows, where the setting is a documented no-op.
    Ref<XboxResult> set_configuration_setting(int64_t p_setting, int64_t p_value);

    // Wraps XNetworkingQueryStatistics. Returns an XboxResult whose data is a
    // Dictionary of XNetworkingTcpQueuedReceivedBufferUsageStatistics fields.
    // Always reports zeros on Windows.
    Ref<XboxResult> query_statistics(int64_t p_statistics_type);

    static Dictionary make_connectivity_hint_dictionary(const XNetworkingConnectivityHint &p_hint);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::XboxNetworking::ConnectivityLevelHint);
VARIANT_ENUM_CAST(godot::XboxNetworking::ConnectivityCostHint);
VARIANT_ENUM_CAST(godot::XboxNetworking::ThumbprintType);
VARIANT_ENUM_CAST(godot::XboxNetworking::ConfigurationSetting);
VARIANT_ENUM_CAST(godot::XboxNetworking::StatisticsType);

#endif // XBOX_NETWORKING_H
