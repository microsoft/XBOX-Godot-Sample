#include "playfab_gamesaves.h"

#include <vector>

#include "playfab_runtime.h"

#include <playfab/gamesave/PFGameSaveFiles.h>

#include "playfab.h"
#include "playfab_pending_signal.h"
#include "playfab_result.h"
#include "playfab_signal_xasync_context.h"
#include "playfab_user.h"

namespace godot {

namespace {

#if defined(HC_PLATFORM) && HC_PLATFORM == HC_PLATFORM_GDK
constexpr bool PLAYFAB_GDK_PLATFORM = true;
#else
constexpr bool PLAYFAB_GDK_PLATFORM = false;
#endif

bool is_cancelled_hresult(HRESULT p_hresult) {
    return p_hresult == E_ABORT || p_hresult == E_PF_GAMESAVE_USER_CANCELLED;
}

Signal make_game_saves_error_signal(
        PlayFabRuntime *p_runtime,
        HRESULT p_hresult,
        const String &p_code,
        const String &p_message,
        const Variant &p_data = Variant()) {
    if (p_runtime != nullptr) {
        return p_runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
    }

    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(PlayFabResult::error_result(p_hresult, p_code, p_message, p_data));
    return pending_signal->get_completed_signal();
}

Ref<PlayFabResult> make_game_saves_error_result(
        PlayFabRuntime *p_runtime,
        HRESULT p_hresult,
        const String &p_code,
        const String &p_message,
        const Variant &p_data = Variant()) {
    Ref<PlayFabResult> result = PlayFabResult::error_result(p_hresult, p_code, p_message, p_data);
    if (p_runtime != nullptr) {
    }
    return result;
}

Ref<PlayFabResult> make_game_saves_hresult_error(
        PlayFabRuntime *p_runtime,
        HRESULT p_hresult,
        const String &p_action,
        const String &p_code,
        const Variant &p_data = Variant()) {
    Ref<PlayFabResult> result = PlayFabResult::hresult_error(p_hresult, p_action, p_code, p_data);
    if (p_runtime != nullptr) {
    }
    return result;
}

Signal make_game_saves_user_error_signal(PlayFabRuntime *p_runtime, HRESULT p_hresult) {
    const bool needs_xbox_user = p_hresult == E_HANDLE;
    return make_game_saves_error_signal(
            p_runtime,
            p_hresult,
            needs_xbox_user ? "xbox_user_required" : "invalid_user",
            needs_xbox_user
                    ? "Game Saves operations require an Xbox-backed PlayFabUser created through PlayFab.users.sign_in_with_xuser_async(). Custom-ID PlayFab users do not expose a local user handle."
                    : "Game Saves operations require a valid signed-in PlayFabUser.");
}

Ref<PlayFabResult> make_game_saves_user_error_result(PlayFabRuntime *p_runtime, HRESULT p_hresult) {
    const bool needs_xbox_user = p_hresult == E_HANDLE;
    return make_game_saves_error_result(
            p_runtime,
            p_hresult,
            needs_xbox_user ? "xbox_user_required" : "invalid_user",
            needs_xbox_user
                    ? "Game Saves operations require an Xbox-backed PlayFabUser created through PlayFab.users.sign_in_with_xuser_async(). Custom-ID PlayFab users do not expose a local user handle."
                    : "Game Saves operations require a valid signed-in PlayFabUser.");
}

Dictionary make_user_identity_data(const Ref<PlayFabUser> &p_user) {
    Dictionary data;
    data["local_id"] = static_cast<int64_t>(p_user->get_local_id());
    data["custom_id"] = p_user->get_custom_id();
    data["entity_key"] = p_user->get_entity_key();
    return data;
}

HRESULT duplicate_local_user_handle(const Ref<PlayFabUser> &p_user, PFLocalUserHandle *r_local_user_handle) {
    if (r_local_user_handle == nullptr) {
        return E_INVALIDARG;
    }

    *r_local_user_handle = nullptr;
    if (!p_user.is_valid()) {
        return E_INVALIDARG;
    }

    return p_user->duplicate_local_user_handle(r_local_user_handle);
}

HRESULT get_folder_for_local_user(PFLocalUserHandle p_local_user_handle, String *r_folder, int64_t *r_folder_size = nullptr) {
    size_t folder_size = 0;
    HRESULT hr = PFGameSaveFilesGetFolderSize(p_local_user_handle, &folder_size);
    if (FAILED(hr)) {
        return hr;
    }

    std::vector<char> folder_buffer(folder_size > 0 ? folder_size : 1, '\0');
    size_t used = 0;
    hr = PFGameSaveFilesGetFolder(
            p_local_user_handle,
            folder_buffer.size(),
            folder_buffer.data(),
            &used);
    if (FAILED(hr)) {
        return hr;
    }

    if (r_folder != nullptr) {
        *r_folder = String::utf8(folder_buffer.data());
    }
    if (r_folder_size != nullptr) {
        *r_folder_size = static_cast<int64_t>(folder_size);
    }

    return S_OK;
}

HRESULT get_connected_to_cloud_for_local_user(PFLocalUserHandle p_local_user_handle, bool *r_connected_to_cloud) {
    if (r_connected_to_cloud == nullptr) {
        return E_INVALIDARG;
    }

    return PFGameSaveFilesIsConnectedToCloud(p_local_user_handle, r_connected_to_cloud);
}

HRESULT get_remaining_quota_for_local_user(PFLocalUserHandle p_local_user_handle, int64_t *r_remaining_quota) {
    if (r_remaining_quota == nullptr) {
        return E_INVALIDARG;
    }

    return PFGameSaveFilesGetRemainingQuota(p_local_user_handle, r_remaining_quota);
}

HRESULT build_user_state_snapshot(PFLocalUserHandle p_local_user_handle, const Ref<PlayFabUser> &p_user, Dictionary *r_snapshot) {
    if (r_snapshot == nullptr) {
        return E_INVALIDARG;
    }

    String folder;
    int64_t folder_size = 0;
    HRESULT hr = get_folder_for_local_user(p_local_user_handle, &folder, &folder_size);
    if (FAILED(hr)) {
        return hr;
    }

    bool connected_to_cloud = false;
    hr = get_connected_to_cloud_for_local_user(p_local_user_handle, &connected_to_cloud);
    if (FAILED(hr)) {
        return hr;
    }

    Dictionary snapshot = make_user_identity_data(p_user);
    snapshot["folder"] = folder;
    snapshot["folder_size"] = folder_size;
    snapshot["connected_to_cloud"] = connected_to_cloud;

    if (connected_to_cloud) {
        int64_t remaining_quota = 0;
        hr = get_remaining_quota_for_local_user(p_local_user_handle, &remaining_quota);
        if (FAILED(hr)) {
            return hr;
        }

        snapshot["remaining_quota"] = remaining_quota;
    }

    *r_snapshot = snapshot;
    return S_OK;
}

class GameSaveSimpleAsyncContext final : public PlayFabSignalXAsyncContext {
    PFLocalUserHandle m_local_user_handle = nullptr;
    HRESULT (*m_result_fn)(XAsyncBlock *) = nullptr;
    Variant m_success_data;
    String m_cancel_message;
    String m_failure_action;
    String m_failure_code;

public:
    GameSaveSimpleAsyncContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            PFLocalUserHandle p_local_user_handle,
            HRESULT (*p_result_fn)(XAsyncBlock *),
            const Variant &p_success_data,
            const String &p_cancel_message,
            const String &p_failure_action,
            const String &p_failure_code) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_local_user_handle(p_local_user_handle),
            m_result_fn(p_result_fn),
            m_success_data(p_success_data),
            m_cancel_message(p_cancel_message),
            m_failure_action(p_failure_action),
            m_failure_code(p_failure_code) {}

