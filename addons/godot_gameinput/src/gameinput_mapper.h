#ifndef GODOT_GAMEINPUT_MAPPER_H
#define GODOT_GAMEINPUT_MAPPER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

class GameInputActionMap;

// Bridges GameInput → Godot's Input/InputMap. For every binding in the
// configured action_map the mapper:
//   * Refreshes the polled action state every frame via Input.action_press /
//     action_release so consumers using Input.is_action_pressed("move_up")
//     keep working.
//   * On every press<->release transition, parses an InputEventAction so
//     event-driven consumers (Viewport GUI focus traversal for ui_*,
//     _gui_input listeners, _input/_unhandled_input handlers) actually
//     observe the change, since Input.action_press alone updates polled
//     state without delivering an InputEvent.
//
// To avoid double-firing when Godot's built-in joypad backend is already
// wired to deliver the same action through an InputEventJoypadButton /
// InputEventJoypadMotion in the InputMap (e.g., default ui_accept ←
// joypad button A), the mapper checks each binding's action against the
// InputMap once, caches the result per binding, and skips its own
// InputEventAction emit when a matching native event exists. The polled
// action_press path keeps running either way; only the synthetic event is
// suppressed. The cache is invalidated whenever the action_map changes.
//
// Action names must already exist in Godot's InputMap for the standard
// Input.is_action_pressed("jump") API to work; the Mapper warns once per
// missing action (debounced via a per-instance set) instead of spamming.
//
// Multiple Mappers in the same scene are safe: GameInput.poll() is per-frame
// idempotent, and each Mapper tracks its own previous-frame "is pressed"
// state per binding so press/release identity is stable.
class GameInputMapper : public Node {
    GDCLASS(GameInputMapper, Node);

public:
    enum KindFlags {
        KIND_GAMEPAD  = 1 << 0,
        KIND_KEYBOARD = 1 << 1,
        KIND_MOUSE    = 1 << 2,
    };

private:
    Ref<GameInputActionMap> m_action_map;
    int m_target_kind_mask = KIND_GAMEPAD;
    int64_t m_target_device_id = -1; // -1 = primary device of the kind mask

    // Per-binding press state from the previous frame so we can emit
    // press/release on the right edges. Keyed by binding index in the map.
    HashMap<int, bool> m_prev_pressed;

    // Per-binding cache: does Godot's built-in joypad backend already drive
    // this binding's action via an equivalent InputEventJoypadButton /
    // InputEventJoypadMotion in the project's InputMap? When true, we skip
    // emitting our own InputEventAction for the same press to avoid every
    // ui_accept / ui_up / etc. firing twice. The polled state (action_press)
    // still gets refreshed every frame either way. Cache is invalidated when
    // the action map changes; rare runtime InputMap edits are tolerated as
    // single-frame staleness.
    HashMap<int, bool> m_native_handles_cache;

    // Action names we've already warned about being missing from InputMap.
    HashSet<StringName> m_warned_missing_actions;

    void _process_bindings();
    bool _is_pressed_for(int source, float &out_strength,
                         class GameInputReading *reading) const;
    bool _native_path_handles_binding(const Ref<class GameInputBinding> &binding,
                                      const StringName &action) const;

    static int _source_to_joy_button(int source);
    static int _source_to_joy_axis(int source);

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    GameInputMapper();
    ~GameInputMapper() = default;

    void set_action_map(const Ref<GameInputActionMap> &map);
    Ref<GameInputActionMap> get_action_map() const;

    void set_target_kind_mask(int mask);
    int get_target_kind_mask() const;

    void set_target_device_id(int64_t id);
    int64_t get_target_device_id() const;

    // For tests / inspection.
    int get_active_binding_count() const;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GameInputMapper::KindFlags);

#endif // GODOT_GAMEINPUT_MAPPER_H
