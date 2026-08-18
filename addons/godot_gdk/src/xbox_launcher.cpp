#include "xbox_launcher.h"

#include "xbox.h"
#include "xbox_result.h"
#include "xbox_runtime.h"
#include "xbox_user.h"

namespace godot {

void XboxLauncher::_bind_methods() {
    ClassDB::bind_method(D_METHOD("launch_uri", "uri", "user"), &XboxLauncher::launch_uri, DEFVAL(Ref<XboxUser>()));
}

void XboxLauncher::set_owner(Xbox *p_owner) {
    m_owner = p_owner;
}

Ref<XboxResult> XboxLauncher::on_runtime_initialized() {
    XboxRuntime *runtime = m_owner != nullptr ? m_owner->get_runtime() : nullptr;
    if (runtime == nullptr || !runtime->is_initialized()) {
        return XboxResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the launcher service before the GDK runtime.");
    }

    m_runtime_ready = true;
    return XboxResult::ok_result();
}

void XboxLauncher::shutdown() {
    m_runtime_ready = false;
}

Ref<XboxResult> XboxLauncher::launch_uri(const String &p_uri, const Ref<XboxUser> &p_user) {
    if (!m_runtime_ready) {
        return XboxResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    const String uri = p_uri.strip_edges();
    String scheme;
    if (!try_parse_uri_scheme_internal(uri, &scheme)) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_uri",
                "A non-empty absolute URI with a valid scheme is required.");
    }

    if (is_disallowed_scheme_internal(scheme)) {
        Dictionary data;
        data["uri"] = uri;
        data["scheme"] = scheme;
        return XboxResult::error_result(
                E_NOTIMPL,
                "unsupported_launcher_destination",
                "This URI destination is not supported by GDK.launcher on PC.",
                data);
    }

    if (!is_supported_scheme_internal(scheme)) {
        Dictionary data;
        data["uri"] = uri;
        data["scheme"] = scheme;
        return XboxResult::error_result(
                E_NOTIMPL,
                "unsupported_launcher_destination",
                "This URI destination is not supported by GDK.launcher on PC.",
                data);
    }

    XUserHandle user_handle = nullptr;
    if (p_user.is_valid()) {
        user_handle = p_user->get_handle();
        if (user_handle == nullptr) {
            return XboxResult::error_result(
                    E_INVALIDARG,
                    "invalid_user",
                    "A signed-in XboxUser is required when a user is provided.");
        }
    }

    const CharString uri_utf8 = uri.utf8();
    HRESULT hr = XLaunchUri(user_handle, uri_utf8.get_data());
    if (FAILED(hr)) {
        Dictionary data;
        data["uri"] = uri;
        data["destination"] = "uri";
        return XboxResult::hresult_error(hr, "Failed to launch the requested URI.", "launch_uri_failed", data);
    }

    Dictionary data;
    data["uri"] = uri;
    data["destination"] = "uri";
    return XboxResult::ok_result(data);
}

bool XboxLauncher::try_parse_uri_scheme_internal(const String &p_uri, String *r_scheme) {
    if (r_scheme == nullptr) {
        return false;
    }

    *r_scheme = String();

    const String uri = p_uri.strip_edges();
    if (uri.is_empty() || uri.contains(" ")) {
        return false;
    }

    const int64_t separator = uri.find(":");
    if (separator <= 0) {
        return false;
    }

    const String scheme = uri.substr(0, separator).to_lower();
    if (scheme.is_empty()) {
        return false;
    }

    *r_scheme = scheme;
    return true;
}

bool XboxLauncher::is_supported_scheme_internal(const String &p_scheme) {
    const String scheme = p_scheme.to_lower();
    if (scheme.begins_with("ms-")) {
        return scheme == "ms-settings" || scheme == "ms-windows-store";
    }
    return true;
}

bool XboxLauncher::is_disallowed_scheme_internal(const String &p_scheme) {
    const String scheme = p_scheme.to_lower();
    return scheme == "file" || scheme == "javascript" || scheme == "data" || scheme == "about";
}

} // namespace godot
