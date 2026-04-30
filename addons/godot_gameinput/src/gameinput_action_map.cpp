#include "gameinput_action_map.h"

#include "gameinput_binding.h"

namespace godot {

void GameInputActionMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_bindings", "bindings"),
                         &GameInputActionMap::set_bindings);
    ClassDB::bind_method(D_METHOD("get_bindings"), &GameInputActionMap::get_bindings);
    ClassDB::bind_method(D_METHOD("get_binding_count"),
                         &GameInputActionMap::get_binding_count);
    ClassDB::bind_method(D_METHOD("get_binding", "index"),
                         &GameInputActionMap::get_binding);
    ClassDB::bind_method(D_METHOD("add_binding", "binding"),
                         &GameInputActionMap::add_binding);
    ClassDB::bind_method(D_METHOD("clear"), &GameInputActionMap::clear);

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "bindings",
                              PROPERTY_HINT_ARRAY_TYPE,
                              vformat("%s/%s:%s",
                                      Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE,
                                      "GameInputBinding")),
                 "set_bindings", "get_bindings");
}

void GameInputActionMap::set_bindings(const TypedArray<GameInputBinding> &p_bindings) {
    m_bindings = p_bindings;
    emit_changed();
}

TypedArray<GameInputBinding> GameInputActionMap::get_bindings() const {
    return m_bindings;
}

int GameInputActionMap::get_binding_count() const {
    return (int)m_bindings.size();
}

Ref<GameInputBinding> GameInputActionMap::get_binding(int index) const {
    if (index < 0 || index >= (int)m_bindings.size()) {
        return Ref<GameInputBinding>();
    }
    return m_bindings[index];
}

void GameInputActionMap::add_binding(const Ref<GameInputBinding> &binding) {
    if (binding.is_null()) return;
    m_bindings.push_back(binding);
    emit_changed();
}

void GameInputActionMap::clear() {
    m_bindings.clear();
    emit_changed();
}

} // namespace godot
