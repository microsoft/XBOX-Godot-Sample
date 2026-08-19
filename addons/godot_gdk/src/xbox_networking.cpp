#include "xbox_networking.h"

#include <cstdio>

#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "xbox.h"
#include "xbox_pending_signal.h"
#include "xbox_result.h"
#include "xbox_runtime.h"
#include "xbox_signal_xasync_context.h"

namespace godot {

namespace {

String _connectivity_level_name(XNetworkingConnectivityLevelHint p_level) {
    switch (p_level) {
        case XNetworkingConnectivityLevelHint::None:
            return "none";
        case XNetworkingConnectivityLevelHint::LocalAccess:
            return "local_access";
        case XNetworkingConnectivityLevelHint::InternetAccess:
            return "internet_access";
        case XNetworkingConnectivityLevelHint::ConstrainedInternetAccess:
            return "constrained_internet_access";
        case XNetworkingConnectivityLevelHint::Unknown:
        default:
            return "unknown";
    }
}

String _connectivity_cost_name(XNetworkingConnectivityCostHint p_cost) {
    switch (p_cost) {
        case XNetworkingConnectivityCostHint::Unrestricted:
            return "unrestricted";
        case XNetworkingConnectivityCostHint::Fixed:
            return "fixed";
        case XNetworkingConnectivityCostHint::Variable:
            return "variable";
        case XNetworkingConnectivityCostHint::Unknown:
        default:
            return "unknown";
    }
}

String _thumbprint_type_name(XNetworkingThumbprintType p_type) {
    switch (p_type) {
        case XNetworkingThumbprintType::Issuer:
            return "issuer";
        case XNetworkingThumbprintType::Root:
            return "root";
        case XNetworkingThumbprintType::Leaf:
        default:
            return "leaf";
    }
}

String _to_hex(const uint8_t *p_bytes, size_t p_count) {
    if (p_bytes == nullptr || p_count == 0) {
        return String();
    }

    String hex;
    char pair[3] = {};
    for (size_t i = 0; i < p_count; ++i) {
        std::snprintf(pair, sizeof(pair), "%02x", static_cast<unsigned int>(p_bytes[i]));
        hex += String(pair);
    }
    return hex;
}

bool _to_native_configuration_setting(int64_t p_value, XNetworkingConfigurationSetting *r_setting) {
    switch (p_value) {
        case XboxNetworking::CONFIGURATION_SETTING_MAX_TITLE_TCP_QUEUED_RECEIVE_BUFFER_SIZE:
            *r_setting = XNetworkingConfigurationSetting::MaxTitleTcpQueuedReceiveBufferSize;
            return true;
        case XboxNetworking::CONFIGURATION_SETTING_MAX_SYSTEM_TCP_QUEUED_RECEIVE_BUFFER_SIZE:
            *r_setting = XNetworkingConfigurationSetting::MaxSystemTcpQueuedReceiveBufferSize;
            return true;
        case XboxNetworking::CONFIGURATION_SETTING_MAX_TOOLS_TCP_QUEUED_RECEIVE_BUFFER_SIZE:
            *r_setting = XNetworkingConfigurationSetting::MaxToolsTcpQueuedReceiveBufferSize;
            return true;
        default:
            return false;
    }
}

bool _to_native_statistics_type(int64_t p_value, XNetworkingStatisticsType *r_type) {
    switch (p_value) {
        case XboxNetworking::STATISTICS_TYPE_TITLE_TCP_QUEUED_RECEIVED_BUFFER_USAGE:
            *r_type = XNetworkingStatisticsType::TitleTcpQueuedReceivedBufferUsage;
            return true;
        case XboxNetworking::STATISTICS_TYPE_SYSTEM_TCP_QUEUED_RECEIVED_BUFFER_USAGE:
            *r_type = XNetworkingStatisticsType::SystemTcpQueuedReceivedBufferUsage;
            return true;
        case XboxNetworking::STATISTICS_TYPE_TOOLS_TCP_QUEUED_RECEIVED_BUFFER_USAGE:
            *r_type = XNetworkingStatisticsType::ToolsTcpQueuedReceivedBufferUsage;
            return true;
        default:
            return false;
    }
}

// XNetworkingQueryPreferredLocalUdpMultiplayerPortAsync -> ...AsyncResult.
class QueryPreferredPortAsyncContext final : public XboxSignalXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(XboxResult::cancelled("Preferred local UDP multiplayer port request cancelled."));
            return;
        }

