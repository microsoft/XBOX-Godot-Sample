#include "xbox.h"

#include "xbox_accessibility.h"
#include "xbox_achievement.h"
#include "xbox_activation.h"
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
#include "xbox_package.h"
#include "xbox_presence.h"
#include "xbox_profile.h"
#include "xbox_privacy.h"
#include "xbox_result.h"
#include "xbox_runtime.h"
#include "xbox_social.h"
#include "xbox_speech_synthesizer.h"
#include "xbox_stats.h"
#include "xbox_store.h"
#include "xbox_string_verify.h"
#include "xbox_system.h"
#include "xbox_title_storage.h"
#include "xbox_user.h"
#include "xbox_services.h"

#include <iterator>
#include <vector>

namespace godot {

Xbox *Xbox::singleton = nullptr;

Xbox *Xbox::get_singleton() {
    return singleton;
}

Xbox::Xbox() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;

    m_runtime = new XboxRuntime();
    m_xbox_services = new XboxServices();
    m_users.instantiate();
    m_users->set_owner(this);
    m_game_ui.instantiate();
    m_game_ui->set_owner(this);
    m_accessibility.instantiate();
    m_accessibility->set_owner(this);
    m_achievements.instantiate();
    m_achievements->set_owner(this);
    m_package.instantiate();
    m_package->set_owner(this);
    m_stats.instantiate();
    m_stats->set_owner(this);
    m_leaderboards.instantiate();
    m_leaderboards->set_owner(this);
    m_privacy.instantiate();
    m_privacy->set_owner(this);
    m_presence.instantiate();
    m_presence->set_owner(this);
    m_social.instantiate();
    m_social->set_owner(this);
    m_store.instantiate();
    m_store->set_owner(this);
    m_profile.instantiate();
    m_profile->set_owner(this);
    m_string_verify.instantiate();
    m_string_verify->set_owner(this);
    m_title_storage.instantiate();
    m_title_storage->set_owner(this);
    m_error_reporting.instantiate();
    m_error_reporting->set_owner(this);
    m_launcher.instantiate();
    m_launcher->set_owner(this);
    m_multiplayer_activity.instantiate();
    m_multiplayer_activity->set_owner(this);
    m_capture.instantiate();
    m_capture->set_owner(this);
    m_system.instantiate();
    m_system->set_owner(this);
    m_display.instantiate();
    m_display->set_owner(this);
    m_activation.instantiate();
    m_activation->set_owner(this);
    m_speech.instantiate();
    m_speech->set_owner(this);
    m_events.instantiate();
    m_events->set_owner(this);
    m_game_save.instantiate();
    m_game_save->set_owner(this);
    m_game_chat.instantiate();
    m_game_chat->set_owner(this);
}

Xbox::~Xbox() {
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
    m_package.unref();
    m_stats.unref();
    m_leaderboards.unref();
    m_privacy.unref();
    m_presence.unref();
    m_social.unref();
    m_store.unref();
    m_profile.unref();
    m_string_verify.unref();
    m_title_storage.unref();
    m_error_reporting.unref();
    m_launcher.unref();
    m_multiplayer_activity.unref();
    m_capture.unref();
    m_system.unref();
    m_display.unref();
    m_activation.unref();
    m_speech.unref();
    m_events.unref();
    m_game_save.unref();
    m_game_chat.unref();
    singleton = nullptr;
}

