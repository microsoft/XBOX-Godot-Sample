#include "playfab_users.h"

#include <list>
#include <string>
#include <vector>

#include <godot_cpp/classes/object.hpp>

#include <playfab/core/PFAuthentication.h>
#include <playfab/core/PFAuthenticationTypes.h>

#include "playfab.h"
#include "playfab_pending_signal.h"
#include "playfab_result.h"
#include "playfab_runtime.h"
#include "playfab_signal_xasync_context.h"
#include "playfab_user.h"

namespace godot {

namespace {

#if defined(HC_PLATFORM) && HC_PLATFORM == HC_PLATFORM_GDK
constexpr bool PLAYFAB_GDK_PLATFORM = true;
#else
constexpr bool PLAYFAB_GDK_PLATFORM = false;
#endif

Signal make_users_error_signal(
        PlayFabRuntime *p_runtime,
        HRESULT p_hresult,
        const String &p_code,
        const String &p_message,
        const Variant &p_data = Variant()) {
    ERR_FAIL_NULL_V(p_runtime, Signal());
    return p_runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
}

class SignInXUserAsyncContext final : public PlayFabSignalXAsyncContext {
    PlayFabUsers *m_users = nullptr;
    XUserHandle m_user_handle = nullptr;

public:
    SignInXUserAsyncContext(
            PlayFabUsers *p_users,
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            XUserHandle p_user_handle) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_users(p_users),
            m_user_handle(p_user_handle) {}

    ~SignInXUserAsyncContext() override {
        if (m_user_handle != nullptr) {
            XUserCloseHandle(m_user_handle);
            m_user_handle = nullptr;
        }
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled("PlayFab sign-in cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled("PlayFab sign-in cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "Failed to sign the Xbox user into PlayFab.", "playfab_sign_in_failed");
            get_pending_signal()->complete(result);
            return;
        }

        size_t buffer_size = 0;
        HRESULT size_hr = PFAuthenticationLoginWithXUserGetResultSize(p_async_block, &buffer_size);
        if (FAILED(size_hr)) {
            result = PlayFabResult::hresult_error(size_hr, "Failed to get the PlayFab login result size.", "playfab_sign_in_result_size_failed");
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<char> buffer(buffer_size);
        PFAuthenticationLoginResult const *login_result = nullptr;
        PFEntityHandle entity_handle = nullptr;
        HRESULT result_hr = PFAuthenticationLoginWithXUserGetResult(
                p_async_block,
                &entity_handle,
                buffer.size(),
                buffer.data(),
                &login_result,
                nullptr);
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, "Failed to retrieve the PlayFab login result.", "playfab_sign_in_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Ref<PlayFabUser> user;
        user.instantiate();

        HRESULT user_hr = user->adopt_session(m_user_handle, entity_handle, get_runtime()->get_service_config_handle());
        if (FAILED(user_hr)) {
            result = PlayFabResult::hresult_error(user_hr, "Failed to translate the PlayFab login result into a Godot user wrapper.", "playfab_user_wrapper_create_failed");
            get_pending_signal()->complete(result);
            return;
        }

        m_users->add_or_update_user_session(user);

        get_pending_signal()->complete(PlayFabResult::ok_result(user));
    }
};

class SignInCustomIDAsyncContext final : public PlayFabSignalXAsyncContext {
    PlayFabUsers *m_users = nullptr;
    String m_custom_id;
    std::string m_custom_id_utf8;

public:
    SignInCustomIDAsyncContext(
            PlayFabUsers *p_users,
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const String &p_custom_id) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_users(p_users),
            m_custom_id(p_custom_id.strip_edges()),
            m_custom_id_utf8(m_custom_id.utf8().get_data()) {}

    const char *get_custom_id_cstr() const {
        return m_custom_id_utf8.c_str();
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled("PlayFab custom-ID sign-in cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled("PlayFab custom-ID sign-in cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "Failed to sign the custom ID into PlayFab.", "playfab_custom_id_sign_in_failed");
            get_pending_signal()->complete(result);
            return;
        }