    ~GameSaveSimpleAsyncContext() override {
        if (m_local_user_handle != nullptr) {
            PFLocalUserCloseHandle(m_local_user_handle);
            m_local_user_handle = nullptr;
        }
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled(m_cancel_message);
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (is_cancelled_hresult(status_hr)) {
            result = PlayFabResult::cancelled(m_cancel_message);
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, m_failure_action, m_failure_code);
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT result_hr = m_result_fn != nullptr ? m_result_fn(p_async_block) : E_FAIL;
        if (is_cancelled_hresult(result_hr)) {
            result = PlayFabResult::cancelled(m_cancel_message);
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, m_failure_action, m_failure_code);
            get_pending_signal()->complete(result);
            return;
        }

        get_pending_signal()->complete(PlayFabResult::ok_result(m_success_data));
    }
};

class GameSaveAddUserContext final : public PlayFabSignalXAsyncContext {
    Ref<PlayFabUser> m_user;
    PFLocalUserHandle m_local_user_handle = nullptr;

public:
    GameSaveAddUserContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            PFLocalUserHandle p_local_user_handle) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_local_user_handle(p_local_user_handle) {}

    ~GameSaveAddUserContext() override {
        if (m_local_user_handle != nullptr) {
            PFLocalUserCloseHandle(m_local_user_handle);
            m_local_user_handle = nullptr;
        }
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled("Game Saves user sync cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (is_cancelled_hresult(status_hr)) {
            result = PlayFabResult::cancelled("Game Saves user sync cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "Failed to add the PlayFab user to Game Saves.", "gamesave_add_user_failed");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT result_hr = PFGameSaveFilesAddUserWithUiResult(p_async_block);
        if (is_cancelled_hresult(result_hr)) {
            result = PlayFabResult::cancelled("Game Saves user sync cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, "Failed to finish adding the PlayFab user to Game Saves.", "gamesave_add_user_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Dictionary snapshot;
        HRESULT snapshot_hr = build_user_state_snapshot(m_local_user_handle, m_user, &snapshot);
        if (FAILED(snapshot_hr)) {
            result = PlayFabResult::hresult_error(snapshot_hr, "Failed to query the synchronized Game Saves state.", "gamesave_state_snapshot_failed");
            get_pending_signal()->complete(result);
            return;
        }

        get_pending_signal()->complete(PlayFabResult::ok_result(snapshot));
    }
};

} // namespace

void PlayFabGameSaves::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_user_with_ui_async", "user", "options"), &PlayFabGameSaves::add_user_with_ui_async, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("upload_with_ui_async", "user", "release_device_as_active"), &PlayFabGameSaves::upload_with_ui_async, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("set_save_description_async", "user", "short_save_description"), &PlayFabGameSaves::set_save_description_async);
    ClassDB::bind_method(D_METHOD("reset_cloud_async", "user"), &PlayFabGameSaves::reset_cloud_async);
    ClassDB::bind_method(D_METHOD("get_folder", "user"), &PlayFabGameSaves::get_folder);
    ClassDB::bind_method(D_METHOD("get_folder_size", "user"), &PlayFabGameSaves::get_folder_size);
    ClassDB::bind_method(D_METHOD("get_remaining_quota", "user"), &PlayFabGameSaves::get_remaining_quota);
    ClassDB::bind_method(D_METHOD("is_connected_to_cloud", "user"), &PlayFabGameSaves::is_connected_to_cloud);

    BIND_CONSTANT(ADD_USER_OPTION_NONE);
    BIND_CONSTANT(ADD_USER_OPTION_ROLLBACK_TO_LAST_KNOWN_GOOD);
    BIND_CONSTANT(ADD_USER_OPTION_ROLLBACK_TO_LAST_CONFLICT);
}

void PlayFabGameSaves::set_owner(PlayFab *p_owner) {
    m_owner = p_owner;
}

Signal PlayFabGameSaves::add_user_with_ui_async(const Ref<PlayFabUser> &p_user, int64_t p_options) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_game_saves_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }
    if (!PLAYFAB_GDK_PLATFORM) {
        return make_game_saves_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab Game Saves is only supported on GDK platforms right now.");
    }
    if (p_options < 0 || p_options > static_cast<int64_t>(UINT32_MAX)) {
        return make_game_saves_error_signal(runtime, E_INVALIDARG, "invalid_options", "Game Saves add-user options must fit in a uint32 bitmask.");
    }

    PFLocalUserHandle local_user_handle = nullptr;
    HRESULT user_hr = duplicate_local_user_handle(p_user, &local_user_handle);
    if (FAILED(user_hr)) {
        return make_game_saves_user_error_signal(runtime, user_hr);
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new GameSaveAddUserContext(runtime, pending_signal, p_user, local_user_handle);
    context->bind_cancel_handler();

    HRESULT hr = PFGameSaveFilesAddUserWithUiAsync(
            local_user_handle,
            static_cast<PFGameSaveFilesAddUserOptions>(static_cast<uint32_t>(p_options)),
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the Game Saves add-user request.", "gamesave_add_user_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabGameSaves::upload_with_ui_async(const Ref<PlayFabUser> &p_user, bool p_release_device_as_active) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_game_saves_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }
    if (!PLAYFAB_GDK_PLATFORM) {
        return make_game_saves_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab Game Saves is only supported on GDK platforms right now.");
    }

    PFLocalUserHandle local_user_handle = nullptr;
    HRESULT user_hr = duplicate_local_user_handle(p_user, &local_user_handle);
    if (FAILED(user_hr)) {
        return make_game_saves_user_error_signal(runtime, user_hr);
    }

    Dictionary success_data = make_user_identity_data(p_user);
    success_data["release_device_as_active"] = p_release_device_as_active;

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new GameSaveSimpleAsyncContext(
            runtime,
            pending_signal,
            local_user_handle,
            PFGameSaveFilesUploadWithUiResult,
            success_data,
            "Game Saves upload cancelled.",
            "Failed to upload Game Saves data.",
            "gamesave_upload_failed");
    context->bind_cancel_handler();

    HRESULT hr = PFGameSaveFilesUploadWithUiAsync(
            local_user_handle,
            p_release_device_as_active ? PFGameSaveFilesUploadOption::ReleaseDeviceAsActive : PFGameSaveFilesUploadOption::KeepDeviceActive,
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the Game Saves upload request.", "gamesave_upload_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabGameSaves::set_save_description_async(const Ref<PlayFabUser> &p_user, const String &p_short_save_description) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_game_saves_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }
    if (!PLAYFAB_GDK_PLATFORM) {
        return make_game_saves_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab Game Saves is only supported on GDK platforms right now.");
    }

    PFLocalUserHandle local_user_handle = nullptr;
    HRESULT user_hr = duplicate_local_user_handle(p_user, &local_user_handle);
    if (FAILED(user_hr)) {
        return make_game_saves_user_error_signal(runtime, user_hr);
    }

    Dictionary success_data = make_user_identity_data(p_user);
    success_data["short_save_description"] = p_short_save_description;

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new GameSaveSimpleAsyncContext(
            runtime,
            pending_signal,
            local_user_handle,
            PFGameSaveFilesSetSaveDescriptionResult,
            success_data,
            "Setting the Game Saves description was cancelled.",
            "Failed to set the Game Saves description.",
            "gamesave_set_description_failed");
    context->bind_cancel_handler();

    const CharString description_utf8 = p_short_save_description.utf8();
    HRESULT hr = PFGameSaveFilesSetSaveDescriptionAsync(
            local_user_handle,
            description_utf8.get_data(),
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the Game Saves description request.", "gamesave_set_description_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabGameSaves::reset_cloud_async(const Ref<PlayFabUser> &p_user) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_game_saves_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }
    if (!PLAYFAB_GDK_PLATFORM) {
        return make_game_saves_error_signal(runtime, E_FAIL, "platform_unsupported", "PlayFab Game Saves is only supported on GDK platforms right now.");
    }

    PFLocalUserHandle local_user_handle = nullptr;
    HRESULT user_hr = duplicate_local_user_handle(p_user, &local_user_handle);
    if (FAILED(user_hr)) {
        return make_game_saves_user_error_signal(runtime, user_hr);
    }

    Dictionary success_data = make_user_identity_data(p_user);

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new GameSaveSimpleAsyncContext(
            runtime,
            pending_signal,
            local_user_handle,
            PFGameSaveFilesResetCloudResult,
            success_data,
            "Resetting cloud Game Saves was cancelled.",
            "Failed to reset cloud Game Saves state.",
            "gamesave_reset_cloud_failed");
    context->bind_cancel_handler();

    HRESULT hr = PFGameSaveFilesResetCloudAsync(local_user_handle, context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the Game Saves reset-cloud request.", "gamesave_reset_cloud_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Ref<PlayFabResult> PlayFabGameSaves::get_folder(const Ref<PlayFabUser> &p_user) const {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_game_saves_error_result(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }
    if (!PLAYFAB_GDK_PLATFORM) {
        return make_game_saves_error_result(runtime, E_FAIL, "platform_unsupported", "PlayFab Game Saves is only supported on GDK platforms right now.");
    }

    PFLocalUserHandle local_user_handle = nullptr;
    HRESULT user_hr = duplicate_local_user_handle(p_user, &local_user_handle);
    if (FAILED(user_hr)) {
        return make_game_saves_user_error_result(runtime, user_hr);
    }

    String folder;
    HRESULT hr = get_folder_for_local_user(local_user_handle, &folder);
    PFLocalUserCloseHandle(local_user_handle);
    if (FAILED(hr)) {
        return make_game_saves_hresult_error(runtime, hr, "Failed to query the Game Saves folder.", "gamesave_get_folder_failed");
    }

    return PlayFabResult::ok_result(folder);
}

Ref<PlayFabResult> PlayFabGameSaves::get_folder_size(const Ref<PlayFabUser> &p_user) const {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_game_saves_error_result(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }
    if (!PLAYFAB_GDK_PLATFORM) {
        return make_game_saves_error_result(runtime, E_FAIL, "platform_unsupported", "PlayFab Game Saves is only supported on GDK platforms right now.");
    }

    PFLocalUserHandle local_user_handle = nullptr;
    HRESULT user_hr = duplicate_local_user_handle(p_user, &local_user_handle);
    if (FAILED(user_hr)) {
        return make_game_saves_user_error_result(runtime, user_hr);
    }

    size_t folder_size = 0;
    HRESULT hr = PFGameSaveFilesGetFolderSize(local_user_handle, &folder_size);
    PFLocalUserCloseHandle(local_user_handle);
    if (FAILED(hr)) {
        return make_game_saves_hresult_error(runtime, hr, "Failed to query the Game Saves folder size.", "gamesave_get_folder_size_failed");
    }

    return PlayFabResult::ok_result(static_cast<int64_t>(folder_size));
}

Ref<PlayFabResult> PlayFabGameSaves::get_remaining_quota(const Ref<PlayFabUser> &p_user) const {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_game_saves_error_result(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }
    if (!PLAYFAB_GDK_PLATFORM) {
        return make_game_saves_error_result(runtime, E_FAIL, "platform_unsupported", "PlayFab Game Saves is only supported on GDK platforms right now.");
    }

    PFLocalUserHandle local_user_handle = nullptr;
    HRESULT user_hr = duplicate_local_user_handle(p_user, &local_user_handle);
    if (FAILED(user_hr)) {
        return make_game_saves_user_error_result(runtime, user_hr);
    }

    int64_t remaining_quota = 0;
    HRESULT hr = get_remaining_quota_for_local_user(local_user_handle, &remaining_quota);
    PFLocalUserCloseHandle(local_user_handle);
    if (FAILED(hr)) {
        return make_game_saves_hresult_error(runtime, hr, "Failed to query the remaining Game Saves quota.", "gamesave_get_remaining_quota_failed");
    }

    return PlayFabResult::ok_result(remaining_quota);
}

Ref<PlayFabResult> PlayFabGameSaves::is_connected_to_cloud(const Ref<PlayFabUser> &p_user) const {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_game_saves_error_result(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }
    if (!PLAYFAB_GDK_PLATFORM) {
        return make_game_saves_error_result(runtime, E_FAIL, "platform_unsupported", "PlayFab Game Saves is only supported on GDK platforms right now.");
    }

    PFLocalUserHandle local_user_handle = nullptr;
    HRESULT user_hr = duplicate_local_user_handle(p_user, &local_user_handle);
    if (FAILED(user_hr)) {
        return make_game_saves_user_error_result(runtime, user_hr);
    }

    bool connected_to_cloud = false;
    HRESULT hr = get_connected_to_cloud_for_local_user(local_user_handle, &connected_to_cloud);
    PFLocalUserCloseHandle(local_user_handle);
    if (FAILED(hr)) {
        return make_game_saves_hresult_error(runtime, hr, "Failed to query the Game Saves cloud connectivity.", "gamesave_get_connected_to_cloud_failed");
    }

    return PlayFabResult::ok_result(connected_to_cloud);
}

PlayFabRuntime *PlayFabGameSaves::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

} // namespace godot