void Xbox::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "config"), &Xbox::initialize, DEFVAL(Variant()));
    ClassDB::bind_method(D_METHOD("shutdown"), &Xbox::shutdown);
    ClassDB::bind_method(D_METHOD("is_available"), &Xbox::is_available);
    ClassDB::bind_method(D_METHOD("is_initialized"), &Xbox::is_initialized);
    ClassDB::bind_method(D_METHOD("dispatch"), &Xbox::dispatch);
    ClassDB::bind_method(D_METHOD("get_users"), &Xbox::get_users);
    ClassDB::bind_method(D_METHOD("get_game_ui"), &Xbox::get_game_ui);
    ClassDB::bind_method(D_METHOD("get_accessibility"), &Xbox::get_accessibility);
    ClassDB::bind_method(D_METHOD("get_achievements"), &Xbox::get_achievements);
    ClassDB::bind_method(D_METHOD("get_package"), &Xbox::get_package);
    ClassDB::bind_method(D_METHOD("get_stats"), &Xbox::get_stats);
    ClassDB::bind_method(D_METHOD("get_leaderboards"), &Xbox::get_leaderboards);
    ClassDB::bind_method(D_METHOD("get_privacy"), &Xbox::get_privacy);
    ClassDB::bind_method(D_METHOD("get_presence"), &Xbox::get_presence);
    ClassDB::bind_method(D_METHOD("get_social"), &Xbox::get_social);
    ClassDB::bind_method(D_METHOD("get_store"), &Xbox::get_store);
    ClassDB::bind_method(D_METHOD("get_profile"), &Xbox::get_profile);
    ClassDB::bind_method(D_METHOD("get_string_verify"), &Xbox::get_string_verify);
    ClassDB::bind_method(D_METHOD("get_title_storage"), &Xbox::get_title_storage);
    ClassDB::bind_method(D_METHOD("get_error_reporting"), &Xbox::get_error_reporting);
    ClassDB::bind_method(D_METHOD("get_launcher"), &Xbox::get_launcher);
    ClassDB::bind_method(D_METHOD("get_multiplayer_activity"), &Xbox::get_multiplayer_activity);
    ClassDB::bind_method(D_METHOD("get_capture"), &Xbox::get_capture);
    ClassDB::bind_method(D_METHOD("get_system"), &Xbox::get_system);
    ClassDB::bind_method(D_METHOD("get_display"), &Xbox::get_display);
    ClassDB::bind_method(D_METHOD("get_activation"), &Xbox::get_activation);
    ClassDB::bind_method(D_METHOD("get_speech"), &Xbox::get_speech);
    ClassDB::bind_method(D_METHOD("get_events"), &Xbox::get_events);
    ClassDB::bind_method(D_METHOD("get_game_save"), &Xbox::get_game_save);
    ClassDB::bind_method(D_METHOD("get_game_chat"), &Xbox::get_game_chat);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "users", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxUsers"), "", "get_users");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "game_ui", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxGameUI"), "", "get_game_ui");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "accessibility", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxAccessibility"), "", "get_accessibility");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "achievements", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxAchievements"), "", "get_achievements");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "package", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxPackage"), "", "get_package");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stats", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxStats"), "", "get_stats");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "leaderboards", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxLeaderboards"), "", "get_leaderboards");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "privacy", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxPrivacy"), "", "get_privacy");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "presence", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxPresence"), "", "get_presence");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "social", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxSocial"), "", "get_social");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "store", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxStore"), "", "get_store");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "profile", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxProfile"), "", "get_profile");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "string_verify", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxStringVerify"), "", "get_string_verify");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "title_storage", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxTitleStorage"), "", "get_title_storage");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "error_reporting", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxErrorReporting"), "", "get_error_reporting");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "launcher", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxLauncher"), "", "get_launcher");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "multiplayer_activity", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxMultiplayerActivity"), "", "get_multiplayer_activity");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "capture", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxCapture"), "", "get_capture");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "system", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxSystem"), "", "get_system");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "display", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxDisplay"), "", "get_display");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "activation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxActivation"), "", "get_activation");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "speech", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxSpeechSynthesizer"), "", "get_speech");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "events", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxEvents"), "", "get_events");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "game_save", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxGameSave"), "", "get_game_save");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "game_chat", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "XboxGameChat"), "", "get_game_chat");

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
    ADD_SIGNAL(MethodInfo("runtime_error", PropertyInfo(Variant::OBJECT, "result")));
}