        size_t buffer_size = 0;
        HRESULT size_hr = PFAuthenticationLoginWithCustomIDGetResultSize(p_async_block, &buffer_size);
        if (FAILED(size_hr)) {
            result = PlayFabResult::hresult_error(size_hr, "Failed to get the PlayFab custom-ID login result size.", "playfab_custom_id_sign_in_result_size_failed");
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<char> buffer(buffer_size);
        PFAuthenticationLoginResult const *login_result = nullptr;
        PFEntityHandle entity_handle = nullptr;
        HRESULT result_hr = PFAuthenticationLoginWithCustomIDGetResult(
                p_async_block,
                &entity_handle,
                buffer.size(),
                buffer.data(),
                &login_result,
                nullptr);
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, "Failed to retrieve the PlayFab custom-ID login result.", "playfab_custom_id_sign_in_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Ref<PlayFabUser> user;
        user.instantiate();

        HRESULT user_hr = user->adopt_custom_id_session(m_custom_id, entity_handle);
        if (FAILED(user_hr)) {
            result = PlayFabResult::hresult_error(user_hr, "Failed to translate the PlayFab custom-ID login result into a Godot user wrapper.", "playfab_custom_id_user_wrapper_create_failed");
            get_pending_signal()->complete(result);
            return;
        }

        m_users->add_or_update_user_session(user);

        get_pending_signal()->complete(PlayFabResult::ok_result(user));
    }
};

// Shared completion handler for token-based PlayFab logins (Steam, OpenID
// Connect, Battle.net). Each of these flows yields only a PFEntityHandle, so
// the resulting PlayFabUser is keyed by its PlayFab entity id rather than a
// local Xbox user id or a title-defined custom id. The PlayFab result
// accessors share an identical signature, so the per-method size/result
// functions are injected as function pointers.
class SignInEntityAsyncContext final : public PlayFabSignalXAsyncContext {
public:
    using GetResultSizeFn = HRESULT (*)(XAsyncBlock *, size_t *);
    using GetResultFn = HRESULT (*)(XAsyncBlock *, PFEntityHandle *, size_t, void *, PFAuthenticationLoginResult const **, size_t *);

private:
    PlayFabUsers *m_users = nullptr;
    String m_method_label;
    String m_error_prefix;
    GetResultSizeFn m_get_result_size_fn = nullptr;
    GetResultFn m_get_result_fn = nullptr;
    std::list<std::string> m_interned_strings;
    std::list<bool> m_interned_bools;

public:
    SignInEntityAsyncContext(
            PlayFabUsers *p_users,
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const String &p_method_label,
            const String &p_error_prefix,
            GetResultSizeFn p_get_result_size_fn,
            GetResultFn p_get_result_fn) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_users(p_users),
            m_method_label(p_method_label),
            m_error_prefix(p_error_prefix),
            m_get_result_size_fn(p_get_result_size_fn),
            m_get_result_fn(p_get_result_fn) {}

    // Copies the supplied value into storage owned by this context so the C
    // string remains valid for the lifetime of the async request. std::list
    // never relocates its elements, so previously returned pointers stay valid
    // as additional values are interned.
    const char *intern_string(const String &p_value) {
        m_interned_strings.push_back(std::string(p_value.utf8().get_data()));
        return m_interned_strings.back().c_str();
    }

    const bool *intern_bool(bool p_value) {
        m_interned_bools.push_back(p_value);
        return &m_interned_bools.back();
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        const String cancel_message = "PlayFab " + m_method_label + " sign-in cancelled.";

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(PlayFabResult::cancelled(cancel_message));
            return;
        }

        HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            get_pending_signal()->complete(PlayFabResult::cancelled(cancel_message));
            return;
        }
        if (FAILED(status_hr)) {
            get_pending_signal()->complete(PlayFabResult::hresult_error(status_hr, "Failed to sign in to PlayFab with " + m_method_label + ".", m_error_prefix + "_sign_in_failed"));
            return;
        }

        size_t buffer_size = 0;
        HRESULT size_hr = m_get_result_size_fn(p_async_block, &buffer_size);
        if (FAILED(size_hr)) {
            get_pending_signal()->complete(PlayFabResult::hresult_error(size_hr, "Failed to get the PlayFab " + m_method_label + " login result size.", m_error_prefix + "_sign_in_result_size_failed"));
            return;
        }

