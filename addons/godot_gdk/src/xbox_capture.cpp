#include "xbox_capture.h"

#include <cmath>
#include <ctime>
#include <limits>

#include <godot_cpp/variant/utility_functions.hpp>

#include "xbox.h"
#include "xbox_pending_signal.h"
#include "xbox_result.h"
#include "xbox_runtime.h"

namespace godot {

// ── XboxCaptureMetaData ────────────────────────────────────────────────────

void XboxCaptureMetaData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &XboxCaptureMetaData::is_valid);
    ClassDB::bind_method(D_METHOD("close"), &XboxCaptureMetaData::close);
    ClassDB::bind_method(D_METHOD("stop_all_states"), &XboxCaptureMetaData::stop_all_states);
    ClassDB::bind_method(D_METHOD("get_remaining_storage_bytes"), &XboxCaptureMetaData::get_remaining_storage_bytes);
    ClassDB::bind_method(
            D_METHOD("add_string_event", "name", "value", "priority"),
            &XboxCaptureMetaData::add_string_event,
            DEFVAL(PRIORITY_GAMEPLAY));
    ClassDB::bind_method(
            D_METHOD("add_double_event", "name", "value", "priority"),
            &XboxCaptureMetaData::add_double_event,
            DEFVAL(PRIORITY_GAMEPLAY));
    ClassDB::bind_method(
            D_METHOD("add_int32_event", "name", "value", "priority"),
            &XboxCaptureMetaData::add_int32_event,
            DEFVAL(PRIORITY_GAMEPLAY));
    ClassDB::bind_method(
            D_METHOD("start_string_state", "name", "value", "priority"),
            &XboxCaptureMetaData::start_string_state,
            DEFVAL(PRIORITY_GAMEPLAY));
    ClassDB::bind_method(
            D_METHOD("start_double_state", "name", "value", "priority"),
            &XboxCaptureMetaData::start_double_state,
            DEFVAL(PRIORITY_GAMEPLAY));
    ClassDB::bind_method(
            D_METHOD("start_int32_state", "name", "value", "priority"),
            &XboxCaptureMetaData::start_int32_state,
            DEFVAL(PRIORITY_GAMEPLAY));

    BIND_ENUM_CONSTANT(PRIORITY_GAMEPLAY);
    BIND_ENUM_CONSTANT(PRIORITY_IMPORTANT);
}

XboxCaptureMetaData::~XboxCaptureMetaData() {
    close();
}

Ref<XboxResult> XboxCaptureMetaData::_check_valid() const {
    if (!m_valid) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_metadata_handle",
                "The capture metadata context is not valid. Call XboxCapture.create_metadata() first.");
    }
    return Ref<XboxResult>();
}

// static
int32_t XboxCaptureMetaData::_clamp_to_int32(int64_t p_value) {
    return static_cast<int32_t>(
            std::max<int64_t>(std::numeric_limits<int32_t>::min(),
            std::min<int64_t>(std::numeric_limits<int32_t>::max(), p_value)));
}

// static
XAppCaptureMetadataPriority XboxCaptureMetaData::_to_native_priority(int64_t p_priority) {
    if (p_priority == PRIORITY_IMPORTANT) {
        return XAppCaptureMetadataPriority::Important;
    }
    return XAppCaptureMetadataPriority::Informational;
}

// static
Ref<XboxResult> XboxCaptureMetaData::_wrap_hresult(HRESULT p_hr, const char *p_action, const char *p_code) {
    if (SUCCEEDED(p_hr)) {
        return XboxResult::ok_result();
    }
    return XboxResult::hresult_error(p_hr, p_action, p_code);
}

bool XboxCaptureMetaData::is_valid() const {
    return m_valid;
}

void XboxCaptureMetaData::close() {
    m_valid = false;
}

Ref<XboxResult> XboxCaptureMetaData::stop_all_states() {
    Ref<XboxResult> valid_check = _check_valid();
    if (valid_check.is_valid()) {
        return valid_check;
    }
    HRESULT hr = XAppCaptureMetadataStopAllStates();
    return _wrap_hresult(hr, "Failed to stop all metadata states.", "capture_metadata_stop_all_states_failed");
}

