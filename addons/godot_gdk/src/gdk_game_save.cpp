#include "gdk_game_save.h"

#include <vector>

#include <godot_cpp/variant/dictionary.hpp>

#include <XAsyncProvider.h>
#include <XGameSaveFiles.h>
#include <XUser.h>

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

// XGameSaveFilesGetRemainingQuota is synchronous, and the GDK fails it with
// E_GS_ASYNC_FUNCTION_REQUIRED (0x8083000E) when it is called from a
// time-sensitive thread - which Godot's main thread is. XGameSaveFiles has no
// async quota entry point, so the synchronous call is wrapped in a custom
// XAsync provider that runs it on the shared task queue's work port (a thread
// pool thread) and reports back through the queue's completion port on the
// main thread.
const char *const REMAINING_QUOTA_IDENTITY = "GDKGameSave::get_remaining_quota_async";

class GetRemainingQuotaAsyncContext final : public GDKSignalXAsyncContext {
    XUserHandle m_user = nullptr;
    CharString m_configuration_id_utf8;
    int64_t m_remaining_quota = 0;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Game save quota request cancelled."));
            return;
        }

        int64_t remaining_quota = 0;
        HRESULT result_hr = XAsyncGetResult(
                p_async_block,
                REMAINING_QUOTA_IDENTITY,
                sizeof(remaining_quota),
                &remaining_quota,
                nullptr);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Game save quota request cancelled."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to query the remaining game save quota.",
                    "game_save_quota_failed"));
            return;
        }

        Dictionary data;
        data["bytes"] = remaining_quota;
        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    GetRemainingQuotaAsyncContext(
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            XUserHandle p_user,
            const String &p_configuration_id) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_configuration_id_utf8(p_configuration_id.utf8()) {}

    // Deleted on the main thread by the shared completion thunk, or directly by
    // the caller when XAsyncBegin fails.
    ~GetRemainingQuotaAsyncContext() override {
        if (m_user != nullptr) {
            XUserCloseHandle(m_user);
            m_user = nullptr;
        }
    }

    HRESULT run_quota_query() {
        return XGameSaveFilesGetRemainingQuota(
                m_user,
                m_configuration_id_utf8.get_data(),
                &m_remaining_quota);
    }

    int64_t get_remaining_quota() const {
        return m_remaining_quota;
    }
};

// The provider context is the GDKSignalXAsyncContext itself, which the shared
// completion thunk deletes. XAsyncOp::Cleanup therefore intentionally does not
// free it: doing so would double-free whenever XAsyncBegin fails and the caller
// deletes the context directly.
HRESULT CALLBACK _remaining_quota_provider(XAsyncOp p_op, const XAsyncProviderData *p_data) {
    auto *context = static_cast<GetRemainingQuotaAsyncContext *>(p_data->context);

    switch (p_op) {
        case XAsyncOp::Begin:
            return XAsyncSchedule(p_data->async, 0);

        case XAsyncOp::DoWork: {
            const HRESULT hr = context->run_quota_query();
            XAsyncComplete(p_data->async, hr, FAILED(hr) ? 0 : sizeof(int64_t));
            return S_OK;
        }

        case XAsyncOp::GetResult:
            if (p_data->buffer != nullptr && p_data->bufferSize >= sizeof(int64_t)) {
                *static_cast<int64_t *>(p_data->buffer) = context->get_remaining_quota();
            }
            return S_OK;

        case XAsyncOp::Cancel:
        case XAsyncOp::Cleanup:
        default:
            return S_OK;
    }
}

} // namespace

void GDKGameSave::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_folder_async", "user"), &GDKGameSave::get_folder_async);
    ClassDB::bind_method(D_METHOD("get_remaining_quota_async", "user"), &GDKGameSave::get_remaining_quota_async);
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

Signal GDKGameSave::get_remaining_quota_async(const Ref<GDKUser> &p_user) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return _make_error_signal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr || !p_user->is_signed_in()) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to query the game save quota.");
    }

    String configuration_id;
    Ref<GDKResult> config_result = _resolve_configuration_id(&configuration_id);
    if (!config_result->is_ok()) {
        return _make_error_signal(static_cast<HRESULT>(config_result->get_hresult()), config_result->get_code(), config_result->get_message());
    }

    // The quota call runs on a work-port thread, so it needs a handle it owns
    // for the lifetime of the request rather than the GDKUser's handle, which
    // the main thread may release while the request is in flight.
    XUserHandle user_handle = nullptr;
    HRESULT duplicate_hr = XUserDuplicateHandle(p_user->get_handle(), &user_handle);
    if (FAILED(duplicate_hr)) {
        return _make_error_signal(duplicate_hr, "invalid_user", "Failed to duplicate the user handle for the game save quota request.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new GetRemainingQuotaAsyncContext(runtime, pending_signal, user_handle, configuration_id);

    HRESULT hr = XAsyncBegin(
            context->get_async_block(),
            context,
            REMAINING_QUOTA_IDENTITY,
            "GDKGameSave::get_remaining_quota_async",
            _remaining_quota_provider);
    if (FAILED(hr)) {
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                hr,
                "Failed to start the game save quota request.",
                "game_save_quota_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

} // namespace godot