        std::vector<char> buffer(buffer_size);
        PFAuthenticationLoginResult const *login_result = nullptr;
        PFEntityHandle entity_handle = nullptr;
        HRESULT result_hr = m_get_result_fn(
                p_async_block,
                &entity_handle,
                buffer.size(),
                buffer.data(),
                &login_result,
                nullptr);
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(PlayFabResult::hresult_error(result_hr, "Failed to retrieve the PlayFab " + m_method_label + " login result.", m_error_prefix + "_sign_in_result_failed"));
            return;
        }

        Ref<PlayFabUser> user;
        user.instantiate();

        HRESULT user_hr = user->adopt_entity_session(entity_handle);
        if (FAILED(user_hr)) {
            get_pending_signal()->complete(PlayFabResult::hresult_error(user_hr, "Failed to translate the PlayFab " + m_method_label + " login result into a Godot user wrapper.", m_error_prefix + "_user_wrapper_create_failed"));
            return;
        }

        m_users->add_or_update_user_session(user);

        get_pending_signal()->complete(PlayFabResult::ok_result(user));
    }
};

} // namespace

void PlayFabUsers::_bind_methods() {
    ClassDB::bind_method(D_METHOD("sign_in_with_xuser_async", "user", "create_account"), &PlayFabUsers::sign_in_with_xuser_async, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("sign_in_with_custom_id_async", "custom_id", "create_account"), &PlayFabUsers::sign_in_with_custom_id_async, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("sign_in_with_steam_async", "steam_ticket", "create_account", "ticket_is_service_specific"), &PlayFabUsers::sign_in_with_steam_async, DEFVAL(true), DEFVAL(false));
    ClassDB::bind_method(D_METHOD("sign_in_with_open_id_connect_async", "connection_id", "id_token", "create_account"), &PlayFabUsers::sign_in_with_open_id_connect_async, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("sign_in_with_battle_net_async", "identity_token", "create_account"), &PlayFabUsers::sign_in_with_battle_net_async, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_user_by_local_id", "local_id"), &PlayFabUsers::get_user_by_local_id);
    ClassDB::bind_method(D_METHOD("get_user_by_custom_id", "custom_id"), &PlayFabUsers::get_user_by_custom_id);
    ClassDB::bind_method(D_METHOD("get_user", "user_or_local_id"), &PlayFabUsers::get_user);
    ClassDB::bind_method(D_METHOD("get_users"), &PlayFabUsers::get_users);
}

void PlayFabUsers::set_owner(PlayFab *p_owner) {
    m_owner = p_owner;
}

Ref<PlayFabResult> PlayFabUsers::on_runtime_initialized() {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return PlayFabResult::error_result(E_FAIL, "runtime_not_initialized", "Cannot initialize the PlayFab users service before the PlayFab runtime.");
    }

    if (!PLAYFAB_GDK_PLATFORM) {
        return PlayFabResult::error_result(E_FAIL, "platform_unsupported", "PlayFab users are only supported on GDK platforms right now.");
    }
    return PlayFabResult::ok_result();
}

void PlayFabUsers::shutdown() {
    m_users.clear();
}