int64_t XboxCaptureMetaData::get_remaining_storage_bytes() const {
    if (!m_valid) {
        return -1;
    }
    uint64_t remaining = 0;
    HRESULT hr = XAppCaptureMetadataRemainingStorageBytesAvailable(&remaining);
    if (FAILED(hr)) {
        return -1;
    }
    return static_cast<int64_t>(remaining);
}

Ref<XboxResult> XboxCaptureMetaData::add_string_event(
        const String &p_name,
        const String &p_value,
        int64_t p_priority) {
    Ref<XboxResult> valid_check = _check_valid();
    if (valid_check.is_valid()) {
        return valid_check;
    }
    if (p_name.strip_edges().is_empty()) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_metadata_name",
                "Metadata event name must not be empty.");
    }
    CharString name_utf8 = p_name.utf8();
    CharString value_utf8 = p_value.utf8();
    HRESULT hr = XAppCaptureMetadataAddStringEvent(
            name_utf8.get_data(),
            value_utf8.get_data(),
            _to_native_priority(p_priority));
    return _wrap_hresult(hr, "Failed to add string event to capture metadata.", "capture_metadata_add_string_event_failed");
}

Ref<XboxResult> XboxCaptureMetaData::add_double_event(
        const String &p_name,
        double p_value,
        int64_t p_priority) {
    Ref<XboxResult> valid_check = _check_valid();
    if (valid_check.is_valid()) {
        return valid_check;
    }
    if (p_name.strip_edges().is_empty()) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_metadata_name",
                "Metadata event name must not be empty.");
    }
    CharString name_utf8 = p_name.utf8();
    HRESULT hr = XAppCaptureMetadataAddDoubleEvent(
            name_utf8.get_data(),
            p_value,
            _to_native_priority(p_priority));
    return _wrap_hresult(hr, "Failed to add double event to capture metadata.", "capture_metadata_add_double_event_failed");
}

Ref<XboxResult> XboxCaptureMetaData::add_int32_event(
        const String &p_name,
        int64_t p_value,
        int64_t p_priority) {
    Ref<XboxResult> valid_check = _check_valid();
    if (valid_check.is_valid()) {
        return valid_check;
    }
    if (p_name.strip_edges().is_empty()) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_metadata_name",
                "Metadata event name must not be empty.");
    }
    const int32_t clamped = _clamp_to_int32(p_value);
    CharString name_utf8 = p_name.utf8();
    HRESULT hr = XAppCaptureMetadataAddInt32Event(
            name_utf8.get_data(),
            clamped,
            _to_native_priority(p_priority));
    return _wrap_hresult(hr, "Failed to add int32 event to capture metadata.", "capture_metadata_add_int32_event_failed");
}

Ref<XboxResult> XboxCaptureMetaData::start_string_state(
        const String &p_name,
        const String &p_value,
        int64_t p_priority) {
    Ref<XboxResult> valid_check = _check_valid();
    if (valid_check.is_valid()) {
        return valid_check;
    }
    if (p_name.strip_edges().is_empty()) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_metadata_name",
                "Metadata state name must not be empty.");
    }
    CharString name_utf8 = p_name.utf8();
    CharString value_utf8 = p_value.utf8();
    HRESULT hr = XAppCaptureMetadataStartStringState(
            name_utf8.get_data(),
            value_utf8.get_data(),
            _to_native_priority(p_priority));
    return _wrap_hresult(hr, "Failed to start string state in capture metadata.", "capture_metadata_start_string_state_failed");
}

