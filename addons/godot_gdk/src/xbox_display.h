#ifndef XBOX_DISPLAY_H
#define XBOX_DISPLAY_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <XDisplay.h>

namespace godot {

class Xbox;
class XboxResult;
class XboxRuntime;

// XboxDisplayTimeoutDeferral
// -------------------------
// RefCounted owner of an XDisplayTimeoutDeferralHandle. Returned by
// XboxDisplay::acquire_timeout_deferral(). The handle is closed via
// XDisplayCloseTimeoutDeferralHandle on release() or destruction.
//
// The deferral is independent of the GDK runtime lifecycle:
// XDisplayCloseTimeoutDeferralHandle does not require an initialized
// XGameRuntime, so it is always safe to release after GDK.shutdown().
class XboxDisplayTimeoutDeferral : public RefCounted {
    GDCLASS(XboxDisplayTimeoutDeferral, RefCounted);

    XDisplayTimeoutDeferralHandle m_handle = nullptr;

protected:
    static void _bind_methods();

public:
    XboxDisplayTimeoutDeferral() = default;
    ~XboxDisplayTimeoutDeferral();

    bool is_valid() const;
    void release();

    // Internal: takes ownership of the handle. Called only by XboxDisplay.
    void set_handle_internal(XDisplayTimeoutDeferralHandle p_handle);
};

// XboxDisplay
// ----------
// Display service for PC GDK. Exposed as GDK.display. Wraps the small
// XDisplay surface: HDR mode probing/enable and display timeout deferrals.
//
// PC GDK availability matrix (XDisplay.h / xgameruntime.lib):
//   XDisplayTryEnableHdrMode             -- YES, _GAMING_DESKTOP
//   XDisplayAcquireTimeoutDeferral       -- YES, _GAMING_DESKTOP
//   XDisplayCloseTimeoutDeferralHandle   -- YES, _GAMING_DESKTOP
class XboxDisplay : public RefCounted {
    GDCLASS(XboxDisplay, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;

    XboxRuntime *_get_runtime() const;

protected:
    static void _bind_methods();

public:
    // Maps to XDisplayHdrModeResult.
    enum HdrMode {
        HDR_MODE_UNKNOWN = static_cast<uint32_t>(XDisplayHdrModeResult::Unknown),
        HDR_MODE_ENABLED = static_cast<uint32_t>(XDisplayHdrModeResult::Enabled),
        HDR_MODE_DISABLED = static_cast<uint32_t>(XDisplayHdrModeResult::Disabled),
    };

    // Maps to XDisplayHdrModePreference.
    enum HdrModePreference {
        HDR_MODE_PREFERENCE_PREFER_HDR = static_cast<uint32_t>(XDisplayHdrModePreference::PreferHdr),
        HDR_MODE_PREFERENCE_PREFER_REFRESH_RATE = static_cast<uint32_t>(XDisplayHdrModePreference::PreferRefreshRate),
    };

    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();

    // Try to enable HDR. Returns an XboxResult whose data is a Dictionary:
    //   { mode: int (HDR_MODE_*),
    //     info: { min_tone_map_luminance, max_tone_map_luminance,
    //             max_full_frame_tone_map_luminance } when mode == HDR_MODE_ENABLED }
    // Wraps XDisplayTryEnableHdrMode.
    Ref<XboxResult> try_enable_hdr_mode(int64_t p_preference = HDR_MODE_PREFERENCE_PREFER_HDR);

    // Acquire a display timeout deferral (prevents idle display sleep while
    // the returned wrapper is held). Returns an XboxResult whose data is a
    // Ref<XboxDisplayTimeoutDeferral>. Wraps XDisplayAcquireTimeoutDeferral.
    Ref<XboxResult> acquire_timeout_deferral();
};

} // namespace godot

VARIANT_ENUM_CAST(godot::XboxDisplay::HdrMode);
VARIANT_ENUM_CAST(godot::XboxDisplay::HdrModePreference);

#endif // XBOX_DISPLAY_H
