#include "gameinput_reading.h"

#include "gameinput_device.h"

namespace godot {

void GameInputReading::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_button_down", "button"), &GameInputReading::is_button_down);
    ClassDB::bind_method(D_METHOD("was_button_pressed", "button"),
                         &GameInputReading::was_button_pressed);
    ClassDB::bind_method(D_METHOD("was_button_released", "button"),
                         &GameInputReading::was_button_released);
    ClassDB::bind_method(D_METHOD("get_axis", "axis"), &GameInputReading::get_axis);
    ClassDB::bind_method(D_METHOD("get_buttons_mask"), &GameInputReading::get_buttons_mask);
    ClassDB::bind_method(D_METHOD("get_timestamp"), &GameInputReading::get_timestamp);
}

static GameInputGamepadButtons _native_button_for(int button) {
    using GD = GameInputDevice;
    switch (button) {
        case GD::BUTTON_MENU:           return GameInputGamepadMenu;
        case GD::BUTTON_VIEW:           return GameInputGamepadView;
        case GD::BUTTON_A:              return GameInputGamepadA;
        case GD::BUTTON_B:              return GameInputGamepadB;
        case GD::BUTTON_X:              return GameInputGamepadX;
        case GD::BUTTON_Y:              return GameInputGamepadY;
        case GD::BUTTON_DPAD_UP:        return GameInputGamepadDPadUp;
        case GD::BUTTON_DPAD_DOWN:      return GameInputGamepadDPadDown;
        case GD::BUTTON_DPAD_LEFT:      return GameInputGamepadDPadLeft;
        case GD::BUTTON_DPAD_RIGHT:     return GameInputGamepadDPadRight;
        case GD::BUTTON_LEFT_SHOULDER:  return GameInputGamepadLeftShoulder;
        case GD::BUTTON_RIGHT_SHOULDER: return GameInputGamepadRightShoulder;
        case GD::BUTTON_LEFT_THUMB:     return GameInputGamepadLeftThumbstick;
        case GD::BUTTON_RIGHT_THUMB:    return GameInputGamepadRightThumbstick;
        default:                        return GameInputGamepadNone;
    }
}

void GameInputReading::_set_state(const GameInputGamepadState &cur,
                                  const GameInputGamepadState &prev,
                                  bool has_prev) {
    m_cur = cur;
    m_prev = prev;
    m_has_prev = has_prev;
}

bool GameInputReading::is_button_down(int button) const {
    GameInputGamepadButtons mask = _native_button_for(button);
    if (mask == GameInputGamepadNone) return false;
    return (m_cur.buttons & mask) != 0;
}

bool GameInputReading::was_button_pressed(int button) const {
    if (!m_has_prev) {
        // No previous frame yet → treat as a fresh press if currently down.
        return is_button_down(button);
    }
    GameInputGamepadButtons mask = _native_button_for(button);
    if (mask == GameInputGamepadNone) return false;
    bool was_down = (m_prev.buttons & mask) != 0;
    bool is_down  = (m_cur.buttons  & mask) != 0;
    return is_down && !was_down;
}

bool GameInputReading::was_button_released(int button) const {
    if (!m_has_prev) {
        return false;
    }
    GameInputGamepadButtons mask = _native_button_for(button);
    if (mask == GameInputGamepadNone) return false;
    bool was_down = (m_prev.buttons & mask) != 0;
    bool is_down  = (m_cur.buttons  & mask) != 0;
    return !is_down && was_down;
}

float GameInputReading::get_axis(int axis) const {
    using GD = GameInputDevice;
    switch (axis) {
        case GD::AXIS_LEFT_X:        return m_cur.leftThumbstickX;
        // Godot Y axis convention: down is positive; GameInput thumbstick Y up is positive.
        case GD::AXIS_LEFT_Y:        return -m_cur.leftThumbstickY;
        case GD::AXIS_RIGHT_X:       return m_cur.rightThumbstickX;
        case GD::AXIS_RIGHT_Y:       return -m_cur.rightThumbstickY;
        case GD::AXIS_LEFT_TRIGGER:  return m_cur.leftTrigger;
        case GD::AXIS_RIGHT_TRIGGER: return m_cur.rightTrigger;
        default:                     return 0.0f;
    }
}

int GameInputReading::get_buttons_mask() const {
    return (int)m_cur.buttons;
}

} // namespace godot
