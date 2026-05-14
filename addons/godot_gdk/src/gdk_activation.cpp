#include "gdk_activation.h"

#include <cstdio>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "gdk.h"
#include "gdk_result.h"
#include "gdk_runtime.h"

namespace godot {

namespace {

String _utf8_or_empty(const char *p_value) {
    if (p_value == nullptr) {
        return String();
    }
    return String::utf8(p_value);
}

} // namespace

void GDKActivation::_bind_methods() {
    ClassDB::bind_method(D_METHOD("accept_pending_invite", "invite_uri"), &GDKActivation::accept_pending_invite);

    ADD_SIGNAL(MethodInfo("protocol_activated", PropertyInfo(Variant::STRING, "uri")));
    ADD_SIGNAL(MethodInfo("file_activated", PropertyInfo(Variant::STRING, "file")));
    ADD_SIGNAL(MethodInfo("pending_invite_received", PropertyInfo(Variant::STRING, "invite_uri")));
    ADD_SIGNAL(MethodInfo("invite_accepted", PropertyInfo(Variant::STRING, "invite_uri")));
    ADD_SIGNAL(MethodInfo("activated", PropertyInfo(Variant::DICTIONARY, "info")));

    BIND_ENUM_CONSTANT(ACTIVATION_TYPE_PROTOCOL);
    BIND_ENUM_CONSTANT(ACTIVATION_TYPE_FILE);
    BIND_ENUM_CONSTANT(ACTIVATION_TYPE_PENDING_GAME_INVITE);
    BIND_ENUM_CONSTANT(ACTIVATION_TYPE_ACCEPTED_GAME_INVITE);
}

void GDKActivation::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKActivation::on_runtime_initialized() {
    GDKRuntime *runtime = m_owner != nullptr ? m_owner->get_runtime() : nullptr;
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the activation service before the GDK runtime.");
    }

    if (m_activation_registered) {
        m_runtime_ready = true;
        return GDKResult::ok_result();
    }

    HRESULT hr = XGameActivationRegisterForEvent(
            runtime->get_task_queue(),
            this,
            _activation_callback,
            &m_activation_token);
    if (FAILED(hr)) {
        // Activation registration is an optional inbound-event listener.
        // On PC GDK with strict GamingServices builds it can return
        // ERROR_NOT_SUPPORTED (0x80070032) when the title is not running
        // inside a fully-registered package context (e.g. F5-from-editor
        // or partially-registered loose builds). Degrade gracefully so the
        // synchronous accept_pending_invite() path still works and the
        // entire GDK init does not fail.
        char hr_buf[16];
        std::snprintf(hr_buf, sizeof(hr_buf), "0x%08X", static_cast<unsigned int>(hr));
        UtilityFunctions::push_warning(
                String("[GDK] XGameActivationRegisterForEvent failed (HRESULT ") +
                String(hr_buf) +
                ") — activation inbound events disabled, accept_pending_invite still available.");
        m_activation_registered = false;
        m_runtime_ready = true;
        return GDKResult::ok_result();
    }

    m_runtime_ready = true;
    m_activation_registered = true;
    return GDKResult::ok_result();
}

void GDKActivation::shutdown() {
    m_runtime_ready = false;

    if (m_activation_registered) {
        XGameActivationUnregisterForEvent(m_activation_token, true);
        m_activation_registered = false;
        m_activation_token = {};
    }
}

Ref<GDKResult> GDKActivation::accept_pending_invite(const String &p_invite_uri) {
    if (!m_runtime_ready) {
        return GDKResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    const String invite_uri = p_invite_uri.strip_edges();
    if (invite_uri.is_empty()) {
        return GDKResult::error_result(
                E_INVALIDARG,
                "invalid_invite_uri",
                "A non-empty invite URI is required.");
    }

    const CharString invite_uri_utf8 = invite_uri.utf8();
    HRESULT hr = XGameActivationAcceptPendingInvite(invite_uri_utf8.get_data());
    if (FAILED(hr)) {
        Dictionary data;
        data["invite_uri"] = invite_uri;
        return GDKResult::hresult_error(
                hr,
                "Failed to accept pending invite.",
                "accept_pending_invite_failed",
                data);
    }

    Dictionary data;
    data["invite_uri"] = invite_uri;
    return GDKResult::ok_result(data);
}

void GDKActivation::handle_activation_internal(const XGameActivationInfo *p_activation_info) {
    if (p_activation_info == nullptr) {
        return;
    }

    Dictionary info;
    info["type"] = static_cast<int64_t>(p_activation_info->type);

    switch (p_activation_info->type) {
        case XGameActivationType::Protocol: {
            const String uri = _utf8_or_empty(p_activation_info->protocolUri);
            info["uri"] = uri;
            emit_signal("protocol_activated", uri);
            break;
        }
        case XGameActivationType::File: {
            const String file = _utf8_or_empty(p_activation_info->file);
            info["file"] = file;
            emit_signal("file_activated", file);
            break;
        }
        case XGameActivationType::PendingGameInvite: {
            const String invite_uri = _utf8_or_empty(p_activation_info->inviteUri);
            info["invite_uri"] = invite_uri;
            emit_signal("pending_invite_received", invite_uri);
            break;
        }
        case XGameActivationType::AcceptedGameInvite: {
            const String invite_uri = _utf8_or_empty(p_activation_info->inviteUri);
            info["invite_uri"] = invite_uri;
            emit_signal("invite_accepted", invite_uri);
            break;
        }
        default:
            break;
    }

    emit_signal("activated", info);
}

void CALLBACK GDKActivation::_activation_callback(void *p_context, const XGameActivationInfo *p_activation_info) {
    auto *service = static_cast<GDKActivation *>(p_context);
    if (service != nullptr) {
        service->handle_activation_internal(p_activation_info);
    }
}

} // namespace godot
