#include "register_types.h"

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>

#include "gdk_achievement.h"
#include "gdk_accessibility.h"
#include "gdk.h"
#include "gdk_launcher.h"
#include "gdk_multiplayer_activity.h"
#include "gdk_pending_signal.h"
#include "gdk_presence.h"
#include "gdk_result.h"
#include "gdk_social.h"
#include "gdk_system.h"
#include "gdk_user.h"

using namespace godot;

static GDK *gdk_singleton = nullptr;

namespace {

constexpr const char *GDK_RUNTIME_INITIALIZE_ON_STARTUP_SETTING = "gdk/runtime/initialize_on_startup";
constexpr bool GDK_RUNTIME_INITIALIZE_ON_STARTUP_DEFAULT = false;
constexpr const char *GDK_RUNTIME_EMBED_DISPATCH_SETTING = "gdk/runtime/embed_dispatch";
constexpr bool GDK_RUNTIME_EMBED_DISPATCH_DEFAULT = true;
constexpr const char *GDK_RUNTIME_AUTO_ADD_PRIMARY_USER_SETTING = "gdk/runtime/auto_add_primary_user";
constexpr bool GDK_RUNTIME_AUTO_ADD_PRIMARY_USER_DEFAULT = false;
constexpr const char *GDK_TESTS_LIVE_REQUIRED_SETTING = "gdk/tests/live_required";
constexpr bool GDK_TESTS_LIVE_REQUIRED_DEFAULT = false;

void register_bool_setting(const char *name, bool default_value) {
    ProjectSettings *project_settings = ProjectSettings::get_singleton();
    if (project_settings == nullptr) {
        return;
    }

    if (!project_settings->has_setting(name)) {
        project_settings->set_setting(name, default_value);
    }

    project_settings->set_initial_value(name, default_value);
    project_settings->set_as_basic(name, true);

    Dictionary setting_info;
    setting_info["name"] = name;
    setting_info["type"] = Variant::BOOL;
    setting_info["hint"] = PROPERTY_HINT_NONE;
    setting_info["hint_string"] = "";
    project_settings->add_property_info(setting_info);
}

void register_gdk_project_settings() {
    register_bool_setting(GDK_RUNTIME_INITIALIZE_ON_STARTUP_SETTING, GDK_RUNTIME_INITIALIZE_ON_STARTUP_DEFAULT);
    register_bool_setting(GDK_RUNTIME_EMBED_DISPATCH_SETTING, GDK_RUNTIME_EMBED_DISPATCH_DEFAULT);
    register_bool_setting(GDK_RUNTIME_AUTO_ADD_PRIMARY_USER_SETTING, GDK_RUNTIME_AUTO_ADD_PRIMARY_USER_DEFAULT);
    register_bool_setting(GDK_TESTS_LIVE_REQUIRED_SETTING, GDK_TESTS_LIVE_REQUIRED_DEFAULT);
}

bool is_embed_dispatch_enabled() {
    ProjectSettings *project_settings = ProjectSettings::get_singleton();
    if (project_settings == nullptr) {
        return GDK_RUNTIME_EMBED_DISPATCH_DEFAULT;
    }

    return static_cast<bool>(project_settings->get_setting(
            GDK_RUNTIME_EMBED_DISPATCH_SETTING,
            GDK_RUNTIME_EMBED_DISPATCH_DEFAULT));
}

#if GODOT_VERSION_MINOR >= 5
void gdk_frame_callback() {
    if (gdk_singleton == nullptr || !gdk_singleton->is_initialized() || !is_embed_dispatch_enabled()) {
        return;
    }

    gdk_singleton->dispatch();
}
#endif

} // namespace

void initialize_gdk_extension(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        ClassDB::register_abstract_class<GDK>();
        ClassDB::register_class<GDKResult>();
        ClassDB::register_internal_class<GDKPendingSignal>();
        ClassDB::register_class<GDKUser>();
        ClassDB::register_class<GDKUsers>();
        ClassDB::register_class<GDKGameUI>();
        ClassDB::register_class<GDKClosedCaptionProperties>();
        ClassDB::register_class<GDKAccessibility>();
        ClassDB::register_class<GDKAchievement>();
        ClassDB::register_class<GDKAchievements>();
        ClassDB::register_class<GDKPresenceRecord>();
        ClassDB::register_class<GDKPresence>();
        ClassDB::register_class<GDKSocialFilter>();
        ClassDB::register_class<GDKSocialGroup>();
        ClassDB::register_class<GDKSocialUser>();
        ClassDB::register_class<GDKSocial>();
        ClassDB::register_class<GDKLauncher>();
        ClassDB::register_class<GDKMultiplayerActivityInfo>();
        ClassDB::register_class<GDKMultiplayerActivity>();
        ClassDB::register_class<GDKSystem>();

        gdk_singleton = memnew(GDK);
        Engine::get_singleton()->register_singleton("GDK", GDK::get_singleton());
        register_gdk_project_settings();
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
#if GODOT_VERSION_MINOR >= 5
    init_obj.register_frame_callback(gdk_frame_callback);
#endif
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}

} // extern "C"
