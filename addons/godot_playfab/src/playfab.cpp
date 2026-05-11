#include "playfab.h"

#include "playfab_gamesaves.h"
#include "playfab_leaderboards.h"
#include "playfab_result.h"
#include "playfab_runtime.h"
#include "playfab_user.h"
#include "playfab_users.h"

namespace godot {

PlayFab *PlayFab::singleton = nullptr;

PlayFab *PlayFab::get_singleton() {
    return singleton;
}

PlayFab::PlayFab() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;

    m_runtime = new PlayFabRuntime();

    m_users.instantiate();
    m_users->set_owner(this);

    m_game_saves.instantiate();
    m_game_saves->set_owner(this);

    m_leaderboards.instantiate();
    m_leaderboards->set_owner(this);
}

PlayFab::~PlayFab() {
    shutdown();

    m_users.unref();
    m_game_saves.unref();
    m_leaderboards.unref();

    if (m_runtime != nullptr) {
        delete m_runtime;
        m_runtime = nullptr;
    }

    singleton = nullptr;
}

void PlayFab::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &PlayFab::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &PlayFab::shutdown);
    ClassDB::bind_method(D_METHOD("is_available"), &PlayFab::is_available);
    ClassDB::bind_method(D_METHOD("is_initialized"), &PlayFab::is_initialized);
    ClassDB::bind_method(D_METHOD("dispatch"), &PlayFab::dispatch);
    ClassDB::bind_method(D_METHOD("get_last_error"), &PlayFab::get_last_error);
    ClassDB::bind_method(D_METHOD("get_users"), &PlayFab::get_users);
    ClassDB::bind_method(D_METHOD("get_game_saves"), &PlayFab::get_game_saves);
    ClassDB::bind_method(D_METHOD("get_leaderboards"), &PlayFab::get_leaderboards);
    ClassDB::bind_method(D_METHOD("sign_in_with_xuser_async", "user", "create_account"), &PlayFab::sign_in_with_xuser_async, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("sign_in_with_custom_id_async", "custom_id", "create_account"), &PlayFab::sign_in_with_custom_id_async, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_user_by_local_id", "local_id"), &PlayFab::get_user_by_local_id);
    ClassDB::bind_method(D_METHOD("get_user_by_custom_id", "custom_id"), &PlayFab::get_user_by_custom_id);
    ClassDB::bind_method(D_METHOD("get_title_id"), &PlayFab::get_title_id);
    ClassDB::bind_method(D_METHOD("get_endpoint"), &PlayFab::get_endpoint);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "users", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabUsers"), "", "get_users");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "game_saves", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabGameSaves"), "", "get_game_saves");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "leaderboards", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabLeaderboards"), "", "get_leaderboards");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "title_id"), "", "get_title_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "endpoint"), "", "get_endpoint");

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
    ADD_SIGNAL(MethodInfo("runtime_error", PropertyInfo(Variant::OBJECT, "result")));
}

Ref<PlayFabResult> PlayFab::initialize() {
    Ref<PlayFabResult> runtime_result = m_runtime->initialize();
    if (!runtime_result->is_ok()) {
        emit_runtime_error(runtime_result);
        return runtime_result;
    }

    Ref<PlayFabResult> users_result = m_users->on_runtime_initialized();
    if (!users_result->is_ok()) {
        emit_runtime_error(users_result);
        m_users->shutdown();
        m_runtime->shutdown();
        return users_result;
    }

    emit_signal("initialized");
    return PlayFabResult::ok_result();
}

void PlayFab::shutdown() {
    if (m_runtime == nullptr || !m_runtime->is_initialized()) {
        return;
    }

    m_users->shutdown();
    m_runtime->shutdown();

    emit_signal("shutdown_completed");
}

bool PlayFab::is_available() const {
    return m_runtime != nullptr && m_runtime->is_available();
}

bool PlayFab::is_initialized() const {
    return m_runtime != nullptr && m_runtime->is_initialized();
}

int64_t PlayFab::dispatch() {
    return m_runtime != nullptr ? static_cast<int64_t>(m_runtime->dispatch()) : 0;
}

Ref<PlayFabResult> PlayFab::get_last_error() const {
    return m_runtime != nullptr ? m_runtime->get_last_error() : PlayFabResult::error_result(E_FAIL, "runtime_unavailable", "PlayFab runtime is unavailable.");
}

Ref<PlayFabUsers> PlayFab::get_users() const {
    return m_users;
}

Ref<PlayFabGameSaves> PlayFab::get_game_saves() const {
    return m_game_saves;
}

Ref<PlayFabLeaderboards> PlayFab::get_leaderboards() const {
    return m_leaderboards;
}

Signal PlayFab::sign_in_with_xuser_async(const Variant &p_user, bool p_create_account) {
    return m_users.is_valid() ? m_users->sign_in_with_xuser_async(p_user, p_create_account) : Signal();
}

Signal PlayFab::sign_in_with_custom_id_async(const String &p_custom_id, bool p_create_account) {
    return m_users.is_valid() ? m_users->sign_in_with_custom_id_async(p_custom_id, p_create_account) : Signal();
}

Ref<PlayFabUser> PlayFab::get_user_by_local_id(int64_t p_local_id) const {
    return m_users.is_valid() ? m_users->get_user_by_local_id(p_local_id) : Ref<PlayFabUser>();
}

Ref<PlayFabUser> PlayFab::get_user_by_custom_id(const String &p_custom_id) const {
    return m_users.is_valid() ? m_users->get_user_by_custom_id(p_custom_id) : Ref<PlayFabUser>();
}

String PlayFab::get_title_id() const {
    return m_runtime != nullptr ? m_runtime->get_title_id() : String();
}

String PlayFab::get_endpoint() const {
    return m_runtime != nullptr ? m_runtime->get_endpoint() : String();
}

PlayFabRuntime *PlayFab::get_runtime() const {
    return m_runtime;
}

void PlayFab::emit_runtime_error(const Ref<PlayFabResult> &p_result) {
    if (m_runtime != nullptr) {
        m_runtime->set_last_error(p_result);
    }
    emit_signal("runtime_error", p_result);
}

} // namespace godot
