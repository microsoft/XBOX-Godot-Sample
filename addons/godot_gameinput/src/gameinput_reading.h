#ifndef GODOT_GAMEINPUT_READING_H
#define GODOT_GAMEINPUT_READING_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <GameInput.h>

namespace godot {

// Snapshot of the gamepad state for a single device, captured at the moment
// GameInput.get_current_reading(device) was called. Holds both the current and
// previous-poll states so was_button_pressed() can return the correct edge.
class GameInputReading : public RefCounted {
    GDCLASS(GameInputReading, RefCounted);

private:
    GameInputGamepadState m_cur{};
    GameInputGamepadState m_prev{};
    bool m_has_prev = false;

protected:
    static void _bind_methods();

public:
    GameInputReading() = default;
    ~GameInputReading() = default;

    void _set_state(const GameInputGamepadState &cur,
                    const GameInputGamepadState &prev,
                    bool has_prev);

    bool is_button_down(int button) const;
    bool was_button_pressed(int button) const;
    bool was_button_released(int button) const;
    float get_axis(int axis) const;
    int get_buttons_mask() const;

    // 0 here = no timestamp captured (we don't surface IGameInputReading::GetTimestamp
    // because we snapshot per-frame; that field is reserved for a future v2).
    int64_t get_timestamp() const { return 0; }
};

} // namespace godot

#endif // GODOT_GAMEINPUT_READING_H
