#ifndef GDK_DISPLAY_H
#define GDK_DISPLAY_H

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

class GDK;
class GDKResult;
class GDKRuntime;

// GDKDisplayTimeoutDeferral
// -------------------------
// RefCounted owner of an XDisplayTimeoutDeferralHandle. Returned by
// GDKDisplay::acquire_timeout_deferral(). The handle is closed via
// XDisplayCloseTimeoutDeferralHandle on release() or destruction.
//
// The deferral is independent of the GDK runtime lifecycle:
// XDisplayCloseTimeoutDeferralHandle does not require an initialized
// XGameRuntime, so it is always safe to release after GDK.shutdown().
class GDKDisplayTimeoutDeferral : public RefCounted {
    GDCLASS(GDKDisplayTimeoutDeferral, RefCounted);

    XDisplayTimeoutDeferralHandle m_handle = nullptr;

protected:
    static void _bind_methods();

public:
    GDKDisplayTimeoutDeferral() = default;
    ~GDKDisplayTimeoutDeferral();

    bool is_valid() const;
    void release();

    // Internal: takes ownership of the handle. Called only by GDKDisplay.
    void set_handle_internal(XDisplayTimeoutDeferralHandle p_handle);
};

// GDKDisplay
// ----------
// Display service for PC GDK. Exposed as GDK.display. Wraps the small
// XDisplay surface: HDR mode probing/enable and display timeout deferrals.
//
// PC GDK availability matrix (XDisplay.h / xgameruntime.lib):
//   XDisplayTryEnableHdrMode             -- YES, _GAMING_DESKTOP
//   XDisplayAcquireTimeoutDeferral       -- YES, _GAMING_DESKTOP
//   XDisplayCloseTimeoutDeferralHandle   -- YES, _GAMING_DESKTOP
class GDKDisplay : public RefCounted {
    GDCLASS(GDKDisplay, RefCounted);

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;

    GDKRuntime *_get_runtime() const;

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

    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    // Try to enable HDR. Returns a GDKResult whose data is a Dictionary:
    //   { mode: int (HDR_MODE_*),
    //     info: { min_tone_map_luminance, max_tone_map_luminance,
    //             max_full_frame_tone_map_luminance } when mode == HDR_MODE_ENABLED }
    // Wraps XDisplayTryEnableHdrMode.
    Ref<GDKResult> try_enable_hdr_mode(int64_t p_preference = HDR_MODE_PREFERENCE_PREFER_HDR);

    // Acquire a display timeout deferral (prevents idle display sleep while
    // the returned wrapper is held). Returns a GDKResult whose data is a
    // Ref<GDKDisplayTimeoutDeferral>. Wraps XDisplayAcquireTimeoutDeferral.
    Ref<GDKResult> acquire_timeout_deferral();
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKDisplay::HdrMode);
VARIANT_ENUM_CAST(godot::GDKDisplay::HdrModePreference);

#endif // GDK_DISPLAY_H
