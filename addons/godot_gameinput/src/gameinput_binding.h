#ifndef GODOT_GAMEINPUT_BINDING_H
#define GODOT_GAMEINPUT_BINDING_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// One row in a GameInputActionMap. Maps a GameInput source (button or axis)
// onto a named Godot action. Inspector-friendly: every field is a typed
// @export so the editor renders dropdowns and sliders for free.
class GameInputBinding : public Resource {
    GDCLASS(GameInputBinding, Resource);

private:
    StringName m_action;
    int m_source = 2; // GameInputDevice::SRC_BTN_A
    bool m_is_axis = false;
    float m_axis_threshold = 0.5f;
    bool m_axis_invert = false;
    float m_deadzone = 0.2f;

protected:
    static void _bind_methods();

public:
    GameInputBinding() = default;
    ~GameInputBinding() = default;

    void set_action(const StringName &p_action);
    StringName get_action() const;

    void set_source(int p_source);
    int get_source() const;

    void set_is_axis(bool p_is_axis);
    bool get_is_axis() const;

    void set_axis_threshold(float p_threshold);
    float get_axis_threshold() const;

    void set_axis_invert(bool p_invert);
    bool get_axis_invert() const;

    void set_deadzone(float p_deadzone);
    float get_deadzone() const;
};

} // namespace godot

#endif // GODOT_GAMEINPUT_BINDING_H
