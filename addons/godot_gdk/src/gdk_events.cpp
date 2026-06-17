#include "gdk_events.h"

#include <array>
#include <cstdio>
#include <random>
#include <string>

#include <XUser.h>
#include <XGameEvent.h>

#include <godot_cpp/classes/json.hpp>

#include "gdk.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_user.h"
#include "gdk_xbox_services.h"

namespace godot {

namespace {

String _generate_play_session_id() {
    std::random_device device;
    std::mt19937_64 generator(((static_cast<uint64_t>(device()) << 32) ^ static_cast<uint64_t>(device())));
    std::uniform_int_distribution<uint32_t> byte_distribution(0, 255);

    std::array<uint8_t, 16> bytes = {};
    for (uint8_t &value : bytes) {
        value = static_cast<uint8_t>(byte_distribution(generator));
    }

    // RFC 4122 version 4 (random) UUID bits.
    bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
    bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

    char buffer[37] = {};
    std::snprintf(
            buffer,
            sizeof(buffer),
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            bytes[0], bytes[1], bytes[2], bytes[3],
            bytes[4], bytes[5], bytes[6], bytes[7],
            bytes[8], bytes[9], bytes[10], bytes[11],
            bytes[12], bytes[13], bytes[14], bytes[15]);
    return String::utf8(buffer);
}

String _stringify_json_fields(const Dictionary &p_fields) {
    if (p_fields.is_empty()) {
        return String();
    }
    return JSON::stringify(p_fields);
}

} // namespace

void GDKEvents::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("write_event", "user", "event_name", "dimensions", "measurements"),
            &GDKEvents::write_event,
            DEFVAL(Dictionary()),
            DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("set_play_session_id", "play_session_id"), &GDKEvents::set_play_session_id);
    ClassDB::bind_method(D_METHOD("get_play_session_id"), &GDKEvents::get_play_session_id);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "play_session_id"), "set_play_session_id", "get_play_session_id");
}

void GDKEvents::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKEvents::on_runtime_initialized() {
    m_runtime_ready = true;
    _ensure_play_session_id();
    return GDKResult::ok_result();
}

void GDKEvents::shutdown() {
    m_runtime_ready = false;
}

GDKRuntime *GDKEvents::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

GDKXboxServices *GDKEvents::_get_xbox_services() const {
    return m_owner != nullptr ? m_owner->get_xbox_services() : nullptr;
}

void GDKEvents::_ensure_play_session_id() {
    if (m_play_session_id.strip_edges().is_empty()) {
        m_play_session_id = _generate_play_session_id();
    }
}

Ref<GDKResult> GDKEvents::write_event(
        const Ref<GDKUser> &p_user,
        const String &p_event_name,
        const Dictionary &p_dimensions,
        const Dictionary &p_measurements) {
    const String event_name = p_event_name.strip_edges();
    if (event_name.is_empty()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_event_name", "A non-empty event name is required.");
    }

    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_available()) {
        return GDKResult::error_result(E_FAIL, "runtime_unavailable", "GDK runtime is unavailable in the current process.");
    }
    if (!m_runtime_ready || !runtime->is_initialized()) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    GDKXboxServices *xbox_services = _get_xbox_services();
    if (xbox_services == nullptr || !xbox_services->is_initialized()) {
        return GDKResult::error_result(E_FAIL, "xbox_services_uninitialized", "Xbox services are not initialized.");
    }
    const String scid = xbox_services->get_scid();
    if (scid.is_empty()) {
        return GDKResult::error_result(E_FAIL, "xbox_service_config_unavailable", "The Xbox Live service configuration ID (SCID) is unavailable.");
    }

    _ensure_play_session_id();

    const std::string scid_utf8 = scid.utf8().get_data();
    const std::string play_session_id_utf8 = m_play_session_id.utf8().get_data();
    const std::string event_name_utf8 = event_name.utf8().get_data();
    const String dimensions_json = _stringify_json_fields(p_dimensions);
    const String measurements_json = _stringify_json_fields(p_measurements);
    const std::string dimensions_utf8 = dimensions_json.utf8().get_data();
    const std::string measurements_utf8 = measurements_json.utf8().get_data();

    HRESULT hr = XGameEventWrite(
            p_user->get_handle(),
            scid_utf8.c_str(),
            play_session_id_utf8.c_str(),
            event_name_utf8.c_str(),
            dimensions_json.is_empty() ? nullptr : dimensions_utf8.c_str(),
            measurements_json.is_empty() ? nullptr : measurements_utf8.c_str());

    if (hr == E_NOTIMPL) {
        return GDKResult::error_result(
                hr,
                "events_feature_unavailable",
                "The GDK XGameEvent telemetry feature is unavailable in the current runtime. Events were not written.");
    }
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to write the in-game event.", "event_write_failed");
    }

    Dictionary data;
    data["event_name"] = event_name;
    data["play_session_id"] = m_play_session_id;
    return GDKResult::ok_result(data);
}

void GDKEvents::set_play_session_id(const String &p_play_session_id) {
    const String play_session_id = p_play_session_id.strip_edges();
    m_play_session_id = play_session_id.is_empty() ? _generate_play_session_id() : play_session_id;
}

String GDKEvents::get_play_session_id() {
    _ensure_play_session_id();
    return m_play_session_id;
}

} // namespace godot
