#ifndef GODOT_PLAYFAB_GENERATED_API_HELPERS_H
#define GODOT_PLAYFAB_GENERATED_API_HELPERS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <memory>
#include <string>
#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "playfab_pending_signal.h"
#include "playfab_result.h"
#include "playfab_runtime.h"
#include "playfab_signal_xasync_context.h"
#include "playfab_user.h"

#include <XAsync.h>
#include <playfab/core/PFEntity.h>

#ifdef CONNECT_DEFERRED
#undef CONNECT_DEFERRED
#endif

namespace godot {
namespace playfab_generated {

using VariantEncoder = Variant (*)(const void *);

Signal make_error_signal(PlayFabRuntime *p_runtime, HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant());
bool get_request_value(const Dictionary &p_request, const char *p_field_name, const char *p_snake_name, Variant *r_value);
String variant_to_json_string(const Variant &p_value);

template <typename RequestOwner, typename RequestT, HRESULT (*StartFn)(PFEntityHandle, const RequestT *, XAsyncBlock *)>
class EntityVoidCallContext final : public PlayFabSignalXAsyncContext {
    Ref<PlayFabUser> m_user;
    std::unique_ptr<RequestOwner> m_request;
    String m_operation_name;

public:
    EntityVoidCallContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            std::unique_ptr<RequestOwner> &&p_request,
            const String &p_operation_name) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_request(std::move(p_request)),
            m_operation_name(p_operation_name) {}

    HRESULT start() {
        return StartFn(m_user->get_entity_handle(), &m_request->value, get_async_block());
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        const HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "PlayFab API call failed: " + m_operation_name + ".", "playfab_api_call_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        get_runtime()->clear_last_error();
        get_pending_signal()->complete(PlayFabResult::ok_result());
    }
};

template <typename RequestOwner, typename RequestT, typename ResultT,
        HRESULT (*StartFn)(PFEntityHandle, const RequestT *, XAsyncBlock *),
        HRESULT (*GetResultFn)(XAsyncBlock *, ResultT *),
        Variant (*EncodeFn)(const ResultT *)>
class EntityFixedResultCallContext final : public PlayFabSignalXAsyncContext {
    Ref<PlayFabUser> m_user;
    std::unique_ptr<RequestOwner> m_request;
    String m_operation_name;

public:
    EntityFixedResultCallContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            std::unique_ptr<RequestOwner> &&p_request,
            const String &p_operation_name) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_request(std::move(p_request)),
            m_operation_name(p_operation_name) {}

    HRESULT start() {
        return StartFn(m_user->get_entity_handle(), &m_request->value, get_async_block());
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        const HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "PlayFab API call failed: " + m_operation_name + ".", "playfab_api_call_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        ResultT service_result = {};
        const HRESULT result_hr = GetResultFn(p_async_block, &service_result);
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, "Failed to retrieve PlayFab API result for " + m_operation_name + ".", "playfab_api_result_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        get_runtime()->clear_last_error();
        get_pending_signal()->complete(PlayFabResult::ok_result(EncodeFn(&service_result)));
    }
};

template <typename RequestOwner, typename RequestT, typename ResultT,
        HRESULT (*StartFn)(PFEntityHandle, const RequestT *, XAsyncBlock *),
        HRESULT (*GetResultSizeFn)(XAsyncBlock *, size_t *),
        HRESULT (*GetResultFn)(XAsyncBlock *, size_t, void *, ResultT **, size_t *),
        Variant (*EncodeFn)(const ResultT *)>