        uint16_t port = 0;
        const HRESULT result_hr = XNetworkingQueryPreferredLocalUdpMultiplayerPortAsyncResult(p_async_block, &port);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(XboxResult::cancelled("Preferred local UDP multiplayer port request cancelled."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(XboxResult::hresult_error(
                    result_hr,
                    "Failed to query the preferred local UDP multiplayer port.",
                    "preferred_local_udp_multiplayer_port_failed"));
            return;
        }

        Dictionary data;
        data["port"] = static_cast<int64_t>(port);
        get_pending_signal()->complete(XboxResult::ok_result(data));
    }

public:
    using XboxSignalXAsyncContext::XboxSignalXAsyncContext;
};

// XNetworkingQuerySecurityInformationForUrlAsync -> ...AsyncResultSize -> ...AsyncResult.
class QuerySecurityInformationAsyncContext final : public XboxSignalXAsyncContext {
    CharString m_url_utf8;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(XboxResult::cancelled("Security information request cancelled."));
            return;
        }

        size_t buffer_size = 0;
        HRESULT size_hr = XNetworkingQuerySecurityInformationForUrlAsyncResultSize(p_async_block, &buffer_size);
        if (size_hr == E_ABORT) {
            get_pending_signal()->complete(XboxResult::cancelled("Security information request cancelled."));
            return;
        }
        if (FAILED(size_hr)) {
            get_pending_signal()->complete(XboxResult::hresult_error(
                    size_hr,
                    "Failed to size the security information result.",
                    "security_information_size_failed"));
            return;
        }
        if (buffer_size < sizeof(XNetworkingSecurityInformation)) {
            get_pending_signal()->complete(XboxResult::error_result(
                    E_UNEXPECTED,
                    "security_information_size_invalid",
                    "The security information result size is too small to hold a result."));
            return;
        }

        // The native struct's pointers point into this buffer, so it must stay
        // at a fixed address for as long as the wrapper is alive.
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[buffer_size]());
        size_t bytes_used = 0;
        XNetworkingSecurityInformation *native = nullptr;
        HRESULT result_hr = XNetworkingQuerySecurityInformationForUrlAsyncResult(
                p_async_block,
                buffer_size,
                &bytes_used,
                buffer.get(),
                &native);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(XboxResult::cancelled("Security information request cancelled."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(XboxResult::hresult_error(
                    result_hr,
                    "Failed to read the security information result.",
                    "security_information_failed"));
            return;
        }
        if (native == nullptr) {
            get_pending_signal()->complete(XboxResult::error_result(
                    E_UNEXPECTED,
                    "security_information_empty",
                    "The GDK reported success but returned no security information."));
            return;
        }

        Ref<XboxNetworkingSecurityInformation> info;
        info.instantiate();
        info->set_native_internal(std::move(buffer), buffer_size, native);
        get_pending_signal()->complete(XboxResult::ok_result(info));
    }

public:
    QuerySecurityInformationAsyncContext(
            XboxRuntime *p_runtime,
            const Ref<XboxPendingSignal> &p_pending_signal,
            const String &p_url) :
            XboxSignalXAsyncContext(p_runtime, p_pending_signal),
            m_url_utf8(p_url.utf8()) {}

    const char *get_url() const {
        return m_url_utf8.get_data();
    }
};

} // namespace

// ─── XboxNetworkingSecurityInformation ────────────────────────────────────

void XboxNetworkingSecurityInformation::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("get_enabled_http_security_protocol_flags"),
            &XboxNetworkingSecurityInformation::get_enabled_http_security_protocol_flags);
    ClassDB::bind_method(D_METHOD("get_thumbprints"), &XboxNetworkingSecurityInformation::get_thumbprints);
    ClassDB::bind_method(D_METHOD("to_dictionary"), &XboxNetworkingSecurityInformation::to_dictionary);
}

int64_t XboxNetworkingSecurityInformation::get_enabled_http_security_protocol_flags() const {
    return m_enabled_http_security_protocol_flags;
}

