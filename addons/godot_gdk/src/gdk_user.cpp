#include "gdk_user.h"

#include <algorithm>

#include "gdk.h"
#include "gdk_async_op.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_xasync_context.h"

namespace godot {

namespace {

class AddUserAsyncContext final : public GDKXAsyncContext {
    GDKUsers *m_users = nullptr;
    String m_action;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_op()->was_cancel_requested()) {
            result = GDKResult::cancelled("User add operation cancelled.");
            get_runtime()->set_last_error(result);
            get_op()->complete(result);
            return;
        }

        XUserHandle user_handle = nullptr;
        HRESULT result_hr = XUserAddResult(p_async_block, &user_handle);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("User add operation cancelled.");
            get_runtime()->set_last_error(result);
            get_op()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(result_hr, m_action, "user_add_result_failed");
            get_runtime()->set_last_error(result);
            get_op()->complete(result);
            return;
        }

        m_users->complete_add_user(user_handle, get_op());
    }

public:
    AddUserAsyncContext(GDKUsers *p_users, GDKRuntime *p_runtime, const Ref<GDKAsyncOp> &p_op, const String &p_action) :
            GDKXAsyncContext(p_runtime, p_op),
            m_users(p_users),
            m_action(p_action) {}
};

} // namespace

void GDKUser::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_local_id"), &GDKUser::get_local_id);
    ClassDB::bind_method(D_METHOD("get_xuid"), &GDKUser::get_xuid);
    ClassDB::bind_method(D_METHOD("get_gamertag"), &GDKUser::get_gamertag);
    ClassDB::bind_method(D_METHOD("is_guest"), &GDKUser::is_guest);
    ClassDB::bind_method(D_METHOD("is_signed_in"), &GDKUser::is_signed_in);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "local_id"), "", "get_local_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "xuid"), "", "get_xuid");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "gamertag"), "", "get_gamertag");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "guest"), "", "is_guest");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "signed_in"), "", "is_signed_in");
}

GDKUser::GDKUser() {}

GDKUser::~GDKUser() {
    clear();
}

int64_t GDKUser::get_local_id() const {
    return static_cast<int64_t>(m_local_id.value);
}

String GDKUser::get_xuid() const {
    return m_xuid;
}

String GDKUser::get_gamertag() const {
    return m_gamertag;
}

bool GDKUser::is_guest() const {
    return m_is_guest;
}

bool GDKUser::is_signed_in() const {
    return m_is_signed_in;
}

HRESULT GDKUser::_populate_from_handle(XUserHandle p_user_handle) {
    XUserLocalId local_id = {};
    HRESULT hr = XUserGetLocalId(p_user_handle, &local_id);
    if (FAILED(hr)) {
        return hr;
    }

    uint64_t xuid = 0;
    hr = XUserGetId(p_user_handle, &xuid);
    if (FAILED(hr)) {
        return hr;
    }

    char gamertag[XUserGamertagComponentClassicMaxBytes] = {};
    size_t gamertag_used = 0;
    hr = XUserGetGamertag(
        p_user_handle,
        XUserGamertagComponent::Classic,
        sizeof(gamertag),
        gamertag,
        &gamertag_used);
    if (FAILED(hr)) {
        return hr;
    }

    bool is_guest = false;
    hr = XUserGetIsGuest(p_user_handle, &is_guest);
    if (FAILED(hr)) {
        return hr;
    }

    XUserState user_state = XUserState::SignedOut;
    hr = XUserGetState(p_user_handle, &user_state);
    if (FAILED(hr)) {
        return hr;
    }

    m_local_id = local_id;
    m_xuid = String::num_uint64(xuid);
    m_gamertag = String::utf8(gamertag);
    m_is_guest = is_guest;
    m_is_signed_in = user_state == XUserState::SignedIn;

    return S_OK;
}

HRESULT GDKUser::adopt_handle(XUserHandle p_user_handle) {
    clear();

    if (p_user_handle == nullptr) {
        return E_INVALIDARG;
    }

    m_user_handle = p_user_handle;
    HRESULT hr = _populate_from_handle(m_user_handle);
    if (FAILED(hr)) {
        clear();
    }

    return hr;
}

