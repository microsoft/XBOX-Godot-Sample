#include "register_types.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>

#include "gdk_achievement.h"
#include "gdk.h"
#include "gdk_async_op.h"
#include "gdk_dispatch_op.h"
#include "gdk_multiplayer_activity.h"
#include "gdk_presence.h"
#include "gdk_result.h"
#include "gdk_social.h"
#include "gdk_user.h"

using namespace godot;

static GDK *gdk_singleton = nullptr;

void initialize_gdk_extension(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        ClassDB::register_class<GDK>();
        ClassDB::register_class<GDKResult>();
        ClassDB::register_class<GDKAsyncOp>();
        ClassDB::register_class<GDKDispatchOp>();
        ClassDB::register_class<GDKUser>();
        ClassDB::register_class<GDKUsers>();
        ClassDB::register_class<GDKAchievement>();
        ClassDB::register_class<GDKAchievements>();
        ClassDB::register_class<GDKPresenceRecord>();
        ClassDB::register_class<GDKPresence>();
        ClassDB::register_class<GDKSocialFilter>();
        ClassDB::register_class<GDKSocialGroup>();
        ClassDB::register_class<GDKSocialUser>();
        ClassDB::register_class<GDKSocial>();
        ClassDB::register_class<GDKMultiplayerActivityInfo>();
        ClassDB::register_class<GDKMultiplayerActivity>();

        gdk_singleton = memnew(GDK);
        Engine::get_singleton()->register_singleton("GDK", GDK::get_singleton());
    }
}

void uninitialize_gdk_extension(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        Engine::get_singleton()->unregister_singleton("GDK");

        if (gdk_singleton) {
            memdelete(gdk_singleton);
            gdk_singleton = nullptr;
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
