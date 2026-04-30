#ifndef GODOT_GAMEINPUT_ACTION_MAP_H
#define GODOT_GAMEINPUT_ACTION_MAP_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

class GameInputBinding;

// A serializable list of GameInputBinding rows. Edit in the inspector or
// instantiate in code; resource paths are accepted by GameInputMapper.
class GameInputActionMap : public Resource {
    GDCLASS(GameInputActionMap, Resource);

private:
    TypedArray<GameInputBinding> m_bindings;

protected:
    static void _bind_methods();

public:
    GameInputActionMap() = default;
    ~GameInputActionMap() = default;

    void set_bindings(const TypedArray<GameInputBinding> &p_bindings);
    TypedArray<GameInputBinding> get_bindings() const;

    int get_binding_count() const;
    Ref<GameInputBinding> get_binding(int index) const;
    void add_binding(const Ref<GameInputBinding> &binding);
    void clear();
};

} // namespace godot

#endif // GODOT_GAMEINPUT_ACTION_MAP_H