HRESULT GDKUser::refresh() {
    if (m_user_handle == nullptr) {
        return E_FAIL;
    }

    return _populate_from_handle(m_user_handle);
}

bool GDKUser::matches_local_id(XUserLocalId p_user_local_id) const {
    return m_local_id.value == p_user_local_id.value;
}

XUserHandle GDKUser::get_handle() const {
    return m_user_handle;
}

void GDKUser::clear() {
    if (m_user_handle != nullptr) {
        XUserCloseHandle(m_user_handle);
        m_user_handle = nullptr;
    }

    m_local_id = {};
    m_xuid = "";
    m_gamertag = "";
    m_is_guest = false;
    m_is_signed_in = false;
}

void GDKUsers::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_default_user_async", "allow_guests"), &GDKUsers::add_default_user_async, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("add_user_with_ui_async"), &GDKUsers::add_user_with_ui_async);
    ClassDB::bind_method(D_METHOD("get_primary_user"), &GDKUsers::get_primary_user);
    ClassDB::bind_method(D_METHOD("get_users"), &GDKUsers::get_users);

    ADD_SIGNAL(MethodInfo("user_added", PropertyInfo(Variant::OBJECT, "user")));
    ADD_SIGNAL(MethodInfo("user_removed", PropertyInfo(Variant::INT, "local_id")));
    ADD_SIGNAL(MethodInfo("user_changed", PropertyInfo(Variant::OBJECT, "user")));
    ADD_SIGNAL(MethodInfo("primary_user_changed", PropertyInfo(Variant::OBJECT, "user")));
}

void GDKUsers::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKUsers::on_runtime_initialized() {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(E_FAIL, "runtime_not_initialized", "Cannot initialize the users service before the GDK runtime.");
    }

    if (m_change_event_registered) {
        m_runtime_ready = true;
        return GDKResult::ok_result();
    }

    HRESULT hr = XUserRegisterForChangeEvent(
        runtime->get_task_queue(),
        this,
        _user_change_callback,
        &m_change_token);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to register the runtime-wide XUser change callback.", "user_change_event_register_failed");
    }

    m_runtime_ready = true;
    m_change_event_registered = true;
    return GDKResult::ok_result();
}

void GDKUsers::shutdown() {
    m_runtime_ready = false;

    if (m_change_event_registered) {
        XUserUnregisterForChangeEvent(m_change_token, false);
        m_change_event_registered = false;
    }

    m_primary_user.unref();
    m_users.clear();
}

Ref<GDKAsyncOp> GDKUsers::add_default_user_async(bool p_allow_guests) {
    XUserAddOptions options = XUserAddOptions::AddDefaultUserSilently;
    if (p_allow_guests) {
        options = options | XUserAddOptions::AllowGuests;
    }

    return _start_add_user_async(options, "Failed to add the default user.");
}

Ref<GDKAsyncOp> GDKUsers::add_user_with_ui_async() {
    return _start_add_user_async(XUserAddOptions::AddDefaultUserAllowingUI, "Failed to add a user with UI.");
}

Ref<GDKUser> GDKUsers::get_primary_user() const {
    return m_primary_user;
}

Array GDKUsers::get_users() const {
    Array users;
    for (const Ref<GDKUser> &user : m_users) {
        users.push_back(user);
    }
    return users;
}

void GDKUsers::on_user_change(XUserLocalId p_user_local_id, XUserChangeEvent p_event) {
    GDKRuntime *runtime = _get_runtime();
    if (!m_runtime_ready || runtime == nullptr || runtime->is_shutting_down()) {
        return;
    }

    Ref<GDKUser> user = _find_user_by_local_id(p_user_local_id);
    if (!user.is_valid()) {
        return;
    }

    switch (p_event) {
        case XUserChangeEvent::SignedOut: {
            const int64_t local_id = user->get_local_id();
            const bool was_primary = m_primary_user.is_valid() && m_primary_user->get_local_id() == local_id;

            if (m_owner != nullptr) {
                m_owner->notify_user_removed(user);
            }

            _remove_user_by_local_id(p_user_local_id);
            if (was_primary) {
                m_primary_user = m_users.empty() ? Ref<GDKUser>() : m_users.front();
                emit_signal("primary_user_changed", m_primary_user);
            }

            emit_signal("user_removed", local_id);
        } break;
        case XUserChangeEvent::SignedInAgain:
        case XUserChangeEvent::Gamertag:
        case XUserChangeEvent::GamerPicture:
        case XUserChangeEvent::Privileges: {
            if (SUCCEEDED(user->refresh())) {
                emit_signal("user_changed", user);
            }
        } break;
        case XUserChangeEvent::SigningOut:
        default:
            break;
    }
}