Array XboxNetworkingSecurityInformation::get_thumbprints() const {
    return m_thumbprints;
}

Dictionary XboxNetworkingSecurityInformation::to_dictionary() const {
    Dictionary data;
    data["enabled_http_security_protocol_flags"] = m_enabled_http_security_protocol_flags;
    data["thumbprints"] = m_thumbprints;
    return data;
}

void XboxNetworkingSecurityInformation::set_native_internal(
        std::unique_ptr<uint8_t[]> p_buffer,
        size_t p_buffer_size,
        const XNetworkingSecurityInformation *p_native) {
    m_buffer = std::move(p_buffer);
    m_buffer_size = p_buffer_size;
    m_native = p_native;
    m_enabled_http_security_protocol_flags = 0;
    m_thumbprints.clear();

    if (m_native == nullptr) {
        return;
    }

    m_enabled_http_security_protocol_flags = static_cast<int64_t>(m_native->enabledHttpSecurityProtocolFlags);
    if (m_native->thumbprints == nullptr) {
        return;
    }

    for (size_t i = 0; i < m_native->thumbprintCount; ++i) {
        const XNetworkingThumbprint &thumbprint = m_native->thumbprints[i];

        PackedByteArray bytes;
        if (thumbprint.thumbprintBuffer != nullptr && thumbprint.thumbprintBufferByteCount > 0) {
            bytes.resize(static_cast<int64_t>(thumbprint.thumbprintBufferByteCount));
            memcpy(bytes.ptrw(), thumbprint.thumbprintBuffer, thumbprint.thumbprintBufferByteCount);
        }

        Dictionary entry;
        entry["type"] = static_cast<int64_t>(thumbprint.thumbprintType);
        entry["type_name"] = _thumbprint_type_name(thumbprint.thumbprintType);
        entry["bytes"] = bytes;
        entry["hex"] = _to_hex(thumbprint.thumbprintBuffer, thumbprint.thumbprintBufferByteCount);
        m_thumbprints.push_back(entry);
    }
}

// ─── XboxNetworking ───────────────────────────────────────────────────────

void XboxNetworking::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("query_preferred_local_udp_multiplayer_port"),
            &XboxNetworking::query_preferred_local_udp_multiplayer_port);
    ClassDB::bind_method(
            D_METHOD("query_preferred_local_udp_multiplayer_port_async"),
            &XboxNetworking::query_preferred_local_udp_multiplayer_port_async);
    ClassDB::bind_method(D_METHOD("get_connectivity_hint"), &XboxNetworking::get_connectivity_hint);
    ClassDB::bind_method(
            D_METHOD("query_security_information_for_url_async", "url"),
            &XboxNetworking::query_security_information_for_url_async);
    ClassDB::bind_method(
            D_METHOD("query_configuration_setting", "setting"),
            &XboxNetworking::query_configuration_setting);
    ClassDB::bind_method(
            D_METHOD("set_configuration_setting", "setting", "value"),
            &XboxNetworking::set_configuration_setting);
    ClassDB::bind_method(D_METHOD("query_statistics", "statistics_type"), &XboxNetworking::query_statistics);

    ADD_SIGNAL(MethodInfo("preferred_local_udp_multiplayer_port_changed", PropertyInfo(Variant::INT, "port")));
    ADD_SIGNAL(MethodInfo("connectivity_hint_changed", PropertyInfo(Variant::DICTIONARY, "hint")));

    BIND_ENUM_CONSTANT(CONNECTIVITY_LEVEL_UNKNOWN);
    BIND_ENUM_CONSTANT(CONNECTIVITY_LEVEL_NONE);
    BIND_ENUM_CONSTANT(CONNECTIVITY_LEVEL_LOCAL_ACCESS);
    BIND_ENUM_CONSTANT(CONNECTIVITY_LEVEL_INTERNET_ACCESS);
    BIND_ENUM_CONSTANT(CONNECTIVITY_LEVEL_CONSTRAINED_INTERNET_ACCESS);

    BIND_ENUM_CONSTANT(CONNECTIVITY_COST_UNKNOWN);
    BIND_ENUM_CONSTANT(CONNECTIVITY_COST_UNRESTRICTED);
    BIND_ENUM_CONSTANT(CONNECTIVITY_COST_FIXED);
    BIND_ENUM_CONSTANT(CONNECTIVITY_COST_VARIABLE);

    BIND_ENUM_CONSTANT(THUMBPRINT_TYPE_LEAF);
    BIND_ENUM_CONSTANT(THUMBPRINT_TYPE_ISSUER);
    BIND_ENUM_CONSTANT(THUMBPRINT_TYPE_ROOT);

    BIND_ENUM_CONSTANT(CONFIGURATION_SETTING_MAX_TITLE_TCP_QUEUED_RECEIVE_BUFFER_SIZE);
    BIND_ENUM_CONSTANT(CONFIGURATION_SETTING_MAX_SYSTEM_TCP_QUEUED_RECEIVE_BUFFER_SIZE);
    BIND_ENUM_CONSTANT(CONFIGURATION_SETTING_MAX_TOOLS_TCP_QUEUED_RECEIVE_BUFFER_SIZE);

    BIND_ENUM_CONSTANT(STATISTICS_TYPE_TITLE_TCP_QUEUED_RECEIVED_BUFFER_USAGE);
    BIND_ENUM_CONSTANT(STATISTICS_TYPE_SYSTEM_TCP_QUEUED_RECEIVED_BUFFER_USAGE);
    BIND_ENUM_CONSTANT(STATISTICS_TYPE_TOOLS_TCP_QUEUED_RECEIVED_BUFFER_USAGE);
}

