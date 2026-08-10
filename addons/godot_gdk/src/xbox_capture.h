#ifndef XBOX_CAPTURE_H
#define XBOX_CAPTURE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <XAppCapture.h>

namespace godot {

class Xbox;
class XboxResult;
class XboxRuntime;

// XboxCaptureMetaData
// ------------------
// Script-side metadata write context. PC GDK exposes XAppCaptureMetadata*
// as stateless process-wide calls, so this object only gates intentional
// metadata writes after XboxCapture::create_metadata().
//
// PC GDK availability: all wrapped XAppCaptureMetadata* functions are
// available in _GAMING_DESKTOP builds via XAppCapture.h / xgameruntime.lib.
class XboxCaptureMetaData : public RefCounted {
    GDCLASS(XboxCaptureMetaData, RefCounted);

    bool m_valid = false;

    Ref<XboxResult> _check_valid() const;
    static int32_t _clamp_to_int32(int64_t p_value);
    static XAppCaptureMetadataPriority _to_native_priority(int64_t p_priority);
    static Ref<XboxResult> _wrap_hresult(HRESULT p_hr, const char *p_action, const char *p_code);

protected:
    static void _bind_methods();

public:
    // Maps to XAppCaptureMetadataPriority.
    enum Priority {
        PRIORITY_GAMEPLAY = 0,  // XAppCaptureMetadataPriority::Informational
        PRIORITY_IMPORTANT = 1, // XAppCaptureMetadataPriority::Important
    };

    ~XboxCaptureMetaData();

    // Returns true while this script-side context is open and usable.
    bool is_valid() const;

    // Close the script-side context. After close() is_valid() returns false.
    // Calling close() on an already-closed context is safe (no-op).
    void close();

    // Stop all active metadata states.
    // Wraps XAppCaptureMetadataStopAllStates.
    Ref<XboxResult> stop_all_states();

    // Returns the number of bytes remaining in the metadata write buffer.
    // Wraps XAppCaptureMetadataRemainingStorageBytesAvailable.
    // Returns -1 when the context is invalid or the call fails.
    int64_t get_remaining_storage_bytes() const;

    // Add a one-shot string event to the metadata buffer.
    // Wraps XAppCaptureMetadataAddStringEvent.
    Ref<XboxResult> add_string_event(
            const String &p_name,
            const String &p_value,
            int64_t p_priority = PRIORITY_GAMEPLAY);

    // Add a one-shot double event to the metadata buffer.
    // Wraps XAppCaptureMetadataAddDoubleEvent.
    Ref<XboxResult> add_double_event(
            const String &p_name,
            double p_value,
            int64_t p_priority = PRIORITY_GAMEPLAY);

    // Add a one-shot int32 event to the metadata buffer.
    // Wraps XAppCaptureMetadataAddInt32Event.
    Ref<XboxResult> add_int32_event(
            const String &p_name,
            int64_t p_value,
            int64_t p_priority = PRIORITY_GAMEPLAY);

    // Begin a persistent string state in the metadata buffer.
    // Wraps XAppCaptureMetadataStartStringState.
    Ref<XboxResult> start_string_state(
            const String &p_name,
            const String &p_value,
            int64_t p_priority = PRIORITY_GAMEPLAY);

    // Begin a persistent double state in the metadata buffer.
    // Wraps XAppCaptureMetadataStartDoubleState.
    Ref<XboxResult> start_double_state(
            const String &p_name,
            double p_value,
            int64_t p_priority = PRIORITY_GAMEPLAY);

    // Begin a persistent int32 state in the metadata buffer.
    // Wraps XAppCaptureMetadataStartInt32State.
    Ref<XboxResult> start_int32_state(
            const String &p_name,
            int64_t p_value,
            int64_t p_priority = PRIORITY_GAMEPLAY);

    // Internal: open the script-side context. Called only by XboxCapture.
    void activate_internal();
};

// XboxCapture
// ----------
// Capture metadata and capture-state service for PC GDK. Exposed as
// GDK.capture. Wraps the PC-supported subset of XAppCapture.
//
// PC GDK availability matrix (XAppCapture.h / xgameruntime.lib):
//   XAppCaptureEnableRecord           -- YES, _GAMING_DESKTOP
//   XAppCaptureDisableRecord          -- YES, _GAMING_DESKTOP
//   XAppCaptureRecordDiagnosticClip   -- YES, _GAMING_DESKTOP (uses Game Bar)
//   XAppCaptureTakeDiagnosticScreenshot -- YES, _GAMING_DESKTOP (uses Game Bar)
//   XAppCaptureMetadata*              -- YES, _GAMING_DESKTOP (all metadata ops)
//
// Console-only paths excluded (not wrapped):
//   XAppCaptureOpenLocalStorageFiles / XAppCaptureCloseLocalStorageFilesHandle
//   XAppCaptureDiagnosticClipLocalId result extraction (console identifier)
class XboxCapture : public RefCounted {
    GDCLASS(XboxCapture, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;

    XboxRuntime *_get_runtime() const;
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) const;

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();

    // Enable capture (re-enables after a previous disable_capture call).
    // Wraps XAppCaptureEnableRecord.
    Ref<XboxResult> enable_capture();

    // Disable capture system-wide for this title.
    // Wraps XAppCaptureDisableRecord.
    Ref<XboxResult> disable_capture();

    // Asynchronously record a short diagnostic clip of the given duration
    // (in seconds). Returns a completion Signal that emits XboxResult.
    // Wraps XAppCaptureRecordDiagnosticClip.
    Signal record_diagnostic_clip_async(double p_duration);

    // Asynchronously take a diagnostic screenshot with the given filename
    // hint. Returns a completion Signal that emits XboxResult.
    // Wraps XAppCaptureTakeDiagnosticScreenshot.
    Signal take_diagnostic_screenshot_async(const String &p_path_hint);

    // Create a metadata write context. Returns an XboxCaptureMetaData wrapper
    // or null if the runtime is not initialized. PC GDK metadata calls are
    // stateless, so p_reserved_bytes is retained for compatibility and ignored.
    Ref<XboxCaptureMetaData> create_metadata(int64_t p_reserved_bytes = 0);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::XboxCaptureMetaData::Priority);

#endif // XBOX_CAPTURE_H