namespace {

struct XboxInitStep {
    Ref<XboxResult> (*init)(Xbox *);
    void (*shutdown)(Xbox *);
};

const XboxInitStep INIT_STEPS[] = {
    { [](Xbox *g) { return g->get_users()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_users()->shutdown(); } },
    { [](Xbox *g) { return g->get_game_ui()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_game_ui()->shutdown(); } },
    { [](Xbox *g) { return g->get_achievements()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_achievements()->shutdown(); } },
    { [](Xbox *g) { return g->get_package()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_package()->shutdown(); } },
    { [](Xbox *g) { return g->get_stats()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_stats()->shutdown(); } },
    { [](Xbox *g) { return g->get_leaderboards()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_leaderboards()->shutdown(); } },
    { [](Xbox *g) { return g->get_privacy()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_privacy()->shutdown(); } },
    { [](Xbox *g) { return g->get_presence()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_presence()->shutdown(); } },
    { [](Xbox *g) { return g->get_social()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_social()->shutdown(); } },
    { [](Xbox *g) { return g->get_store()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_store()->shutdown(); } },
    { [](Xbox *g) { return g->get_profile()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_profile()->shutdown(); } },
    { [](Xbox *g) { return g->get_string_verify()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_string_verify()->shutdown(); } },
    { [](Xbox *g) { return g->get_title_storage()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_title_storage()->shutdown(); } },
    { [](Xbox *g) { return g->get_error_reporting()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_error_reporting()->shutdown(); } },
    { [](Xbox *g) { return g->get_launcher()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_launcher()->shutdown(); } },
    { [](Xbox *g) { return g->get_activation()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_activation()->shutdown(); } },
    { [](Xbox *g) { return g->get_multiplayer_activity()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_multiplayer_activity()->shutdown(); } },
    { [](Xbox *g) { return g->get_capture()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_capture()->shutdown(); } },
    { [](Xbox *g) { return g->get_display()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_display()->shutdown(); } },
    { [](Xbox *g) { return g->get_speech()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_speech()->shutdown(); } },
    { [](Xbox *g) { return g->get_events()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_events()->shutdown(); } },
    { [](Xbox *g) { return g->get_game_save()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_game_save()->shutdown(); } },
    { [](Xbox *g) { return g->get_game_chat()->on_runtime_initialized(); },
      [](Xbox *g) { g->get_game_chat()->shutdown(); } },
};

struct XboxDispatchStep {
    int (*dispatch)(Xbox *);
};

const XboxDispatchStep DISPATCH_STEPS[] = {
    { [](Xbox *g) { return g->get_runtime()->dispatch(); } },
    { [](Xbox *g) { return g->get_achievements()->dispatch(); } },
    { [](Xbox *g) { return g->get_stats()->dispatch(); } },
    { [](Xbox *g) { return g->get_privacy()->dispatch(); } },
    { [](Xbox *g) { return g->get_presence()->dispatch(); } },
    { [](Xbox *g) { return g->get_social()->dispatch(); } },
    { [](Xbox *g) { return g->get_error_reporting()->dispatch(); } },
    { [](Xbox *g) { return g->get_game_chat()->dispatch(); } },
};

class XboxShutdownGuard {
public:
    explicit XboxShutdownGuard(Xbox *p_gdk) :
            m_gdk(p_gdk) {}

    ~XboxShutdownGuard() {
        if (m_committed || m_gdk == nullptr) {
            return;
        }
        for (auto it = m_shutdowns.rbegin(); it != m_shutdowns.rend(); ++it) {
            (*it)(m_gdk);
        }
    }

    void push(void (*p_shutdown)(Xbox *)) {
        m_shutdowns.push_back(p_shutdown);
    }

    void commit() {
        m_committed = true;
    }

private:
    Xbox *m_gdk = nullptr;
    std::vector<void (*)(Xbox *)> m_shutdowns;
    bool m_committed = false;
};

} // namespace

Ref<XboxResult> Xbox::initialize(const Variant &p_config) {
    Ref<XboxResult> runtime_result = m_runtime->initialize();
    if (!runtime_result->is_ok()) {
        return runtime_result;
    }

    XboxShutdownGuard guard(this);
    guard.push([](Xbox *g) { g->get_runtime()->shutdown(); });

    Ref<XboxResult> xbox_services_result = m_xbox_services->initialize(m_runtime->get_task_queue(), p_config);
    if (!xbox_services_result->is_ok()) {
        if (xbox_services_result->get_code() != "xbox_title_id_unavailable") {
            return xbox_services_result;
        }
    }
    guard.push([](Xbox *g) { g->get_xbox_services()->shutdown(); });

    for (const auto &step : INIT_STEPS) {
        Ref<XboxResult> result = step.init(this);
        if (!result->is_ok()) {
            return result;
        }
        guard.push(step.shutdown);
    }

    guard.commit();
    emit_signal("initialized");
    return XboxResult::ok_result();
}

void Xbox::shutdown() {
    if (!m_runtime->is_initialized()) {
        return;
    }

    for (auto it = std::rbegin(INIT_STEPS); it != std::rend(INIT_STEPS); ++it) {
        it->shutdown(this);
    }
    m_xbox_services->shutdown();
    m_runtime->shutdown();

    emit_signal("shutdown_completed");
}

bool Xbox::is_available() const {
    return m_runtime->is_available();
}

bool Xbox::is_initialized() const {
    return m_runtime->is_initialized();
}

