#include "gdk_game_save.h"

#include <vector>

#include <godot_cpp/variant/dictionary.hpp>

#include <XGameSaveFiles.h>

#include "gdk.h"
#include "gdk_pending_signal.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_signal_xasync_context.h"
#include "gdk_user.h"
#include "gdk_xbox_services.h"

namespace godot {

namespace {

class GetFolderAsyncContext final : public GDKSignalXAsyncContext {
    CharString m_configuration_id_utf8;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Game save folder request cancelled."));
            return;
        }

        char folder[MAX_PATH] = {};
        HRESULT result_hr = XGameSaveFilesGetFolderWithUiResult(p_async_block, sizeof(folder), folder);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Game save folder request cancelled."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to resolve the game save folder.",
                    "game_save_folder_failed"));
            return;
        }

        folder[sizeof(folder) - 1] = '\0';
        Dictionary data;
        data["path"] = String::utf8(folder);
        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    GetFolderAsyncContext(
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_configuration_id) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_configuration_id_utf8(p_configuration_id.utf8()) {}

    const char *get_configuration_id() const {
        return m_configuration_id_utf8.get_data();
    }
};

} // namespace

void GDKGameSave::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_folder_async", "user"), &GDKGameSave::get_folder_async);
    ClassDB::bind_method(D_METHOD("get_remaining_quota", "user"), &GDKGameSave::get_remaining_quota);
}

void GDKGameSave::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

GDKRuntime *GDKGameSave::_get_runtime() const {
    return m_owner == nullptr ? nullptr : m_owner->get_runtime();
}

GDKXboxServices *GDKGameSave::_get_xbox_services() const {
    return m_owner == nullptr ? nullptr : m_owner->get_xbox_services();
}

Ref<GDKResult> GDKGameSave::_resolve_configuration_id(String *r_configuration_id) const {
    GDKXboxServices *xbox_services = _get_xbox_services();
    if (xbox_services == nullptr || !xbox_services->is_initialized()) {
        return GDKResult::error_result(E_FAIL, "xbox_services_uninitialized", "Xbox services are not initialized.");
    }

    const String scid = xbox_services->get_scid();
    if (scid.is_empty()) {
        return GDKResult::error_result(
                E_FAIL,
                "service_configuration_id_unavailable",
                "Service configuration ID is unavailable for game saves.");
    }

    if (r_configuration_id != nullptr) {
        *r_configuration_id = scid;
    }
    return GDKResult::ok_result();
}

Signal GDKGameSave::_make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) const {
    GDKRuntime *runtime = _get_runtime();
    if (runtime != nullptr) {
        return runtime->make_error_signal(p_hresult, p_code, p_message);
    }

    Ref<GDKPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(GDKResult::error_result(p_hresult, p_code, p_message));
    return pending_signal->get_completed_signal();
}

Ref<GDKResult> GDKGameSave::on_runtime_initialized() {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the game save service before the GDK runtime.");
    }

    m_runtime_ready = true;
    return GDKResult::ok_result();
}

void GDKGameSave::shutdown() {
    m_runtime_ready = false;
}

Signal GDKGameSave::get_folder_async(const Ref<GDKUser> &p_user) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return _make_error_signal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr || !p_user->is_signed_in()) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to resolve the game save folder.");
    }

    String configuration_id;
    Ref<GDKResult> config_result = _resolve_configuration_id(&configuration_id);
    if (!config_result->is_ok()) {
        return _make_error_signal(static_cast<HRESULT>(config_result->get_hresult()), config_result->get_code(), config_result->get_message());
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new GetFolderAsyncContext(runtime, pending_signal, configuration_id);
    context->bind_cancel_handler();

    HRESULT hr = XGameSaveFilesGetFolderWithUiAsync(
            p_user->get_handle(),
            context->get_configuration_id(),
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                hr,
                "Failed to start the game save folder request.",
                "game_save_folder_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Ref<GDKResult> GDKGameSave::get_remaining_quota(const Ref<GDKUser> &p_user) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr || !p_user->is_signed_in()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to query the game save quota.");
    }

    String configuration_id;
    Ref<GDKResult> config_result = _resolve_configuration_id(&configuration_id);
    if (!config_result->is_ok()) {
        return config_result;
    }

    const CharString configuration_id_utf8 = configuration_id.utf8();
    int64_t remaining_quota = 0;
    HRESULT hr = XGameSaveFilesGetRemainingQuota(
            p_user->get_handle(),
            configuration_id_utf8.get_data(),
            &remaining_quota);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(
                hr,
                "Failed to query the remaining game save quota.",
                "game_save_quota_failed");
    }

    Dictionary data;
    data["bytes"] = remaining_quota;
    return GDKResult::ok_result(data);
}

} // namespace godot