void GDKUsers::complete_add_user(XUserHandle p_user_handle, const Ref<GDKAsyncOp> &p_op) {
    Ref<GDKUser> user;
    user.instantiate();

    HRESULT hr = user->adopt_handle(p_user_handle);
    if (FAILED(hr)) {
        if (p_user_handle != nullptr) {
            XUserCloseHandle(p_user_handle);
        }

        Ref<GDKResult> result = GDKResult::hresult_error(hr, "Failed to translate the native XUser into a Godot wrapper.", "user_wrapper_create_failed");
        _get_runtime()->set_last_error(result);
        p_op->complete(result);
        return;
    }

    const bool is_new_user = _upsert_user(user);
    const bool primary_changed = !m_primary_user.is_valid() || m_primary_user->get_local_id() != user->get_local_id();
    m_primary_user = user;

    if (is_new_user) {
        emit_signal("user_added", user);
    } else {
        emit_signal("user_changed", user);
    }

    if (primary_changed) {
        emit_signal("primary_user_changed", user);
    }

    _get_runtime()->clear_last_error();
    p_op->complete(GDKResult::ok_result(user));
}

void CALLBACK GDKUsers::_user_change_callback(void *p_context, XUserLocalId p_user_local_id, XUserChangeEvent p_event) {
    auto *users = static_cast<GDKUsers *>(p_context);
    users->on_user_change(p_user_local_id, p_event);
}

GDKRuntime *GDKUsers::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

Ref<GDKAsyncOp> GDKUsers::_start_add_user_async(XUserAddOptions p_options, const String &p_action) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        if (m_owner != nullptr) {
            return m_owner->make_async_error_op(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
        }

        Ref<GDKAsyncOp> op;
        op.instantiate();
        op->complete(GDKResult::error_result(E_FAIL, "not_initialized", "The users service is not attached to a GDK runtime."));
        return op;
    }

    Ref<GDKAsyncOp> op;
    op.instantiate();
    runtime->retain_op(op);

    auto *context = new AddUserAsyncContext(this, runtime, op, p_action);
    context->bind_cancel_handler();

    HRESULT hr = XUserAddAsync(p_options, context->get_async_block());
    if (FAILED(hr)) {
        op->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(hr, p_action, "user_add_start_failed");
        runtime->set_last_error(result);
        op->complete(result);
        return op;
    }

    return op;
}

bool GDKUsers::_upsert_user(const Ref<GDKUser> &p_user) {
    for (Ref<GDKUser> &existing : m_users) {
        if (existing.is_valid() && existing->get_local_id() == p_user->get_local_id()) {
            existing = p_user;
            return false;
        }
    }

    m_users.push_back(p_user);
    return true;
}

Ref<GDKUser> GDKUsers::_find_user_by_local_id(XUserLocalId p_user_local_id) const {
    for (const Ref<GDKUser> &user : m_users) {
        if (user.is_valid() && user->matches_local_id(p_user_local_id)) {
            return user;
        }
    }

    return Ref<GDKUser>();
}

void GDKUsers::_remove_user_by_local_id(XUserLocalId p_user_local_id) {
    m_users.erase(
        std::remove_if(
            m_users.begin(),
            m_users.end(),
            [p_user_local_id](const Ref<GDKUser> &user) {
                return user.is_null() || user->matches_local_id(p_user_local_id);
            }),
        m_users.end());
}

} // namespace godot