void XboxNetworking::set_owner(Xbox *p_owner) {
    m_owner = p_owner;
}

XboxRuntime *XboxNetworking::_get_runtime() const {
    return m_owner == nullptr ? nullptr : m_owner->get_runtime();
}

Signal XboxNetworking::_make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) const {
    XboxRuntime *runtime = _get_runtime();
    if (runtime != nullptr) {
        return runtime->make_error_signal(p_hresult, p_code, p_message);
    }

    Ref<XboxPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(XboxResult::error_result(p_hresult, p_code, p_message));
    return pending_signal->get_completed_signal();
}

Dictionary XboxNetworking::make_connectivity_hint_dictionary(const XNetworkingConnectivityHint &p_hint) {
    Dictionary hint;
    hint["connectivity_level"] = static_cast<int64_t>(p_hint.connectivityLevel);
    hint["connectivity_level_name"] = _connectivity_level_name(p_hint.connectivityLevel);
    hint["connectivity_cost"] = static_cast<int64_t>(p_hint.connectivityCost);
    hint["connectivity_cost_name"] = _connectivity_cost_name(p_hint.connectivityCost);
    hint["iana_interface_type"] = static_cast<int64_t>(p_hint.ianaInterfaceType);
    hint["network_initialized"] = p_hint.networkInitialized;
    hint["approaching_data_limit"] = p_hint.approachingDataLimit;
    hint["over_data_limit"] = p_hint.overDataLimit;
    hint["roaming"] = p_hint.roaming;
    return hint;
}

Ref<XboxResult> XboxNetworking::on_runtime_initialized() {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return XboxResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the networking service before the GDK runtime.");
    }

    m_runtime_ready = true;

    // Both registrations fire an initial callback immediately, so the first
    // preferred_local_udp_multiplayer_port_changed / connectivity_hint_changed
    // emission carries the current state rather than a change. They are queued
    // on the shared task queue's completion port and therefore arrive on the
    // main thread during GDK.dispatch(), like every other GDK callback.
    //
    // Registration is an optional inbound-event listener: a failure disables the
    // signals but leaves every query method working, so it degrades with a
    // warning instead of failing GDK.initialize().
    XTaskQueueHandle queue = runtime->get_task_queue();

    if (!m_port_changed_registered) {
        HRESULT hr = XNetworkingRegisterPreferredLocalUdpMultiplayerPortChanged(
                queue,
                this,
                _port_changed_callback,
                &m_port_changed_token);
        if (SUCCEEDED(hr)) {
            m_port_changed_registered = true;
        } else {
            m_port_changed_token = {};
            char hr_buf[16];
            std::snprintf(hr_buf, sizeof(hr_buf), "0x%08X", static_cast<unsigned int>(hr));
            UtilityFunctions::push_warning(
                    String("[GDK] XNetworkingRegisterPreferredLocalUdpMultiplayerPortChanged failed (HRESULT ") +
                    String(hr_buf) +
                    ") — preferred_local_udp_multiplayer_port_changed will not fire.");
        }
    }

    if (!m_connectivity_changed_registered) {
        HRESULT hr = XNetworkingRegisterConnectivityHintChanged(
                queue,
                this,
                _connectivity_hint_changed_callback,
                &m_connectivity_changed_token);
        if (SUCCEEDED(hr)) {
            m_connectivity_changed_registered = true;
        } else {
            m_connectivity_changed_token = {};
            char hr_buf[16];
            std::snprintf(hr_buf, sizeof(hr_buf), "0x%08X", static_cast<unsigned int>(hr));
            UtilityFunctions::push_warning(
                    String("[GDK] XNetworkingRegisterConnectivityHintChanged failed (HRESULT ") +
                    String(hr_buf) +
                    ") — connectivity_hint_changed will not fire.");
        }
    }

    return XboxResult::ok_result();
}