int64_t Xbox::dispatch() {
    int64_t total = 0;
    for (const auto &step : DISPATCH_STEPS) {
        total += static_cast<int64_t>(step.dispatch(this));
    }
    return total;
}

Ref<XboxUsers> Xbox::get_users() const {
    return m_users;
}

Ref<XboxGameUI> Xbox::get_game_ui() const {
    return m_game_ui;
}

Ref<XboxAccessibility> Xbox::get_accessibility() const {
    return m_accessibility;
}

Ref<XboxAchievements> Xbox::get_achievements() const {
    return m_achievements;
}

Ref<XboxPackage> Xbox::get_package() const {
    return m_package;
}

Ref<XboxStats> Xbox::get_stats() const {
    return m_stats;
}

Ref<XboxLeaderboards> Xbox::get_leaderboards() const {
    return m_leaderboards;
}

Ref<XboxPrivacy> Xbox::get_privacy() const {
    return m_privacy;
}

Ref<XboxPresence> Xbox::get_presence() const {
    return m_presence;
}

Ref<XboxSocial> Xbox::get_social() const {
    return m_social;
}

Ref<XboxStore> Xbox::get_store() const {
    return m_store;
}

Ref<XboxProfile> Xbox::get_profile() const {
    return m_profile;
}

Ref<XboxStringVerify> Xbox::get_string_verify() const {
    return m_string_verify;
}

Ref<XboxTitleStorage> Xbox::get_title_storage() const {
    return m_title_storage;
}

Ref<XboxErrorReporting> Xbox::get_error_reporting() const {
    return m_error_reporting;
}

Ref<XboxLauncher> Xbox::get_launcher() const {
    return m_launcher;
}

Ref<XboxMultiplayerActivity> Xbox::get_multiplayer_activity() const {
    return m_multiplayer_activity;
}

Ref<XboxCapture> Xbox::get_capture() const {
    return m_capture;
}

Ref<XboxSystem> Xbox::get_system() const {
    return m_system;
}

Ref<XboxDisplay> Xbox::get_display() const {
    return m_display;
}

Ref<XboxActivation> Xbox::get_activation() const {
    return m_activation;
}

Ref<XboxSpeechSynthesizer> Xbox::get_speech() const {
    return m_speech;
}

Ref<XboxEvents> Xbox::get_events() const {
    return m_events;
}

Ref<XboxGameSave> Xbox::get_game_save() const {
    return m_game_save;
}

Ref<XboxGameChat> Xbox::get_game_chat() const {
    return m_game_chat;
}

XboxRuntime *Xbox::get_runtime() const {
    return m_runtime;
}

XboxServices *Xbox::get_xbox_services() const {
    return m_xbox_services;
}

void Xbox::emit_runtime_error(const Ref<XboxResult> &p_result) {
    emit_signal("runtime_error", p_result);
}

void Xbox::notify_user_removed(const Ref<XboxUser> &p_user) {
    if (!p_user.is_valid()) {
        return;
    }

    if (m_achievements.is_valid()) {
        m_achievements->on_user_removed(p_user);
    }
    if (m_stats.is_valid()) {
        m_stats->on_user_removed(p_user);
    }
    if (m_leaderboards.is_valid()) {
        m_leaderboards->on_user_removed(p_user);
    }
    if (m_privacy.is_valid()) {
        m_privacy->on_user_removed(p_user);
    }
    if (m_presence.is_valid()) {
        m_presence->on_user_removed(p_user);
    }
    if (m_social.is_valid()) {
        m_social->on_user_removed(p_user);
    }
    if (m_store.is_valid()) {
        m_store->on_user_removed(p_user);
    }
    if (m_profile.is_valid()) {
        m_profile->on_user_removed(p_user);
    }
    if (m_string_verify.is_valid()) {
        m_string_verify->on_user_removed(p_user);
    }
    if (m_title_storage.is_valid()) {
        m_title_storage->on_user_removed(p_user);
    }
    if (m_multiplayer_activity.is_valid()) {
        m_multiplayer_activity->on_user_removed(p_user);
    }
    if (m_game_chat.is_valid()) {
        m_game_chat->on_user_removed(p_user);
    }
    if (m_xbox_services != nullptr) {
        XUserLocalId local_id = {};
        local_id.value = static_cast<uint64_t>(p_user->get_local_id());
        m_xbox_services->forget_user(local_id);
    }
}

} // namespace godot
