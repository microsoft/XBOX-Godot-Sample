#include "register_types.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>

#include "gdk_core.h"
#include "gdk_user.h"
#include "gdk_input.h"
#include "gdk_achievements.h"

using namespace godot;

static GDKCore *gdk_core_singleton = nullptr;
static GDKUserManager *gdk_user_singleton = nullptr;
static GDKInput *gdk_input_singleton = nullptr;
static GDKAchievements *gdk_achievements_singleton = nullptr;

void initialize_gdk_extension(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        // Register classes
        ClassDB::register_class<GDKCore>();
        ClassDB::register_class<GDKUserInfo>();
        ClassDB::register_class<GDKUserManager>();
        ClassDB::register_class<GDKInput>();
        ClassDB::register_class<GDKAchievements>();

        // Create and register singletons
        gdk_core_singleton = memnew(GDKCore);
        Engine::get_singleton()->register_singleton("GDK", GDKCore::get_singleton());

        gdk_user_singleton = memnew(GDKUserManager);
        Engine::get_singleton()->register_singleton("GDKUser", GDKUserManager::get_singleton());

        gdk_input_singleton = memnew(GDKInput);
        Engine::get_singleton()->register_singleton("GDKInput", GDKInput::get_singleton());

        gdk_achievements_singleton = memnew(GDKAchievements);
        Engine::get_singleton()->register_singleton("GDKAchievements", GDKAchievements::get_singleton());
    }
}

void uninitialize_gdk_extension(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        // Unregister singletons
        Engine::get_singleton()->unregister_singleton("GDKAchievements");
        Engine::get_singleton()->unregister_singleton("GDKInput");
        Engine::get_singleton()->unregister_singleton("GDKUser");
        Engine::get_singleton()->unregister_singleton("GDK");

        // Clean up (reverse order of creation)
        if (gdk_achievements_singleton) {
            memdelete(gdk_achievements_singleton);
            gdk_achievements_singleton = nullptr;
        }
        if (gdk_input_singleton) {
            memdelete(gdk_input_singleton);
            gdk_input_singleton = nullptr;
        }
        if (gdk_user_singleton) {
            memdelete(gdk_user_singleton);
            gdk_user_singleton = nullptr;
        }
        if (gdk_core_singleton) {
            memdelete(gdk_core_singleton);
            gdk_core_singleton = nullptr;
        }
    }
}

extern "C" {

GDExtensionBool GDE_EXPORT gdk_extension_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization
) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_gdk_extension);
    init_obj.register_terminator(uninitialize_gdk_extension);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}

} // extern "C"
