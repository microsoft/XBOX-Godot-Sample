#include "xbox_pending_signal.h"

namespace godot {

void XboxPendingSignal::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_emit_deferred_completion"), &XboxPendingSignal::_emit_deferred_completion);

    ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::OBJECT, "result")));
}

bool XboxPendingSignal::is_done() const {
    return m_done;
}

bool XboxPendingSignal::request_cancel() {
    if (m_done || m_cancel_requested) {
        return false;
    }

    m_cancel_requested = true;
    return true;
}

void XboxPendingSignal::invoke_cancel_handler() {
    if (m_cancel_handler) {
        m_cancel_handler();
    }
}

bool XboxPendingSignal::was_cancel_requested() const {
    return m_cancel_requested;
}

Signal XboxPendingSignal::get_completed_signal() const {
    return Signal(const_cast<XboxPendingSignal *>(this), StringName("completed"));
}

void XboxPendingSignal::cancel() {
    if (!request_cancel()) {
        return;
    }

    invoke_cancel_handler();
}

void XboxPendingSignal::complete(const Ref<XboxResult> &p_result) {
    if (m_done) {
        return;
    }

    Ref<XboxPendingSignal> self_guard(this);
    Ref<XboxResult> final_result = p_result;
    if (!final_result.is_valid()) {
        final_result = XboxResult::error_result(E_FAIL, "internal_error", "Async request completed without a result.");
    }

    if (m_cancel_requested && final_result->get_hresult() != static_cast<int64_t>(E_ABORT)) {
        final_result = XboxResult::cancelled();
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

void XboxPendingSignal::complete_deferred(const Ref<XboxResult> &p_result) {
    if (m_done || m_deferred_completion_queued) {
        return;
    }

    if (m_self_ref.is_null()) {
        m_self_ref = Ref<XboxPendingSignal>(this);
    }

    m_deferred_result = p_result;
    m_deferred_completion_queued = true;
    call_deferred("_emit_deferred_completion");
}

void XboxPendingSignal::_emit_deferred_completion() {
    if (m_done) {
        return;
    }

    Ref<XboxResult> result = m_deferred_result;
    m_deferred_completion_queued = false;
    m_deferred_result.unref();
    complete(result);
}

void XboxPendingSignal::set_cancel_handler(std::function<void()> p_handler) {
    m_cancel_handler = std::move(p_handler);
}

void XboxPendingSignal::clear_cancel_handler() {
    m_cancel_handler = nullptr;
}

void XboxPendingSignal::set_release_handler(std::function<void(XboxPendingSignal *)> p_handler) {
    m_release_handler = std::move(p_handler);
}

void XboxPendingSignal::clear_release_handler() {
    m_release_handler = nullptr;
}

} // namespace godot