Ref<XboxResult> XboxCaptureMetaData::start_double_state(
        const String &p_name,
        double p_value,
        int64_t p_priority) {
    Ref<XboxResult> valid_check = _check_valid();
    if (valid_check.is_valid()) {
        return valid_check;
    }
    if (p_name.strip_edges().is_empty()) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_metadata_name",
                "Metadata state name must not be empty.");
    }
    CharString name_utf8 = p_name.utf8();
    HRESULT hr = XAppCaptureMetadataStartDoubleState(
            name_utf8.get_data(),
            p_value,
            _to_native_priority(p_priority));
    return _wrap_hresult(hr, "Failed to start double state in capture metadata.", "capture_metadata_start_double_state_failed");
}

Ref<XboxResult> XboxCaptureMetaData::start_int32_state(
        const String &p_name,
        int64_t p_value,
        int64_t p_priority) {
    Ref<XboxResult> valid_check = _check_valid();
    if (valid_check.is_valid()) {
        return valid_check;
    }
    if (p_name.strip_edges().is_empty()) {
        return XboxResult::error_result(
                E_INVALIDARG,
                "invalid_metadata_name",
                "Metadata state name must not be empty.");
    }
    const int32_t clamped = _clamp_to_int32(p_value);
    CharString name_utf8 = p_name.utf8();
    HRESULT hr = XAppCaptureMetadataStartInt32State(
            name_utf8.get_data(),
            clamped,
            _to_native_priority(p_priority));
    return _wrap_hresult(hr, "Failed to start int32 state in capture metadata.", "capture_metadata_start_int32_state_failed");
}

void XboxCaptureMetaData::activate_internal() {
    m_valid = true;
}

// ── XboxCapture ────────────────────────────────────────────────────────────

void XboxCapture::_bind_methods() {
    ClassDB::bind_method(D_METHOD("enable_capture"), &XboxCapture::enable_capture);
    ClassDB::bind_method(D_METHOD("disable_capture"), &XboxCapture::disable_capture);
    ClassDB::bind_method(
            D_METHOD("record_diagnostic_clip_async", "duration"),
            &XboxCapture::record_diagnostic_clip_async);
    ClassDB::bind_method(
            D_METHOD("take_diagnostic_screenshot_async", "path_hint"),
            &XboxCapture::take_diagnostic_screenshot_async);
    ClassDB::bind_method(
            D_METHOD("create_metadata", "reserved_bytes"),
            &XboxCapture::create_metadata,
            DEFVAL(0));
}

XboxRuntime *XboxCapture::_get_runtime() const {
    if (m_owner == nullptr) {
        return nullptr;
    }
    return m_owner->get_runtime();
}

Signal XboxCapture::_make_error_signal(
        HRESULT p_hresult,
        const String &p_code,
        const String &p_message) const {
    XboxRuntime *runtime = _get_runtime();
    return runtime != nullptr ? runtime->make_error_signal(p_hresult, p_code, p_message) : Signal();
}

void XboxCapture::set_owner(Xbox *p_owner) {
    m_owner = p_owner;
}

Ref<XboxResult> XboxCapture::on_runtime_initialized() {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return XboxResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the capture service before the GDK runtime.");
    }
    m_runtime_ready = true;
    return XboxResult::ok_result();
}

void XboxCapture::shutdown() {
    m_runtime_ready = false;
}

Ref<XboxResult> XboxCapture::enable_capture() {
    XboxRuntime *runtime = _get_runtime();
    if (!m_runtime_ready) {
        Ref<XboxResult> result = XboxResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "The GDK runtime must be initialized before enabling capture.");
        if (runtime != nullptr) {
        }
        return result;
    }
    HRESULT hr = XAppCaptureEnableRecord();
    if (FAILED(hr)) {
        Ref<XboxResult> result = XboxResult::hresult_error(hr, "Failed to enable capture.", "capture_enable_failed");
        if (runtime != nullptr) {
        }
        return result;
    }
    if (runtime != nullptr) {
    }
    return XboxResult::ok_result();
}

