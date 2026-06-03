#include "playfab_users.h"

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

class SignInUserAsyncContext final : public PlayFabSignalXAsyncContext {
    PlayFabUsers *m_users = nullptr;
    XUserHandle m_user_handle = nullptr;

public:
    SignInUserAsyncContext(
            PlayFabUsers *p_users,
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            XUserHandle p_user_handle) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_users(p_users),
            m_user_handle(p_user_handle) {}

    ~SignInUserAsyncContext() override {
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
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled("PlayFab sign-in cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "Failed to sign the Xbox user into PlayFab.", "playfab_sign_in_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        size_t buffer_size = 0;
        HRESULT size_hr = PFAuthenticationLoginWithXUserGetResultSize(p_async_block, &buffer_size);
        if (FAILED(size_hr)) {
            result = PlayFabResult::hresult_error(size_hr, "Failed to get the PlayFab login result size.", "playfab_sign_in_result_size_failed");
            get_runtime()->set_last_error(result);
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
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        Ref<PlayFabUser> user;
        user.instantiate();

        HRESULT user_hr = user->adopt_session(m_user_handle, entity_handle, get_runtime()->get_service_config_handle());
        if (FAILED(user_hr)) {
            result = PlayFabResult::hresult_error(user_hr, "Failed to translate the PlayFab login result into a Godot user wrapper.", "playfab_user_wrapper_create_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        m_users->add_or_update_user_session(user);

        get_runtime()->clear_last_error();
        get_pending_signal()->complete(PlayFabResult::ok_result(user));
    }
};

} // namespace

void PlayFabUsers::_bind_methods() {
    ClassDB::bind_method(D_METHOD("sign_in_async", "user_or_local_id", "create_account"), &PlayFabUsers::sign_in_async, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_user_by_local_id", "local_id"), &PlayFabUsers::get_user_by_local_id);
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

Signal PlayFabUsers::sign_in_async(const Variant &p_user_or_local_id, bool p_create_account) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_users_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    if (!PLAYFAB_GDK_PLATFORM) {
        return make_users_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab sign-in is only supported on GDK platforms right now.");
    }

    XUserLocalId local_id = {};
    String error_message;
    if (!_try_get_local_id_from_variant(p_user_or_local_id, &local_id, &error_message)) {
        return make_users_error_signal(runtime, E_INVALIDARG, "invalid_user_or_local_id", error_message);
    }

    XUserHandle user_handle = nullptr;
    HRESULT hr = XUserFindUserByLocalId(local_id, &user_handle);
    if (FAILED(hr)) {
        return make_users_error_signal(runtime, hr, "xuser_not_found", "Failed to find an active XUserHandle for the requested local_id.");
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new SignInUserAsyncContext(this, runtime, pending_signal, user_handle);
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
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Ref<PlayFabUser> PlayFabUsers::get_user_by_local_id(int64_t p_local_id) const {
    XUserLocalId local_id = {};
    local_id.value = static_cast<uint64_t>(p_local_id);
    return _find_user_by_local_id(local_id);
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
    for (Ref<PlayFabUser> &existing : m_users) {
        if (existing.is_valid() && existing->get_local_id() == p_user->get_local_id()) {
            existing = p_user;
            return false;
        }
    }

    m_users.push_back(p_user);
    return true;
}

Ref<PlayFabUser> PlayFabUsers::_find_user_by_local_id(XUserLocalId p_user_local_id) const {
    for (const Ref<PlayFabUser> &user : m_users) {
        if (user.is_valid() && user->matches_local_id(p_user_local_id)) {
            return user;
        }
    }

    return Ref<PlayFabUser>();
}

} // namespace godot
