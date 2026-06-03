#include "gameinput_device.h"

#include "gameinput_singleton.h"

namespace godot {

void GameInputDevice::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_device_id"), &GameInputDevice::get_device_id);
    ClassDB::bind_method(D_METHOD("get_display_name"), &GameInputDevice::get_display_name);
    ClassDB::bind_method(D_METHOD("get_kind_mask"), &GameInputDevice::get_kind_mask);
    ClassDB::bind_method(D_METHOD("is_connected"), &GameInputDevice::is_connected);
    ClassDB::bind_method(D_METHOD("supports_vibration"), &GameInputDevice::supports_vibration);
    ClassDB::bind_method(D_METHOD("supports_haptics"), &GameInputDevice::supports_haptics);
    ClassDB::bind_method(D_METHOD("get_battery_level"), &GameInputDevice::get_battery_level);
    ClassDB::bind_method(D_METHOD("get_device_info"), &GameInputDevice::get_device_info);
    ClassDB::bind_static_method("GameInputDevice",
                                D_METHOD("button_to_source", "button"),
                                &GameInputDevice::button_to_source);
    ClassDB::bind_static_method("GameInputDevice",
                                D_METHOD("axis_to_source", "axis"),
                                &GameInputDevice::axis_to_source);

    BIND_ENUM_CONSTANT(BUTTON_NONE);
    BIND_ENUM_CONSTANT(BUTTON_MENU);
    BIND_ENUM_CONSTANT(BUTTON_VIEW);
    BIND_ENUM_CONSTANT(BUTTON_A);
    BIND_ENUM_CONSTANT(BUTTON_B);
    BIND_ENUM_CONSTANT(BUTTON_X);
    BIND_ENUM_CONSTANT(BUTTON_Y);
    BIND_ENUM_CONSTANT(BUTTON_DPAD_UP);
    BIND_ENUM_CONSTANT(BUTTON_DPAD_DOWN);
    BIND_ENUM_CONSTANT(BUTTON_DPAD_LEFT);
    BIND_ENUM_CONSTANT(BUTTON_DPAD_RIGHT);
    BIND_ENUM_CONSTANT(BUTTON_LEFT_SHOULDER);
    BIND_ENUM_CONSTANT(BUTTON_RIGHT_SHOULDER);
    BIND_ENUM_CONSTANT(BUTTON_LEFT_THUMB);
    BIND_ENUM_CONSTANT(BUTTON_RIGHT_THUMB);

    BIND_ENUM_CONSTANT(AXIS_LEFT_X);
    BIND_ENUM_CONSTANT(AXIS_LEFT_Y);
    BIND_ENUM_CONSTANT(AXIS_RIGHT_X);
    BIND_ENUM_CONSTANT(AXIS_RIGHT_Y);
    BIND_ENUM_CONSTANT(AXIS_LEFT_TRIGGER);
    BIND_ENUM_CONSTANT(AXIS_RIGHT_TRIGGER);

    BIND_ENUM_CONSTANT(SRC_BTN_MENU);
    BIND_ENUM_CONSTANT(SRC_BTN_VIEW);
    BIND_ENUM_CONSTANT(SRC_BTN_A);
    BIND_ENUM_CONSTANT(SRC_BTN_B);
    BIND_ENUM_CONSTANT(SRC_BTN_X);
    BIND_ENUM_CONSTANT(SRC_BTN_Y);
    BIND_ENUM_CONSTANT(SRC_BTN_DPAD_UP);
    BIND_ENUM_CONSTANT(SRC_BTN_DPAD_DOWN);
    BIND_ENUM_CONSTANT(SRC_BTN_DPAD_LEFT);
    BIND_ENUM_CONSTANT(SRC_BTN_DPAD_RIGHT);
    BIND_ENUM_CONSTANT(SRC_BTN_LEFT_SHOULDER);
    BIND_ENUM_CONSTANT(SRC_BTN_RIGHT_SHOULDER);
    BIND_ENUM_CONSTANT(SRC_BTN_LEFT_THUMB);
    BIND_ENUM_CONSTANT(SRC_BTN_RIGHT_THUMB);
    BIND_ENUM_CONSTANT(SRC_AXIS_LEFT_X);
    BIND_ENUM_CONSTANT(SRC_AXIS_LEFT_Y);
    BIND_ENUM_CONSTANT(SRC_AXIS_RIGHT_X);
    BIND_ENUM_CONSTANT(SRC_AXIS_RIGHT_Y);
    BIND_ENUM_CONSTANT(SRC_AXIS_LEFT_TRIGGER);
    BIND_ENUM_CONSTANT(SRC_AXIS_RIGHT_TRIGGER);
}

