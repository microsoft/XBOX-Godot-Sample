#include "register_types.h"

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/version.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "xbox_achievement.h"
#include "xbox_accessibility.h"
#include "xbox_activation.h"
#include "xbox.h"
#include "xbox_capture.h"
#include "xbox_display.h"
#include "xbox_error_reporting.h"
#include "xbox_events.h"
#include "xbox_game_chat.h"
#include "xbox_game_save.h"
#include "xbox_game_ui.h"
#include "xbox_launcher.h"
#include "xbox_leaderboards.h"
#include "xbox_multiplayer_activity.h"
#include "xbox_networking.h"
#include "xbox_package.h"
#include "xbox_pending_signal.h"
#include "xbox_presence.h"
#include "xbox_profile.h"
#include "xbox_privacy.h"
#include "xbox_result.h"
#include "xbox_social.h"
#include "xbox_speech_synthesizer.h"
#include "xbox_stats.h"
#include "xbox_store.h"
#include "xbox_string_verify.h"
#include "xbox_system.h"
#include "xbox_title_storage.h"
#include "xbox_user.h"

using namespace godot;

static Xbox *gdk_singleton = nullptr;
static int gdk_extension_ref_count = 0;

namespace {

constexpr const char *GDK_RUNTIME_INITIALIZE_ON_STARTUP_SETTING = "gdk/runtime/initialize_on_startup";
constexpr bool GDK_RUNTIME_INITIALIZE_ON_STARTUP_DEFAULT = false;
constexpr const char *GDK_RUNTIME_EMBED_DISPATCH_SETTING = "gdk/runtime/embed_dispatch";
constexpr bool GDK_RUNTIME_EMBED_DISPATCH_DEFAULT = true;
constexpr const char *GDK_RUNTIME_AUTO_ADD_PRIMARY_USER_SETTING = "gdk/runtime/auto_add_primary_user";
constexpr bool GDK_RUNTIME_AUTO_ADD_PRIMARY_USER_DEFAULT = false;
constexpr const char *GDK_RUNTIME_SINGLETON_NAME_SETTING = "gdk/runtime/singleton_name";
constexpr const char *GDK_RUNTIME_SINGLETON_NAME_DEFAULT = "GDK";

// Name the singleton was actually registered under. Captured at initialize
// time so uninitialize unregisters the same name even if the project setting
// is edited while the extension is loaded. Function-local so the String is
// constructed on first use: a file-scope String would run its constructor from
// DllMain, before the GDExtension interface pointers exist, and fail the load.
String &registered_singleton_name() {
    static String name;
    return name;
}

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

void register_string_setting(const char *name, const String &default_value, bool basic) {
    ProjectSettings *project_settings = ProjectSettings::get_singleton();
    if (project_settings == nullptr) {
        return;
    }

    if (!project_settings->has_setting(name)) {
        project_settings->set_setting(name, default_value);
    }

    project_settings->set_initial_value(name, default_value);
    project_settings->set_as_basic(name, basic);

    Dictionary setting_info;
    setting_info["name"] = name;
    setting_info["type"] = Variant::STRING;
    setting_info["hint"] = PROPERTY_HINT_NONE;
    setting_info["hint_string"] = "";
    project_settings->add_property_info(setting_info);
}

void register_gdk_project_settings() {
    register_bool_setting(GDK_RUNTIME_INITIALIZE_ON_STARTUP_SETTING, GDK_RUNTIME_INITIALIZE_ON_STARTUP_DEFAULT);
    register_bool_setting(GDK_RUNTIME_EMBED_DISPATCH_SETTING, GDK_RUNTIME_EMBED_DISPATCH_DEFAULT);
    register_bool_setting(GDK_RUNTIME_AUTO_ADD_PRIMARY_USER_SETTING, GDK_RUNTIME_AUTO_ADD_PRIMARY_USER_DEFAULT);
    register_string_setting(GDK_RUNTIME_SINGLETON_NAME_SETTING, GDK_RUNTIME_SINGLETON_NAME_DEFAULT, false);
}

// Resolves the Engine singleton name from `gdk/runtime/singleton_name`,
// falling back to "GDK" when the configured value is unusable. Rejecting a bad
// value instead of honouring it keeps the addon reachable: an unregisterable
// name would leave every `GDK.*` call in a project unresolved with no clue why.
String resolve_gdk_singleton_name() {
    ProjectSettings *project_settings = ProjectSettings::get_singleton();
    if (project_settings == nullptr) {
        return String(GDK_RUNTIME_SINGLETON_NAME_DEFAULT);
    }

    const String configured = String(project_settings->get_setting(
                                             GDK_RUNTIME_SINGLETON_NAME_SETTING,
                                             GDK_RUNTIME_SINGLETON_NAME_DEFAULT))
                                      .strip_edges();
    if (configured.is_empty() || configured == GDK_RUNTIME_SINGLETON_NAME_DEFAULT) {
        return String(GDK_RUNTIME_SINGLETON_NAME_DEFAULT);
    }

    if (!configured.is_valid_ascii_identifier()) {
        UtilityFunctions::push_warning(
                String("[GDK] Project setting '") + GDK_RUNTIME_SINGLETON_NAME_SETTING + "' is not a valid identifier ('" +
                configured + "'). Falling back to '" + GDK_RUNTIME_SINGLETON_NAME_DEFAULT + "'.");
        return String(GDK_RUNTIME_SINGLETON_NAME_DEFAULT);
    }

    if (Engine::get_singleton()->has_singleton(configured)) {
        UtilityFunctions::push_warning(
                String("[GDK] Project setting '") + GDK_RUNTIME_SINGLETON_NAME_SETTING + "' requests '" + configured +
                "', which is already a registered singleton. Falling back to '" + GDK_RUNTIME_SINGLETON_NAME_DEFAULT + "'.");
        return String(GDK_RUNTIME_SINGLETON_NAME_DEFAULT);
    }

    return configured;
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
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ++gdk_extension_ref_count;
    if (gdk_extension_ref_count > 1) {
        return;
    }

    ClassDB::register_abstract_class<Xbox>();
    ClassDB::register_class<XboxResult>();
    ClassDB::register_internal_class<XboxPendingSignal>();
    ClassDB::register_class<XboxUser>();
    ClassDB::register_class<XboxUserSignOutDeferral>();
    ClassDB::register_class<XboxUsers>();
    ClassDB::register_class<XboxGameUI>();
    ClassDB::register_class<XboxClosedCaptionProperties>();
    ClassDB::register_class<XboxAccessibility>();
    ClassDB::register_class<XboxAchievement>();
    ClassDB::register_class<XboxAchievements>();
    ClassDB::register_class<XboxPackageMount>();
    ClassDB::register_class<XboxPackageResourcePack>();
    ClassDB::register_class<XboxPackage>();
    ClassDB::register_class<XboxStats>();
    ClassDB::register_class<XboxLeaderboardColumn>();
    ClassDB::register_class<XboxLeaderboardRow>();
    ClassDB::register_class<XboxLeaderboard>();
    ClassDB::register_class<XboxLeaderboards>();
    ClassDB::register_class<XboxPrivacy>();
    ClassDB::register_class<XboxPresenceRecord>();
    ClassDB::register_class<XboxPresence>();
    ClassDB::register_class<XboxSocialFilter>();
    ClassDB::register_class<XboxSocialGroup>();
    ClassDB::register_class<XboxSocialUser>();
    ClassDB::register_class<XboxSocial>();
    ClassDB::register_class<XboxStoreLicenseStatus>();
    ClassDB::register_class<XboxStore>();
    ClassDB::register_class<XboxUserProfile>();
    ClassDB::register_class<XboxProfile>();
    ClassDB::register_class<XboxStringVerify>();
    ClassDB::register_class<XboxTitleStorageBlobMetadata>();
    ClassDB::register_class<XboxTitleStorageBlobMetadataResult>();
    ClassDB::register_class<XboxTitleStorage>();
    ClassDB::register_class<XboxErrorReporting>();
    ClassDB::register_class<XboxLauncher>();
    ClassDB::register_class<XboxMultiplayerActivityInfo>();
    ClassDB::register_class<XboxMultiplayerActivity>();
    ClassDB::register_class<XboxCaptureMetaData>();
    ClassDB::register_class<XboxCapture>();
    ClassDB::register_class<XboxSystem>();
    ClassDB::register_class<XboxDisplayTimeoutDeferral>();
    ClassDB::register_class<XboxDisplay>();
    ClassDB::register_class<XboxActivation>();
    ClassDB::register_class<XboxSpeechSynthesizer>();
    ClassDB::register_class<XboxEvents>();
    ClassDB::register_class<XboxGameSave>();
    ClassDB::register_class<XboxGameChat>();
    ClassDB::register_class<XboxNetworkingSecurityInformation>();
    ClassDB::register_class<XboxNetworking>();

    gdk_singleton = memnew(Xbox);
    register_gdk_project_settings();
    registered_singleton_name() = resolve_gdk_singleton_name();
    Engine::get_singleton()->register_singleton(registered_singleton_name(), Xbox::get_singleton());
}

void uninitialize_gdk_extension(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE || gdk_extension_ref_count == 0) {
        return;
    }

    --gdk_extension_ref_count;
    if (gdk_extension_ref_count > 0) {
        return;
    }

    if (!registered_singleton_name().is_empty()) {
        Engine::get_singleton()->unregister_singleton(registered_singleton_name());
        registered_singleton_name() = String();
    }

    if (gdk_singleton) {
        memdelete(gdk_singleton);
        gdk_singleton = nullptr;
    }
}

extern "C" {

GDExtensionBool GDE_EXPORT gdk_addon_init(
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
    // The per-frame dispatch pump is registered with the engine only during
    // CORE-level init (godot-cpp's register_main_loop_callbacks). On editor
    // hot-reload the engine re-initializes an extension from its declared minimum
    // level upward, so a SCENE minimum would skip CORE and silently drop the pump,
    // leaving every await *_async() hung with no notice. Declaring CORE instead makes
    // the engine return LOAD_STATUS_NEEDS_RESTART on reload (an explicit "restart
    // required" prompt); cold start/export are unaffected (first load always inits
    // from CORE regardless of this minimum).
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_CORE);

    return init_obj.init();
}

} // extern "C"
