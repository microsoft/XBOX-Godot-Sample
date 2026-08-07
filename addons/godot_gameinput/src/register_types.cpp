#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "gameinput_singleton.h"
#include "gameinput_action_map.h"
#include "gameinput_binding.h"
#include "gameinput_device.h"
#include "gameinput_mapper.h"
#include "gameinput_reading.h"

// All initialize/uninitialize helpers live inside `namespace godot` (matching
// the other .cpp files in this addon). This keeps unqualified `GameInput` here
// resolving to our `godot::GameInput` class rather than the global `::GameInput`
// namespace that the vcpkg gameinput port (GameInput v3) introduces via
// `<GameInput.h>`.
namespace godot {

static GameInput *gameinput_singleton = nullptr;

static constexpr const char *GAMEINPUT_SINGLETON_NAME_SETTING = "game_input/runtime/singleton_name";
static constexpr const char *GAMEINPUT_SINGLETON_NAME_DEFAULT = "GameInput";

// Name the singleton was actually registered under. Captured at initialize
// time so uninitialize unregisters the same name even if the project setting
// is edited while the extension is loaded. Function-local so the String is
// constructed on first use: a file-scope String would run its constructor from
// DllMain, before the GDExtension interface pointers exist, and fail the load.
static String &_registered_singleton_name() {
    static String name;
    return name;
}

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

// Resolves the Engine singleton name from `game_input/runtime/singleton_name`,
// falling back to "GameInput" when the configured value is unusable. Rejecting
// a bad value instead of honouring it keeps the addon reachable: an
// unregisterable name would leave every `GameInput.*` call in a project
// unresolved with no clue why.
static String _resolve_singleton_name() {
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (!ps) {
        return String(GAMEINPUT_SINGLETON_NAME_DEFAULT);
    }

    const String configured = String(ps->get_setting(GAMEINPUT_SINGLETON_NAME_SETTING,
                                                     GAMEINPUT_SINGLETON_NAME_DEFAULT))
                                      .strip_edges();
    if (configured.is_empty() || configured == GAMEINPUT_SINGLETON_NAME_DEFAULT) {
        return String(GAMEINPUT_SINGLETON_NAME_DEFAULT);
    }

    if (!configured.is_valid_ascii_identifier()) {
        UtilityFunctions::push_warning(
                String("[GameInput] Project setting '") + GAMEINPUT_SINGLETON_NAME_SETTING +
                "' is not a valid identifier ('" + configured + "'). Falling back to '" +
                GAMEINPUT_SINGLETON_NAME_DEFAULT + "'.");
        return String(GAMEINPUT_SINGLETON_NAME_DEFAULT);
    }

    if (Engine::get_singleton()->has_singleton(configured)) {
        UtilityFunctions::push_warning(
                String("[GameInput] Project setting '") + GAMEINPUT_SINGLETON_NAME_SETTING + "' requests '" +
                configured + "', which is already a registered singleton. Falling back to '" +
                GAMEINPUT_SINGLETON_NAME_DEFAULT + "'.");
        return String(GAMEINPUT_SINGLETON_NAME_DEFAULT);
    }

    return configured;
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

    // Project settings — read by the bootstrap autoload at runtime.
    _register_setting("game_input/runtime/initialize_on_startup", false, Variant::BOOL);
    _register_setting("game_input/runtime/auto_poll", true, Variant::BOOL);
    _register_setting(GAMEINPUT_SINGLETON_NAME_SETTING, String(GAMEINPUT_SINGLETON_NAME_DEFAULT),
                      Variant::STRING);
    _register_setting("game_input/mapper/default_action_map", String(""),
                      Variant::STRING, PROPERTY_HINT_FILE, "*.tres,*.res");

    _registered_singleton_name() = _resolve_singleton_name();
    Engine::get_singleton()->register_singleton(_registered_singleton_name(), GameInput::get_singleton());
}

void uninitialize_godot_gameinput_extension(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (!_registered_singleton_name().is_empty()) {
        Engine::get_singleton()->unregister_singleton(_registered_singleton_name());
        _registered_singleton_name() = String();
    }

    if (gameinput_singleton) {
        gameinput_singleton->shutdown();
        memdelete(gameinput_singleton);
        gameinput_singleton = nullptr;
    }
}

} // namespace godot

extern "C" {

GDExtensionBool GDE_EXPORT godot_gameinput_extension_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization
) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(godot::initialize_godot_gameinput_extension);
    init_obj.register_terminator(godot::uninitialize_godot_gameinput_extension);
    init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}

} // extern "C"