class EntityVariableResultCallContext final : public PlayFabSignalXAsyncContext {
    Ref<PlayFabUser> m_user;
    std::unique_ptr<RequestOwner> m_request;
    String m_operation_name;

public:
    EntityVariableResultCallContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            std::unique_ptr<RequestOwner> &&p_request,
            const String &p_operation_name) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_request(std::move(p_request)),
            m_operation_name(p_operation_name) {}

    HRESULT start() {
        return StartFn(m_user->get_entity_handle(), &m_request->value, get_async_block());
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        const HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "PlayFab API call failed: " + m_operation_name + ".", "playfab_api_call_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        size_t buffer_size = 0;
        const HRESULT size_hr = GetResultSizeFn(p_async_block, &buffer_size);
        if (FAILED(size_hr)) {
            result = PlayFabResult::hresult_error(size_hr, "Failed to retrieve PlayFab API result size for " + m_operation_name + ".", "playfab_api_result_size_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<char> buffer(buffer_size > 0 ? buffer_size : 1);
        ResultT *service_result = nullptr;
        const HRESULT result_hr = GetResultFn(p_async_block, buffer.size(), buffer.data(), &service_result, nullptr);
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, "Failed to retrieve PlayFab API result for " + m_operation_name + ".", "playfab_api_result_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        get_runtime()->clear_last_error();
        get_pending_signal()->complete(PlayFabResult::ok_result(EncodeFn(service_result)));
    }
};

template <typename ResultT,
        HRESULT (*StartFn)(PFEntityHandle, XAsyncBlock *),
        HRESULT (*GetResultFn)(XAsyncBlock *, ResultT *),
        Variant (*EncodeFn)(const ResultT *)>
class EntityNoRequestFixedResultCallContext final : public PlayFabSignalXAsyncContext {
    Ref<PlayFabUser> m_user;
    String m_operation_name;

public:
    EntityNoRequestFixedResultCallContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            const String &p_operation_name) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_operation_name(p_operation_name) {}

    HRESULT start() {
        return StartFn(m_user->get_entity_handle(), get_async_block());
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        const HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "PlayFab API call failed: " + m_operation_name + ".", "playfab_api_call_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        ResultT service_result = {};
        const HRESULT result_hr = GetResultFn(p_async_block, &service_result);
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, "Failed to retrieve PlayFab API result for " + m_operation_name + ".", "playfab_api_result_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        get_runtime()->clear_last_error();
        get_pending_signal()->complete(PlayFabResult::ok_result(EncodeFn(&service_result)));
    }
};

template <typename ResultT,
        HRESULT (*StartFn)(PFEntityHandle, XAsyncBlock *),
        HRESULT (*GetResultSizeFn)(XAsyncBlock *, size_t *),
        HRESULT (*GetResultFn)(XAsyncBlock *, size_t, void *, ResultT **, size_t *),
        Variant (*EncodeFn)(const ResultT *)>
class EntityNoRequestVariableResultCallContext final : public PlayFabSignalXAsyncContext {
    Ref<PlayFabUser> m_user;
    String m_operation_name;

public:
    EntityNoRequestVariableResultCallContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            const String &p_operation_name) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_operation_name(p_operation_name) {}

    HRESULT start() {
        return StartFn(m_user->get_entity_handle(), get_async_block());
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        const HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled(m_operation_name + " cancelled.");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "PlayFab API call failed: " + m_operation_name + ".", "playfab_api_call_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        size_t buffer_size = 0;
        const HRESULT size_hr = GetResultSizeFn(p_async_block, &buffer_size);
        if (FAILED(size_hr)) {
            result = PlayFabResult::hresult_error(size_hr, "Failed to retrieve PlayFab API result size for " + m_operation_name + ".", "playfab_api_result_size_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<char> buffer(buffer_size > 0 ? buffer_size : 1);
        ResultT *service_result = nullptr;
        const HRESULT result_hr = GetResultFn(p_async_block, buffer.size(), buffer.data(), &service_result, nullptr);
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, "Failed to retrieve PlayFab API result for " + m_operation_name + ".", "playfab_api_result_failed");
            get_runtime()->set_last_error(result);
            get_pending_signal()->complete(result);
            return;
        }

        get_runtime()->clear_last_error();
        get_pending_signal()->complete(PlayFabResult::ok_result(EncodeFn(service_result)));
    }
};

} // namespace playfab_generated
} // namespace godot

#endif // GODOT_PLAYFAB_GENERATED_API_HELPERS_H