String GameInputDevice::get_display_name() const {
    GameInput *gi = GameInput::get_singleton();
    if (!gi) return String();
    return gi->device_get_display_name(m_id);
}

int GameInputDevice::get_kind_mask() const {
    GameInput *gi = GameInput::get_singleton();
    int mask = (int)GameInput::DEVICE_UNKNOWN;
    if (gi) gi->device_lookup(m_id, nullptr, &mask);
    return mask;
}

bool GameInputDevice::is_connected() const {
    GameInput *gi = GameInput::get_singleton();
    return gi ? gi->device_is_connected(m_id) : false;
}

bool GameInputDevice::supports_vibration() const {
    GameInput *gi = GameInput::get_singleton();
    return gi ? gi->device_supports_vibration(m_id) : false;
}

bool GameInputDevice::supports_haptics() const {
    GameInput *gi = GameInput::get_singleton();
    return gi ? gi->device_supports_haptics(m_id) : false;
}

float GameInputDevice::get_battery_level() const {
    GameInput *gi = GameInput::get_singleton();
    return gi ? gi->device_get_battery_level(m_id) : -1.0f;
}

Dictionary GameInputDevice::get_device_info() const {
    GameInput *gi = GameInput::get_singleton();
    return gi ? gi->device_get_device_info(m_id) : Dictionary();
}

int GameInputDevice::button_to_source(int button) {
    switch (button) {
        case BUTTON_MENU:           return SRC_BTN_MENU;
        case BUTTON_VIEW:           return SRC_BTN_VIEW;
        case BUTTON_A:              return SRC_BTN_A;
        case BUTTON_B:              return SRC_BTN_B;
        case BUTTON_X:              return SRC_BTN_X;
        case BUTTON_Y:              return SRC_BTN_Y;
        case BUTTON_DPAD_UP:        return SRC_BTN_DPAD_UP;
        case BUTTON_DPAD_DOWN:      return SRC_BTN_DPAD_DOWN;
        case BUTTON_DPAD_LEFT:      return SRC_BTN_DPAD_LEFT;
        case BUTTON_DPAD_RIGHT:     return SRC_BTN_DPAD_RIGHT;
        case BUTTON_LEFT_SHOULDER:  return SRC_BTN_LEFT_SHOULDER;
        case BUTTON_RIGHT_SHOULDER: return SRC_BTN_RIGHT_SHOULDER;
        case BUTTON_LEFT_THUMB:     return SRC_BTN_LEFT_THUMB;
        case BUTTON_RIGHT_THUMB:    return SRC_BTN_RIGHT_THUMB;
        default:                    return SRC_BTN_A; // safe default
    }
}

int GameInputDevice::axis_to_source(int axis) {
    switch (axis) {
        case AXIS_LEFT_X:        return SRC_AXIS_LEFT_X;
        case AXIS_LEFT_Y:        return SRC_AXIS_LEFT_Y;
        case AXIS_RIGHT_X:       return SRC_AXIS_RIGHT_X;
        case AXIS_RIGHT_Y:       return SRC_AXIS_RIGHT_Y;
        case AXIS_LEFT_TRIGGER:  return SRC_AXIS_LEFT_TRIGGER;
        case AXIS_RIGHT_TRIGGER: return SRC_AXIS_RIGHT_TRIGGER;
        default:                 return SRC_AXIS_LEFT_X;
    }
}

} // namespace godot
