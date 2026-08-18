#include "xbox_display.h"

#include <godot_cpp/variant/dictionary.hpp>

#include "xbox.h"
#include "xbox_result.h"
#include "xbox_runtime.h"

namespace godot {

// ─── XboxDisplayTimeoutDeferral ───────────────────────────────────────────

void XboxDisplayTimeoutDeferral::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &XboxDisplayTimeoutDeferral::is_valid);
    ClassDB::bind_method(D_METHOD("release"), &XboxDisplayTimeoutDeferral::release);
}

XboxDisplayTimeoutDeferral::~XboxDisplayTimeoutDeferral() {
    release();
}

bool XboxDisplayTimeoutDeferral::is_valid() const {
    return m_handle != nullptr;
}

void XboxDisplayTimeoutDeferral::release() {
    if (m_handle != nullptr) {
        XDisplayCloseTimeoutDeferralHandle(m_handle);
        m_handle = nullptr;
    }
}

void XboxDisplayTimeoutDeferral::set_handle_internal(XDisplayTimeoutDeferralHandle p_handle) {
    if (m_handle == p_handle) {
        return;
    }
    release();
    m_handle = p_handle;
}

// ─── XboxDisplay ──────────────────────────────────────────────────────────

void XboxDisplay::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("try_enable_hdr_mode", "preference"),
            &XboxDisplay::try_enable_hdr_mode,
            DEFVAL(static_cast<int64_t>(HDR_MODE_PREFERENCE_PREFER_HDR)));
    ClassDB::bind_method(D_METHOD("acquire_timeout_deferral"), &XboxDisplay::acquire_timeout_deferral);

    BIND_ENUM_CONSTANT(HDR_MODE_UNKNOWN);
    BIND_ENUM_CONSTANT(HDR_MODE_ENABLED);
    BIND_ENUM_CONSTANT(HDR_MODE_DISABLED);

    BIND_ENUM_CONSTANT(HDR_MODE_PREFERENCE_PREFER_HDR);
    BIND_ENUM_CONSTANT(HDR_MODE_PREFERENCE_PREFER_REFRESH_RATE);
}

void XboxDisplay::set_owner(Xbox *p_owner) {
    m_owner = p_owner;
}

XboxRuntime *XboxDisplay::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

Ref<XboxResult> XboxDisplay::on_runtime_initialized() {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return XboxResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the display service before the GDK runtime.");
    }
    m_runtime_ready = true;
    return XboxResult::ok_result();
}

void XboxDisplay::shutdown() {
    m_runtime_ready = false;
}

Ref<XboxResult> XboxDisplay::try_enable_hdr_mode(int64_t p_preference) {
    if (!m_runtime_ready) {
        return XboxResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    XDisplayHdrModePreference native_preference;
    switch (p_preference) {
        case HDR_MODE_PREFERENCE_PREFER_HDR:
            native_preference = XDisplayHdrModePreference::PreferHdr;
            break;
        case HDR_MODE_PREFERENCE_PREFER_REFRESH_RATE:
            native_preference = XDisplayHdrModePreference::PreferRefreshRate;
            break;
        default:
            return XboxResult::error_result(
                    E_INVALIDARG,
                    "invalid_preference",
                    "Unknown HDR mode preference value.");
    }

    XDisplayHdrModeInfo info_native = {};
    const XDisplayHdrModeResult mode_result = XDisplayTryEnableHdrMode(native_preference, &info_native);

    Dictionary data;
    data["mode"] = static_cast<int64_t>(mode_result);
    if (mode_result == XDisplayHdrModeResult::Enabled) {
        Dictionary info;
        info["min_tone_map_luminance"] = static_cast<double>(info_native.minToneMapLuminance);
        info["max_tone_map_luminance"] = static_cast<double>(info_native.maxToneMapLuminance);
        info["max_full_frame_tone_map_luminance"] = static_cast<double>(info_native.maxFullFrameToneMapLuminance);
        data["info"] = info;
    }
    return XboxResult::ok_result(data);
}

Ref<XboxResult> XboxDisplay::acquire_timeout_deferral() {
    if (!m_runtime_ready) {
        return XboxResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    XDisplayTimeoutDeferralHandle handle = nullptr;
    HRESULT hr = XDisplayAcquireTimeoutDeferral(&handle);
    if (FAILED(hr)) {
        return XboxResult::hresult_error(
                hr,
                "Failed to acquire display timeout deferral.",
                "acquire_timeout_deferral_failed");
    }

    Ref<XboxDisplayTimeoutDeferral> deferral;
    deferral.instantiate();
    deferral->set_handle_internal(handle);
    return XboxResult::ok_result(deferral);
}

} // namespace godot