Signal PlayFabUsers::sign_in_with_xuser_async(Object *p_user, bool p_create_account) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_users_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    if (!PLAYFAB_GDK_PLATFORM) {
        return make_users_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab sign-in is only supported on GDK platforms right now.");
    }

    XUserLocalId local_id = {};
    String error_message;
    if (!_try_get_local_id_from_xuser_object(p_user, &local_id, &error_message)) {
        return make_users_error_signal(runtime, E_INVALIDARG, "invalid_xuser", error_message);
    }

    XUserHandle user_handle = nullptr;
    HRESULT hr = XUserFindUserByLocalId(local_id, &user_handle);
    if (FAILED(hr)) {
        return make_users_error_signal(runtime, hr, "xuser_not_found", "Failed to find an active XUserHandle for the provided GDK user.");
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new SignInXUserAsyncContext(this, runtime, pending_signal, user_handle);
    context->bind_cancel_handler();

    PFAuthenticationLoginWithXUserRequest request = {};
    request.createAccount = p_create_account;
    request.user = user_handle;

    hr = PFAuthenticationLoginWithXUserAsync(
            runtime->get_service_config_handle(),
            &request,
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the PlayFab XUser login request.", "playfab_sign_in_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabUsers::sign_in_with_custom_id_async(const String &p_custom_id, bool p_create_account) {
    PlayFabRuntime *runtime = _get_runtime();
    const String custom_id = p_custom_id.strip_edges();
    if (custom_id.is_empty()) {
        return make_users_error_signal(runtime, E_INVALIDARG, "invalid_custom_id", "PlayFab custom-ID sign-in requires a non-empty custom_id.");
    }

    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_users_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new SignInCustomIDAsyncContext(this, runtime, pending_signal, custom_id);
    context->bind_cancel_handler();

    PFAuthenticationLoginWithCustomIDRequest request = {};
    request.createAccount = p_create_account;
    request.customId = context->get_custom_id_cstr();

    HRESULT hr = PFAuthenticationLoginWithCustomIDAsync(
            runtime->get_service_config_handle(),
            &request,
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the PlayFab custom-ID login request.", "playfab_custom_id_sign_in_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabUsers::sign_in_with_steam_async(const String &p_steam_ticket, bool p_create_account, bool p_ticket_is_service_specific) {
    PlayFabRuntime *runtime = _get_runtime();
    const String steam_ticket = p_steam_ticket.strip_edges();
    if (steam_ticket.is_empty()) {
        return make_users_error_signal(runtime, E_INVALIDARG, "invalid_steam_ticket", "PlayFab Steam sign-in requires a non-empty steam_ticket.");
    }

    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_users_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    if (!PLAYFAB_GDK_PLATFORM) {
        return make_users_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab sign-in is only supported on GDK platforms right now.");
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new SignInEntityAsyncContext(
            this,
            runtime,
            pending_signal,
            "Steam",
            "playfab_steam",
            &PFAuthenticationLoginWithSteamGetResultSize,
            &PFAuthenticationLoginWithSteamGetResult);
    context->bind_cancel_handler();

    PFAuthenticationLoginWithSteamRequest request = {};
    request.createAccount = p_create_account;
    request.steamTicket = context->intern_string(steam_ticket);
    request.ticketIsServiceSpecific = context->intern_bool(p_ticket_is_service_specific);

    HRESULT hr = PFAuthenticationLoginWithSteamAsync(
            runtime->get_service_config_handle(),
            &request,
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the PlayFab Steam login request.", "playfab_steam_sign_in_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabUsers::sign_in_with_open_id_connect_async(const String &p_connection_id, const String &p_id_token, bool p_create_account) {
    PlayFabRuntime *runtime = _get_runtime();
    const String connection_id = p_connection_id.strip_edges();
    const String id_token = p_id_token.strip_edges();
    if (connection_id.is_empty()) {
        return make_users_error_signal(runtime, E_INVALIDARG, "invalid_connection_id", "PlayFab OpenID Connect sign-in requires a non-empty connection_id.");
    }
    if (id_token.is_empty()) {
        return make_users_error_signal(runtime, E_INVALIDARG, "invalid_id_token", "PlayFab OpenID Connect sign-in requires a non-empty id_token.");
    }

    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_users_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    if (!PLAYFAB_GDK_PLATFORM) {
        return make_users_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab sign-in is only supported on GDK platforms right now.");
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new SignInEntityAsyncContext(
            this,
            runtime,
            pending_signal,
            "OpenID Connect",
            "playfab_open_id_connect",
            &PFAuthenticationLoginWithOpenIdConnectGetResultSize,
            &PFAuthenticationLoginWithOpenIdConnectGetResult);
    context->bind_cancel_handler();

    PFAuthenticationLoginWithOpenIdConnectRequest request = {};
    request.connectionId = context->intern_string(connection_id);
    request.createAccount = p_create_account;
    request.idToken = context->intern_string(id_token);

    HRESULT hr = PFAuthenticationLoginWithOpenIdConnectAsync(
            runtime->get_service_config_handle(),
            &request,
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the PlayFab OpenID Connect login request.", "playfab_open_id_connect_sign_in_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabUsers::sign_in_with_battle_net_async(const String &p_identity_token, bool p_create_account) {
    PlayFabRuntime *runtime = _get_runtime();
    const String identity_token = p_identity_token.strip_edges();
    if (identity_token.is_empty()) {
        return make_users_error_signal(runtime, E_INVALIDARG, "invalid_identity_token", "PlayFab Battle.net sign-in requires a non-empty identity_token.");
    }

    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_users_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    if (!PLAYFAB_GDK_PLATFORM) {
        return make_users_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab sign-in is only supported on GDK platforms right now.");
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new SignInEntityAsyncContext(
            this,
            runtime,
            pending_signal,
            "Battle.net",
            "playfab_battle_net",
            &PFAuthenticationLoginWithBattleNetGetResultSize,
            &PFAuthenticationLoginWithBattleNetGetResult);
    context->bind_cancel_handler();

    PFAuthenticationLoginWithBattleNetRequest request = {};
    request.createAccount = p_create_account;
    request.identityToken = context->intern_string(identity_token);

    HRESULT hr = PFAuthenticationLoginWithBattleNetAsync(
            runtime->get_service_config_handle(),
            &request,
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the PlayFab Battle.net login request.", "playfab_battle_net_sign_in_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Ref<PlayFabUser> PlayFabUsers::get_user_by_local_id(int64_t p_local_id) const {
    XUserLocalId local_id = {};
    local_id.value = static_cast<uint64_t>(p_local_id);
    return _find_user_by_local_id(local_id);
}

Ref<PlayFabUser> PlayFabUsers::get_user_by_custom_id(const String &p_custom_id) const {
    return _find_user_by_custom_id(p_custom_id);
}

Ref<PlayFabUser> PlayFabUsers::get_user(const Variant &p_user_or_local_id) const {
    if (p_user_or_local_id.get_type() == Variant::OBJECT) {
        Object *object = Object::cast_to<Object>(p_user_or_local_id);
        if (auto *playfab_user = Object::cast_to<PlayFabUser>(object)) {
            return Ref<PlayFabUser>(playfab_user);
        }
    }

    XUserLocalId local_id = {};
    if (!_try_get_local_id_from_variant(p_user_or_local_id, &local_id, nullptr)) {
        return Ref<PlayFabUser>();
    }

    return _find_user_by_local_id(local_id);
}

Array PlayFabUsers::get_users() const {
    Array users;
    for (const Ref<PlayFabUser> &user : m_users) {
        users.push_back(user);
    }
    return users;
}

bool PlayFabUsers::add_or_update_user_session(const Ref<PlayFabUser> &p_user) {
    return _add_or_update_user(p_user);
}

PlayFabRuntime *PlayFabUsers::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

bool PlayFabUsers::_try_get_local_id_from_xuser_object(Object *p_user, XUserLocalId *r_local_id, String *r_error) {
    if (r_local_id == nullptr) {
        return false;
    }

    *r_local_id = {};

    if (p_user == nullptr) {
        if (r_error != nullptr) {
            *r_error = "PlayFab XUser sign-in requires a GDKUser object.";
        }
        return false;
    }

    if (Object::cast_to<PlayFabUser>(p_user) != nullptr) {
        if (r_error != nullptr) {
            *r_error = "PlayFab XUser sign-in requires a GDKUser object, not a PlayFabUser.";
        }
        return false;
    }

    if (!p_user->has_method("is_signed_in") || !p_user->has_method("get_local_id")) {
        if (r_error != nullptr) {
            *r_error = "PlayFab XUser sign-in requires a GDKUser object.";
        }
        return false;
    }

    Variant signed_in = p_user->call("is_signed_in");
    if (signed_in.get_type() != Variant::BOOL) {
        if (r_error != nullptr) {
            *r_error = "The provided GDK user does not expose a boolean signed-in state.";
        }
        return false;
    }

    if (!static_cast<bool>(signed_in)) {
        if (r_error != nullptr) {
            *r_error = "The provided GDK user is not signed in.";
        }
        return false;
    }

    Variant method_value = p_user->call("get_local_id");
    if (method_value.get_type() != Variant::INT) {
        if (r_error != nullptr) {
            *r_error = "The provided GDK user does not expose an integer local_id.";
        }
        return false;
    }

    r_local_id->value = static_cast<uint64_t>(static_cast<int64_t>(method_value));
    if (r_local_id->value == 0) {
        if (r_error != nullptr) {
            *r_error = "The provided GDK user does not have a valid local_id.";
        }
        return false;
    }

    return true;
}

bool PlayFabUsers::_try_get_local_id_from_variant(const Variant &p_user_or_local_id, XUserLocalId *r_local_id, String *r_error) {
    if (r_local_id == nullptr) {
        return false;
    }

    const Variant::Type type = p_user_or_local_id.get_type();
    if (type == Variant::INT) {
        r_local_id->value = static_cast<uint64_t>(static_cast<int64_t>(p_user_or_local_id));
        return r_local_id->value != 0;
    }

    if (type != Variant::OBJECT) {
        if (r_error != nullptr) {
            *r_error = "PlayFab user methods expect either a local_id integer, a GDKUser-like object, or a PlayFabUser.";
        }
        return false;
    }

    Object *object = Object::cast_to<Object>(p_user_or_local_id);
    if (object == nullptr) {
        if (r_error != nullptr) {
            *r_error = "The provided object is null.";
        }
        return false;
    }

    Variant local_id_variant = object->get("local_id");
    if (local_id_variant.get_type() == Variant::INT) {
        r_local_id->value = static_cast<uint64_t>(static_cast<int64_t>(local_id_variant));
        return r_local_id->value != 0;
    }

    if (object->has_method("get_local_id")) {
        Variant method_value = object->call("get_local_id");
        if (method_value.get_type() == Variant::INT) {
            r_local_id->value = static_cast<uint64_t>(static_cast<int64_t>(method_value));
            return r_local_id->value != 0;
        }
    }

    if (r_error != nullptr) {
        *r_error = "The provided object does not expose an integer local_id.";
    }
    return false;
}

bool PlayFabUsers::_add_or_update_user(const Ref<PlayFabUser> &p_user) {
    if (!p_user.is_valid()) {
        return false;
    }

    const uint64_t local_id = p_user->get_local_id();
    const String custom_id = p_user->get_custom_id();
    const String entity_id = p_user->get_entity_id();
    if (local_id == 0 && custom_id.is_empty() && entity_id.is_empty()) {
        return false;
    }
    XUserLocalId user_local_id = {};
    user_local_id.value = local_id;

    for (Ref<PlayFabUser> &existing : m_users) {
        if (!existing.is_valid()) {
            continue;
        }
        if (local_id != 0 && existing->matches_local_id(user_local_id)) {
            existing = p_user;
            return false;
        }
        if (!custom_id.is_empty() && existing->matches_custom_id(custom_id)) {
            existing = p_user;
            return false;
        }
        // Token-only sessions (Steam/OpenID/Battle.net) carry no local id or
        // custom id, so they are deduplicated by PlayFab entity id. Only match
        // against another entity-only session: XUser and custom-id wrappers
        // also populate entity_id, and replacing one of those richer sessions
        // with a leaner token wrapper would drop its local-user handle.
        if (local_id == 0 && custom_id.is_empty() && !entity_id.is_empty()
                && existing->get_local_id() == 0 && existing->get_custom_id().is_empty()
                && existing->matches_entity_id(entity_id)) {
            existing = p_user;
            return false;
        }
    }

    m_users.push_back(p_user);
    return true;
}

Ref<PlayFabUser> PlayFabUsers::_find_user_by_local_id(XUserLocalId p_user_local_id) const {
    if (p_user_local_id.value == 0) {
        return Ref<PlayFabUser>();
    }

    for (const Ref<PlayFabUser> &user : m_users) {
        if (user.is_valid() && user->matches_local_id(p_user_local_id)) {
            return user;
        }
    }

    return Ref<PlayFabUser>();
}

Ref<PlayFabUser> PlayFabUsers::_find_user_by_custom_id(const String &p_custom_id) const {
    const String custom_id = p_custom_id.strip_edges();
    if (custom_id.is_empty()) {
        return Ref<PlayFabUser>();
    }

    for (const Ref<PlayFabUser> &user : m_users) {
        if (user.is_valid() && user->matches_custom_id(custom_id)) {
            return user;
        }
    }

    return Ref<PlayFabUser>();
}

} // namespace godot
