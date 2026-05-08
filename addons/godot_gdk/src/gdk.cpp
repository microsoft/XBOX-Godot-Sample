#include "gdk.h"

#include "gdk_multiplayer_activity.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_system.h"
#include "gdk_xbox_services.h"

namespace godot {

GDK *GDK::singleton = nullptr;

GDK *GDK::get_singleton() {
    return singleton;
}

GDK::GDK() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;

    m_runtime = new GDKRuntime();
    m_xbox_services = new GDKXboxServices();
    m_users.instantiate();
    m_users->set_owner(this);
    m_game_ui.instantiate();
    m_game_ui->set_owner(this);
    m_accessibility.instantiate();
    m_accessibility->set_owner(this);
    m_achievements.instantiate();
    m_achievements->set_owner(this);
    m_presence.instantiate();
    m_presence->set_owner(this);
    m_social.instantiate();
    m_social->set_owner(this);
    m_launcher.instantiate();
    m_launcher->set_owner(this);
    m_multiplayer_activity.instantiate();
    m_multiplayer_activity->set_owner(this);
    m_system.instantiate();
    m_system->set_owner(this);
}

GDK::~GDK() {
    shutdown();

    if (m_xbox_services != nullptr) {
        delete m_xbox_services;
        m_xbox_services = nullptr;
    }

    if (m_runtime != nullptr) {
        delete m_runtime;
        m_runtime = nullptr;
    }

    m_users.unref();
    m_game_ui.unref();
    m_accessibility.unref();
    m_achievements.unref();
    m_presence.unref();
    m_social.unref();
    m_launcher.unref();
    m_multiplayer_activity.unref();
    m_system.unref();
    singleton = nullptr;
}

void GDK::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "config"), &GDK::initialize, DEFVAL(Variant()));
    ClassDB::bind_method(D_METHOD("shutdown"), &GDK::shutdown);
    ClassDB::bind_method(D_METHOD("is_available"), &GDK::is_available);
    ClassDB::bind_method(D_METHOD("is_initialized"), &GDK::is_initialized);
    ClassDB::bind_method(D_METHOD("dispatch"), &GDK::dispatch);
    ClassDB::bind_method(D_METHOD("get_last_error"), &GDK::get_last_error);
    ClassDB::bind_method(D_METHOD("get_users"), &GDK::get_users);
    ClassDB::bind_method(D_METHOD("get_game_ui"), &GDK::get_game_ui);
    ClassDB::bind_method(D_METHOD("get_accessibility"), &GDK::get_accessibility);
    ClassDB::bind_method(D_METHOD("get_achievements"), &GDK::get_achievements);
    ClassDB::bind_method(D_METHOD("get_presence"), &GDK::get_presence);
    ClassDB::bind_method(D_METHOD("get_social"), &GDK::get_social);
    ClassDB::bind_method(D_METHOD("get_launcher"), &GDK::get_launcher);
    ClassDB::bind_method(D_METHOD("get_multiplayer_activity"), &GDK::get_multiplayer_activity);
    ClassDB::bind_method(D_METHOD("get_system"), &GDK::get_system);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "users", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKUsers"), "", "get_users");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "game_ui", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKGameUI"), "", "get_game_ui");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "accessibility", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKAccessibility"), "", "get_accessibility");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "achievements", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKAchievements"), "", "get_achievements");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "presence", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKPresence"), "", "get_presence");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "social", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKSocial"), "", "get_social");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "launcher", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKLauncher"), "", "get_launcher");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "multiplayer_activity", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKMultiplayerActivity"), "", "get_multiplayer_activity");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "system", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "GDKSystem"), "", "get_system");

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
    ADD_SIGNAL(MethodInfo("runtime_error", PropertyInfo(Variant::OBJECT, "result")));
    ADD_SIGNAL(MethodInfo("availability_changed", PropertyInfo(Variant::BOOL, "available")));
}