void XboxNetworking::shutdown() {
    m_runtime_ready = false;

    if (m_port_changed_registered) {
        XNetworkingUnregisterPreferredLocalUdpMultiplayerPortChanged(m_port_changed_token, true);
        m_port_changed_token = {};
        m_port_changed_registered = false;
    }

    if (m_connectivity_changed_registered) {
        XNetworkingUnregisterConnectivityHintChanged(m_connectivity_changed_token, true);
        m_connectivity_changed_token = {};
        m_connectivity_changed_registered = false;
    }
}

void XboxNetworking::_port_changed_callback(void *p_context, uint16_t p_port) {
    auto *service = static_cast<XboxNetworking *>(p_context);
    if (service != nullptr) {
        service->handle_port_changed_internal(p_port);
    }
}

void XboxNetworking::_connectivity_hint_changed_callback(void *p_context, const XNetworkingConnectivityHint *p_hint) {
    auto *service = static_cast<XboxNetworking *>(p_context);
    if (service != nullptr) {
        service->handle_connectivity_hint_changed_internal(p_hint);
    }
}

void XboxNetworking::handle_port_changed_internal(uint16_t p_port) {
    if (!m_runtime_ready) {
        return;
    }
    emit_signal("preferred_local_udp_multiplayer_port_changed", static_cast<int64_t>(p_port));
}

void XboxNetworking::handle_connectivity_hint_changed_internal(const XNetworkingConnectivityHint *p_hint) {
    if (!m_runtime_ready || p_hint == nullptr) {
        return;
    }
    emit_signal("connectivity_hint_changed", make_connectivity_hint_dictionary(*p_hint));
}

