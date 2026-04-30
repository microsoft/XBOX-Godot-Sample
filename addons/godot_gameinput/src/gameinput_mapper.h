#ifndef GODOT_GAMEINPUT_MAPPER_H
#define GODOT_GAMEINPUT_MAPPER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

class GameInputActionMap;

// Bridges GameInput → Godot's Input/InputMap by emitting Input.action_press /
// action_release each frame for every binding in the configured action_map.
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

    // Action names we've already warned about being missing from InputMap.
    HashSet<StringName> m_warned_missing_actions;

    void _process_bindings();
    bool _is_pressed_for(int source, float &out_strength,
                         class GameInputReading *reading) const;

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