Ref<GDKResult> GDK::initialize(const Variant &p_config) {
    (void)p_config;

    Ref<GDKResult> runtime_result = m_runtime->initialize();
    if (!runtime_result->is_ok()) {
        emit_runtime_error(runtime_result);
        return runtime_result;
    }

    Ref<GDKResult> xbox_services_result = m_xbox_services->initialize(m_runtime->get_task_queue(), p_config);
    if (!xbox_services_result->is_ok()) {
        if (xbox_services_result->get_code() == "xbox_title_id_unavailable") {
            emit_runtime_error(xbox_services_result);
        } else {
            emit_runtime_error(xbox_services_result);
            m_xbox_services->shutdown();
            m_runtime->shutdown();
            return xbox_services_result;
        }
    }

    Ref<GDKResult> users_result = m_users->on_runtime_initialized();
    if (!users_result->is_ok()) {
        emit_runtime_error(users_result);
        m_users->shutdown();
        m_xbox_services->shutdown();
        m_runtime->shutdown();
        return users_result;
    }

    Ref<GDKResult> game_ui_result = m_game_ui->on_runtime_initialized();
    if (!game_ui_result->is_ok()) {
        emit_runtime_error(game_ui_result);
        m_game_ui->shutdown();
        m_users->shutdown();
        m_xbox_services->shutdown();
        m_runtime->shutdown();
        return game_ui_result;
    }

    Ref<GDKResult> achievements_result = m_achievements->on_runtime_initialized();
    if (!achievements_result->is_ok()) {
        emit_runtime_error(achievements_result);
        m_achievements->shutdown();
        m_game_ui->shutdown();
        m_users->shutdown();
        m_xbox_services->shutdown();
        m_runtime->shutdown();
        return achievements_result;
    }

    Ref<GDKResult> presence_result = m_presence->on_runtime_initialized();
    if (!presence_result->is_ok()) {
        emit_runtime_error(presence_result);
        m_presence->shutdown();
        m_achievements->shutdown();
        m_game_ui->shutdown();
        m_users->shutdown();
        m_xbox_services->shutdown();
        m_runtime->shutdown();
        return presence_result;
    }

    Ref<GDKResult> social_result = m_social->on_runtime_initialized();
    if (!social_result->is_ok()) {
        emit_runtime_error(social_result);
        m_social->shutdown();
        m_presence->shutdown();
        m_achievements->shutdown();
        m_game_ui->shutdown();
        m_users->shutdown();
        m_xbox_services->shutdown();
        m_runtime->shutdown();
        return social_result;
    }

    Ref<GDKResult> launcher_result = m_launcher->on_runtime_initialized();
    if (!launcher_result->is_ok()) {
        emit_runtime_error(launcher_result);
        m_launcher->shutdown();
        m_social->shutdown();
        m_presence->shutdown();
        m_achievements->shutdown();
        m_users->shutdown();
        m_xbox_services->shutdown();
        m_runtime->shutdown();
        return launcher_result;
    }

    Ref<GDKResult> multiplayer_activity_result = m_multiplayer_activity->on_runtime_initialized();
    if (!multiplayer_activity_result->is_ok()) {
        emit_runtime_error(multiplayer_activity_result);
        m_multiplayer_activity->shutdown();
        m_launcher->shutdown();
        m_social->shutdown();
        m_presence->shutdown();
        m_achievements->shutdown();
        m_game_ui->shutdown();
        m_users->shutdown();
        m_xbox_services->shutdown();
        m_runtime->shutdown();
        return multiplayer_activity_result;
    }

    emit_signal("initialized");
    return GDKResult::ok_result();
}

void GDK::shutdown() {
    if (!m_runtime->is_initialized()) {
        return;
    }

    m_multiplayer_activity->shutdown();
    m_social->shutdown();
    m_launcher->shutdown();
    m_presence->shutdown();
    m_achievements->shutdown();
    m_game_ui->shutdown();
    m_users->shutdown();
    m_xbox_services->shutdown();
    m_runtime->shutdown();

    emit_signal("shutdown_completed");
}

bool GDK::is_available() const {
    return m_runtime->is_available();
}

bool GDK::is_initialized() const {
    return m_runtime->is_initialized();
}

int64_t GDK::dispatch() {
    return static_cast<int64_t>(m_runtime->dispatch() + m_achievements->dispatch() + m_social->dispatch());
}

Ref<GDKResult> GDK::get_last_error() const {
    return m_runtime->get_last_error();
}

Ref<GDKUsers> GDK::get_users() const {
    return m_users;
}

Ref<GDKGameUI> GDK::get_game_ui() const {
    return m_game_ui;
}

Ref<GDKAccessibility> GDK::get_accessibility() const {
    return m_accessibility;
}

Ref<GDKAchievements> GDK::get_achievements() const {
    return m_achievements;
}

Ref<GDKPresence> GDK::get_presence() const {
    return m_presence;
}

Ref<GDKSocial> GDK::get_social() const {
    return m_social;
}

Ref<GDKLauncher> GDK::get_launcher() const {
    return m_launcher;
}

Ref<GDKMultiplayerActivity> GDK::get_multiplayer_activity() const {
    return m_multiplayer_activity;
}

Ref<GDKSystem> GDK::get_system() const {
    return m_system;
}

GDKRuntime *GDK::get_runtime() const {
    return m_runtime;
}

GDKXboxServices *GDK::get_xbox_services() const {
    return m_xbox_services;
}

void GDK::emit_runtime_error(const Ref<GDKResult> &p_result) {
    m_runtime->set_last_error(p_result);
    emit_signal("runtime_error", p_result);
}

void GDK::notify_user_removed(const Ref<GDKUser> &p_user) {
    if (!p_user.is_valid()) {
        return;
    }

    if (m_achievements.is_valid()) {
        m_achievements->on_user_removed(p_user);
    }
    if (m_presence.is_valid()) {
        m_presence->on_user_removed(p_user);
    }
    if (m_social.is_valid()) {
        m_social->on_user_removed(p_user);
    }
    if (m_multiplayer_activity.is_valid()) {
        m_multiplayer_activity->on_user_removed(p_user);
    }
    if (m_xbox_services != nullptr) {
        XUserLocalId local_id = {};
        local_id.value = static_cast<uint64_t>(p_user->get_local_id());
        m_xbox_services->forget_user(local_id);
    }
}

} // namespace godot