Ref<XboxResult> XboxNetworking::query_preferred_local_udp_multiplayer_port() {
    if (!m_runtime_ready) {
        return XboxResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    uint16_t port = 0;
    const HRESULT hr = XNetworkingQueryPreferredLocalUdpMultiplayerPort(&port);
    if (FAILED(hr)) {
        return XboxResult::hresult_error(
                hr,
                "Failed to query the preferred local UDP multiplayer port.",
                "preferred_local_udp_multiplayer_port_failed");
    }

    Dictionary data;
    data["port"] = static_cast<int64_t>(port);
    return XboxResult::ok_result(data);
}

Signal XboxNetworking::query_preferred_local_udp_multiplayer_port_async() {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return _make_error_signal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    Ref<XboxPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new QueryPreferredPortAsyncContext(runtime, pending_signal);
    context->bind_cancel_handler();

    const HRESULT hr = XNetworkingQueryPreferredLocalUdpMultiplayerPortAsync(context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(XboxResult::hresult_error(
                hr,
                "Failed to start the preferred local UDP multiplayer port request.",
                "preferred_local_udp_multiplayer_port_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Ref<XboxResult> XboxNetworking::get_connectivity_hint() {
    if (!m_runtime_ready) {
        return XboxResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    XNetworkingConnectivityHint hint = {};
    const HRESULT hr = XNetworkingGetConnectivityHint(&hint);
    if (FAILED(hr)) {
        return XboxResult::hresult_error(
                hr,
                "Failed to query the connectivity hint.",
                "connectivity_hint_failed");
    }

    return XboxResult::ok_result(make_connectivity_hint_dictionary(hint));
}

Signal XboxNetworking::query_security_information_for_url_async(const String &p_url) {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return _make_error_signal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (p_url.strip_edges().is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_url", "A non-empty URL is required to query security information.");
    }

    Ref<XboxPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new QuerySecurityInformationAsyncContext(runtime, pending_signal, p_url);
    context->bind_cancel_handler();

    const HRESULT hr = XNetworkingQuerySecurityInformationForUrlAsync(context->get_url(), context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(XboxResult::hresult_error(
                hr,
                "Failed to start the security information request.",
                "security_information_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Ref<XboxResult> XboxNetworking::query_configuration_setting(int64_t p_setting) {
    if (!m_runtime_ready) {
        return XboxResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    XNetworkingConfigurationSetting setting;
    if (!_to_native_configuration_setting(p_setting, &setting)) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_configuration_setting",
                "Unknown networking configuration setting value.");
    }

    uint64_t value = 0;
    const HRESULT hr = XNetworkingQueryConfigurationSetting(setting, &value);
    if (FAILED(hr)) {
        return XboxResult::hresult_error(
                hr,
                "Failed to query the networking configuration setting.",
                "configuration_setting_query_failed");
    }

    // uint64 does not round-trip through GDScript's signed int, and UINT64_MAX
    // is the documented "unlimited" default for some settings on console.
    const bool unlimited = value == UINT64_MAX;
    Dictionary data;
    data["value"] = static_cast<int64_t>(value);
    data["unlimited"] = unlimited;
    return XboxResult::ok_result(data);
}

Ref<XboxResult> XboxNetworking::set_configuration_setting(int64_t p_setting, int64_t p_value) {
    if (!m_runtime_ready) {
        return XboxResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    XNetworkingConfigurationSetting setting;
    if (!_to_native_configuration_setting(p_setting, &setting)) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_configuration_setting",
                "Unknown networking configuration setting value.");
    }
    if (p_value < 0) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_configuration_value",
                "Networking configuration values must be zero or greater.");
    }

    const HRESULT hr = XNetworkingSetConfigurationSetting(setting, static_cast<uint64_t>(p_value));
    if (hr == E_NOTIMPL) {
        return XboxResult::error_result(
                hr,
                "not_supported_on_platform",
                "Networking configuration settings are not implemented on this platform.");
    }
    if (FAILED(hr)) {
        return XboxResult::hresult_error(
                hr,
                "Failed to set the networking configuration setting.",
                "configuration_setting_set_failed");
    }

    Dictionary data;
    data["setting"] = p_setting;
    data["value"] = p_value;
    return XboxResult::ok_result(data);
}

Ref<XboxResult> XboxNetworking::query_statistics(int64_t p_statistics_type) {
    if (!m_runtime_ready) {
        return XboxResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    XNetworkingStatisticsType statistics_type;
    if (!_to_native_statistics_type(p_statistics_type, &statistics_type)) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_statistics_type",
                "Unknown networking statistics type value.");
    }

    XNetworkingStatisticsBuffer buffer = {};
    const HRESULT hr = XNetworkingQueryStatistics(statistics_type, &buffer);
    if (FAILED(hr)) {
        return XboxResult::hresult_error(
                hr,
                "Failed to query networking statistics.",
                "statistics_query_failed");
    }

    const XNetworkingTcpQueuedReceivedBufferUsageStatistics &usage = buffer.tcpQueuedReceiveBufferUsage;
    Dictionary data;
    data["num_bytes_currently_queued"] = static_cast<int64_t>(usage.numBytesCurrentlyQueued);
    data["peak_num_bytes_ever_queued"] = static_cast<int64_t>(usage.peakNumBytesEverQueued);
    data["total_num_bytes_queued"] = static_cast<int64_t>(usage.totalNumBytesQueued);
    data["num_bytes_dropped_for_exceeding_configured_max"] = static_cast<int64_t>(usage.numBytesDroppedForExceedingConfiguredMax);
    data["num_bytes_dropped_due_to_any_failure"] = static_cast<int64_t>(usage.numBytesDroppedDueToAnyFailure);
    return XboxResult::ok_result(data);
}

} // namespace godot
