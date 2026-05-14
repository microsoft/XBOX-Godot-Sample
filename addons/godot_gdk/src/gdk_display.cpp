#include "gdk_display.h"

#include <godot_cpp/variant/dictionary.hpp>

#include "gdk.h"
#include "gdk_result.h"
#include "gdk_runtime.h"

namespace godot {

// ─── GDKDisplayTimeoutDeferral ───────────────────────────────────────────

void GDKDisplayTimeoutDeferral::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &GDKDisplayTimeoutDeferral::is_valid);
    ClassDB::bind_method(D_METHOD("release"), &GDKDisplayTimeoutDeferral::release);
}

GDKDisplayTimeoutDeferral::~GDKDisplayTimeoutDeferral() {
    release();
}

bool GDKDisplayTimeoutDeferral::is_valid() const {
    return m_handle != nullptr;
}

void GDKDisplayTimeoutDeferral::release() {
    if (m_handle != nullptr) {
        XDisplayCloseTimeoutDeferralHandle(m_handle);
        m_handle = nullptr;
    }
}

void GDKDisplayTimeoutDeferral::set_handle_internal(XDisplayTimeoutDeferralHandle p_handle) {
    if (m_handle == p_handle) {
        return;
    }
    release();
    m_handle = p_handle;
}

// ─── GDKDisplay ──────────────────────────────────────────────────────────

void GDKDisplay::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("try_enable_hdr_mode", "preference"),
            &GDKDisplay::try_enable_hdr_mode,
            DEFVAL(static_cast<int64_t>(HDR_MODE_PREFERENCE_PREFER_HDR)));
    ClassDB::bind_method(D_METHOD("acquire_timeout_deferral"), &GDKDisplay::acquire_timeout_deferral);

    BIND_ENUM_CONSTANT(HDR_MODE_UNKNOWN);
    BIND_ENUM_CONSTANT(HDR_MODE_ENABLED);
    BIND_ENUM_CONSTANT(HDR_MODE_DISABLED);

    BIND_ENUM_CONSTANT(HDR_MODE_PREFERENCE_PREFER_HDR);
    BIND_ENUM_CONSTANT(HDR_MODE_PREFERENCE_PREFER_REFRESH_RATE);
}

void GDKDisplay::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

GDKRuntime *GDKDisplay::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

Ref<GDKResult> GDKDisplay::on_runtime_initialized() {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the display service before the GDK runtime.");
    }
    m_runtime_ready = true;
    return GDKResult::ok_result();
}

void GDKDisplay::shutdown() {
    m_runtime_ready = false;
}

Ref<GDKResult> GDKDisplay::try_enable_hdr_mode(int64_t p_preference) {
    if (!m_runtime_ready) {
        return GDKResult::error_result(
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
            return GDKResult::error_result(
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
    return GDKResult::ok_result(data);
}

Ref<GDKResult> GDKDisplay::acquire_timeout_deferral() {
    if (!m_runtime_ready) {
        return GDKResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    XDisplayTimeoutDeferralHandle handle = nullptr;
    HRESULT hr = XDisplayAcquireTimeoutDeferral(&handle);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(
                hr,
                "Failed to acquire display timeout deferral.",
                "acquire_timeout_deferral_failed");
    }

    Ref<GDKDisplayTimeoutDeferral> deferral;
    deferral.instantiate();
    deferral->set_handle_internal(handle);
    return GDKResult::ok_result(deferral);
}

} // namespace godot