Ref<XboxResult> XboxCapture::disable_capture() {
    XboxRuntime *runtime = _get_runtime();
    if (!m_runtime_ready) {
        Ref<XboxResult> result = XboxResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "The GDK runtime must be initialized before disabling capture.");
        if (runtime != nullptr) {
        }
        return result;
    }
    HRESULT hr = XAppCaptureDisableRecord();
    if (FAILED(hr)) {
        Ref<XboxResult> result = XboxResult::hresult_error(hr, "Failed to disable capture.", "capture_disable_failed");
        if (runtime != nullptr) {
        }
        return result;
    }
    if (runtime != nullptr) {
    }
    return XboxResult::ok_result();
}

Signal XboxCapture::record_diagnostic_clip_async(double p_duration) {
    XboxRuntime *runtime = _get_runtime();

    if (!std::isfinite(p_duration) || p_duration <= 0.0) {
        return _make_error_signal(
                E_INVALIDARG,
                "invalid_capture_duration",
                "Clip duration must be greater than zero.");
    }

    const double duration_seconds = std::ceil(p_duration);
    const double duration_ms = duration_seconds * 1000.0;
    if (duration_ms > static_cast<double>(std::numeric_limits<uint32_t>::max())) {
        return _make_error_signal(
                E_INVALIDARG,
                "invalid_capture_duration",
                "Clip duration is too large.");
    }

    if (!m_runtime_ready || runtime == nullptr) {
        return _make_error_signal(
                E_FAIL,
                "runtime_not_initialized",
                "The GDK runtime must be initialized before recording a diagnostic clip.");
    }

    Ref<XboxPendingSignal> pending_signal = runtime->make_pending_signal();
    ERR_FAIL_COND_V(pending_signal.is_null(), Signal());

    XAppCaptureRecordClipResult clip_result = {};
    const time_t now = std::time(nullptr);
    const time_t start_time = now - static_cast<time_t>(duration_seconds);
    HRESULT hr = XAppCaptureRecordDiagnosticClip(
            start_time,
            static_cast<uint32_t>(duration_ms),
            nullptr,
            &clip_result);
    if (FAILED(hr)) {
        Ref<XboxResult> result = XboxResult::hresult_error(
                hr,
                "Failed to record diagnostic clip.",
                "capture_record_failed");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    pending_signal->complete_deferred(XboxResult::ok_result());
    return pending_signal->get_completed_signal();
}

Signal XboxCapture::take_diagnostic_screenshot_async(const String &p_path_hint) {
    XboxRuntime *runtime = _get_runtime();

    if (p_path_hint.strip_edges().is_empty()) {
        return _make_error_signal(
                E_INVALIDARG,
                "invalid_screenshot_path_hint",
                "Screenshot path hint must not be empty.");
    }

    if (!m_runtime_ready || runtime == nullptr) {
        return _make_error_signal(
                E_FAIL,
                "runtime_not_initialized",
                "The GDK runtime must be initialized before taking a diagnostic screenshot.");
    }

    Ref<XboxPendingSignal> pending_signal = runtime->make_pending_signal();
    ERR_FAIL_COND_V(pending_signal.is_null(), Signal());

    CharString path_hint_utf8 = p_path_hint.utf8();
    XAppCaptureDiagnosticScreenshotResult screenshot_result = {};
    HRESULT hr = XAppCaptureTakeDiagnosticScreenshot(
            true,
            XAppCaptureScreenshotFormatFlag::SDR,
            path_hint_utf8.get_data(),
            &screenshot_result);
    if (FAILED(hr)) {
        Ref<XboxResult> result = XboxResult::hresult_error(
                hr,
                "Failed to take diagnostic screenshot.",
                "capture_screenshot_failed");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    pending_signal->complete_deferred(XboxResult::ok_result());
    return pending_signal->get_completed_signal();
}

Ref<XboxCaptureMetaData> XboxCapture::create_metadata(int64_t p_reserved_bytes) {
    (void)p_reserved_bytes;

    if (!m_runtime_ready) {
        return Ref<XboxCaptureMetaData>();
    }

    Ref<XboxCaptureMetaData> metadata;
    metadata.instantiate();
    metadata->activate_internal();
    return metadata;
}

} // namespace godot
