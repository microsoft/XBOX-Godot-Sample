#include "playfab_pending_signal.h"

namespace godot {

void PlayFabPendingSignal::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_emit_deferred_completion"), &PlayFabPendingSignal::_emit_deferred_completion);

    ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::OBJECT, "result")));
}

bool PlayFabPendingSignal::is_done() const {
    return m_done;
}

bool PlayFabPendingSignal::request_cancel() {
    if (m_done || m_cancel_requested) {
        return false;
    }

    m_cancel_requested = true;
    return true;
}

void PlayFabPendingSignal::invoke_cancel_handler() {
    if (m_cancel_handler) {
        m_cancel_handler();
    }
}

bool PlayFabPendingSignal::was_cancel_requested() const {
    return m_cancel_requested;
}

Signal PlayFabPendingSignal::get_completed_signal() const {
    return Signal(const_cast<PlayFabPendingSignal *>(this), StringName("completed"));
}

void PlayFabPendingSignal::cancel() {
    if (!request_cancel()) {
        return;
    }

    invoke_cancel_handler();
}

void PlayFabPendingSignal::complete(const Ref<PlayFabResult> &p_result) {
    if (m_done) {
        return;
    }

    Ref<PlayFabPendingSignal> self_guard(this);
    Ref<PlayFabResult> final_result = p_result;
    if (!final_result.is_valid()) {
        final_result = PlayFabResult::error_result(E_FAIL, "internal_error", "Async request completed without a result.");
    }

    if (m_cancel_requested && final_result->get_hresult() != static_cast<int64_t>(E_ABORT)) {
        final_result = PlayFabResult::cancelled();
    }

    m_result = final_result;
    m_done = true;
    m_deferred_completion_queued = false;
    m_deferred_result.unref();
    m_cancel_handler = nullptr;

    emit_signal("completed", m_result);

    if (m_release_handler) {
        auto release_handler = m_release_handler;
        m_release_handler = nullptr;
        release_handler(this);
    }

    m_self_ref.unref();
}

void PlayFabPendingSignal::complete_deferred(const Ref<PlayFabResult> &p_result) {
    if (m_done || m_deferred_completion_queued) {
        return;
    }

    if (m_self_ref.is_null()) {
        m_self_ref = Ref<PlayFabPendingSignal>(this);
    }

    m_deferred_result = p_result;
    m_deferred_completion_queued = true;
    call_deferred("_emit_deferred_completion");
}

void PlayFabPendingSignal::_emit_deferred_completion() {
    if (m_done) {
        return;
    }

    Ref<PlayFabResult> result = m_deferred_result;
    m_deferred_completion_queued = false;
    m_deferred_result.unref();
    complete(result);
}

void PlayFabPendingSignal::set_cancel_handler(std::function<void()> p_handler) {
    m_cancel_handler = std::move(p_handler);
}

void PlayFabPendingSignal::clear_cancel_handler() {
    m_cancel_handler = nullptr;
}

void PlayFabPendingSignal::set_release_handler(std::function<void(PlayFabPendingSignal *)> p_handler) {
    m_release_handler = std::move(p_handler);
}

void PlayFabPendingSignal::clear_release_handler() {
    m_release_handler = nullptr;
}

} // namespace godot
