#include "gameinput_binding.h"

#include "gameinput_device.h"

namespace godot {

void GameInputBinding::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_action", "action"), &GameInputBinding::set_action);
    ClassDB::bind_method(D_METHOD("get_action"), &GameInputBinding::get_action);
    ClassDB::bind_method(D_METHOD("set_source", "source"), &GameInputBinding::set_source);
    ClassDB::bind_method(D_METHOD("get_source"), &GameInputBinding::get_source);
    ClassDB::bind_method(D_METHOD("set_is_axis", "is_axis"), &GameInputBinding::set_is_axis);
    ClassDB::bind_method(D_METHOD("get_is_axis"), &GameInputBinding::get_is_axis);
    ClassDB::bind_method(D_METHOD("set_axis_threshold", "threshold"),
                         &GameInputBinding::set_axis_threshold);
    ClassDB::bind_method(D_METHOD("get_axis_threshold"), &GameInputBinding::get_axis_threshold);
    ClassDB::bind_method(D_METHOD("set_axis_invert", "invert"),
                         &GameInputBinding::set_axis_invert);
    ClassDB::bind_method(D_METHOD("get_axis_invert"), &GameInputBinding::get_axis_invert);
    ClassDB::bind_method(D_METHOD("set_deadzone", "deadzone"),
                         &GameInputBinding::set_deadzone);
    ClassDB::bind_method(D_METHOD("get_deadzone"), &GameInputBinding::get_deadzone);

    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "action"), "set_action", "get_action");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "source", PROPERTY_HINT_ENUM,
                              "Menu:0,View:1,A:2,B:3,X:4,Y:5,DPad Up:6,DPad Down:7,"
                              "DPad Left:8,DPad Right:9,Left Shoulder:10,Right Shoulder:11,"
                              "Left Thumb:12,Right Thumb:13,Axis Left X:100,Axis Left Y:101,"
                              "Axis Right X:102,Axis Right Y:103,Axis Left Trigger:104,"
                              "Axis Right Trigger:105"),
                 "set_source", "get_source");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_axis"), "set_is_axis", "get_is_axis");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "axis_threshold",
                              PROPERTY_HINT_RANGE, "0.0,1.0,0.01"),
                 "set_axis_threshold", "get_axis_threshold");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_invert"),
                 "set_axis_invert", "get_axis_invert");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "deadzone",
                              PROPERTY_HINT_RANGE, "0.0,1.0,0.01"),
                 "set_deadzone", "get_deadzone");
}

void GameInputBinding::set_action(const StringName &p_action) {
    m_action = p_action;
    emit_changed();
}

StringName GameInputBinding::get_action() const {
    return m_action;
}

void GameInputBinding::set_source(int p_source) {
    m_source = p_source;
    emit_changed();
}

int GameInputBinding::get_source() const {
    return m_source;
}

void GameInputBinding::set_is_axis(bool p_is_axis) {
    m_is_axis = p_is_axis;
    emit_changed();
}

bool GameInputBinding::get_is_axis() const {
    return m_is_axis;
}

void GameInputBinding::set_axis_threshold(float p_threshold) {
    if (p_threshold < 0.0f) p_threshold = 0.0f;
    if (p_threshold > 1.0f) p_threshold = 1.0f;
    m_axis_threshold = p_threshold;
    emit_changed();
}

float GameInputBinding::get_axis_threshold() const {
    return m_axis_threshold;
}

void GameInputBinding::set_axis_invert(bool p_invert) {
    m_axis_invert = p_invert;
    emit_changed();
}

bool GameInputBinding::get_axis_invert() const {
    return m_axis_invert;
}

void GameInputBinding::set_deadzone(float p_deadzone) {
    if (p_deadzone < 0.0f) p_deadzone = 0.0f;
    if (p_deadzone > 1.0f) p_deadzone = 1.0f;
    m_deadzone = p_deadzone;
    emit_changed();
}

float GameInputBinding::get_deadzone() const {
    return m_deadzone;
}

} // namespace godot
