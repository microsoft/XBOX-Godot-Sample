#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "gameinput_singleton.h"
#include "gameinput_action_map.h"
#include "gameinput_binding.h"
#include "gameinput_device.h"
#include "gameinput_mapper.h"
#include "gameinput_reading.h"

using namespace godot;

static GameInput *gameinput_singleton = nullptr;

static void _register_setting(const String &name, const Variant &default_value,
                              Variant::Type type, PropertyHint hint = PROPERTY_HINT_NONE,
                              const String &hint_string = String()) {
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (!ps) return;
    if (!ps->has_setting(name)) {
        ps->set_setting(name, default_value);
    }
    ps->set_initial_value(name, default_value);

    Dictionary info;
    info["name"] = name;
    info["type"] = type;
    info["hint"] = hint;
    info["hint_string"] = hint_string;
    ps->add_property_info(info);
}

void initialize_godot_gameinput_extension(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ClassDB::register_class<GameInput>();
    ClassDB::register_class<GameInputDevice>();
    ClassDB::register_class<GameInputReading>();
    ClassDB::register_class<GameInputBinding>();
    ClassDB::register_class<GameInputActionMap>();
    ClassDB::register_class<GameInputMapper>();

    gameinput_singleton = memnew(GameInput);
    Engine::get_singleton()->register_singleton("GameInput", GameInput::get_singleton());

    // Project settings — read by the bootstrap autoload at runtime.
    _register_setting("game_input/runtime/initialize_on_startup", false, Variant::BOOL);
    _register_setting("game_input/runtime/auto_poll", true, Variant::BOOL);
    _register_setting("game_input/mapper/default_action_map", String(""),
                      Variant::STRING, PROPERTY_HINT_FILE, "*.tres,*.res");
}

void uninitialize_godot_gameinput_extension(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    Engine::get_singleton()->unregister_singleton("GameInput");

    if (gameinput_singleton) {
        memdelete(gameinput_singleton);
        gameinput_singleton = nullptr;
    }
}

extern "C" {

GDExtensionBool GDE_EXPORT godot_gameinput_extension_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization
) {
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_godot_gameinput_extension);
    init_obj.register_terminator(uninitialize_godot_gameinput_extension);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}

} // extern "C"
