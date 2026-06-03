#ifndef GODOT_GAMEINPUT_DEVICE_H
#define GODOT_GAMEINPUT_DEVICE_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

// GDScript-facing wrapper around an IGameInputDevice. Holds ONLY a
// session-local monotonic device id (a weak handle). All methods resolve
// through the GameInput singleton; if the device has been disconnected,
// methods return safe defaults so user code holding a stale reference does
// not crash.
class GameInputDevice : public RefCounted {
    GDCLASS(GameInputDevice, RefCounted);

public:
    enum Button {
        BUTTON_NONE           = 0,
        BUTTON_MENU           = 1 << 0,
        BUTTON_VIEW           = 1 << 1,
        BUTTON_A              = 1 << 2,
        BUTTON_B              = 1 << 3,
        BUTTON_X              = 1 << 4,
        BUTTON_Y              = 1 << 5,
        BUTTON_DPAD_UP        = 1 << 6,
        BUTTON_DPAD_DOWN      = 1 << 7,
        BUTTON_DPAD_LEFT      = 1 << 8,
        BUTTON_DPAD_RIGHT     = 1 << 9,
        BUTTON_LEFT_SHOULDER  = 1 << 10,
        BUTTON_RIGHT_SHOULDER = 1 << 11,
        BUTTON_LEFT_THUMB     = 1 << 12,
        BUTTON_RIGHT_THUMB    = 1 << 13,
    };

    enum Axis {
        AXIS_LEFT_X       = 0,
        AXIS_LEFT_Y       = 1,
        AXIS_RIGHT_X      = 2,
        AXIS_RIGHT_Y      = 3,
        AXIS_LEFT_TRIGGER = 4,
        AXIS_RIGHT_TRIGGER= 5,
    };

    // Source ids used by GameInputBinding.source. Negative values = axes,
    // non-negative values = button bitmask shifts. Kept as a single namespace
    // so an inspector dropdown can show one combined list.
    enum Source {
        SRC_BTN_MENU            = 0,
        SRC_BTN_VIEW            = 1,
        SRC_BTN_A               = 2,
        SRC_BTN_B               = 3,
        SRC_BTN_X               = 4,
        SRC_BTN_Y               = 5,
        SRC_BTN_DPAD_UP         = 6,
        SRC_BTN_DPAD_DOWN       = 7,
        SRC_BTN_DPAD_LEFT       = 8,
        SRC_BTN_DPAD_RIGHT      = 9,
        SRC_BTN_LEFT_SHOULDER   = 10,
        SRC_BTN_RIGHT_SHOULDER  = 11,
        SRC_BTN_LEFT_THUMB      = 12,
        SRC_BTN_RIGHT_THUMB     = 13,

        SRC_AXIS_LEFT_X         = 100,
        SRC_AXIS_LEFT_Y         = 101,
        SRC_AXIS_RIGHT_X        = 102,
        SRC_AXIS_RIGHT_Y        = 103,
        SRC_AXIS_LEFT_TRIGGER   = 104,
        SRC_AXIS_RIGHT_TRIGGER  = 105,
    };

private:
    int64_t m_id = 0;

protected:
    static void _bind_methods();

public:
    GameInputDevice() = default;
    ~GameInputDevice() = default;

    // Internal: set by GameInput when constructing the wrapper.
    void _set_device_id(int64_t id) { m_id = id; }

    int64_t get_device_id() const { return m_id; }
    String get_display_name() const;
    int get_kind_mask() const;
    bool is_connected() const;
    bool supports_vibration() const;
    bool supports_haptics() const;
    float get_battery_level() const;
    Dictionary get_device_info() const;

    // Helpers used internally + exposed to GDScript for convenience.
    static int button_to_source(int button);
    static int axis_to_source(int axis);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GameInputDevice::Button);
VARIANT_ENUM_CAST(godot::GameInputDevice::Axis);
VARIANT_ENUM_CAST(godot::GameInputDevice::Source);

#endif // GODOT_GAMEINPUT_DEVICE_H
