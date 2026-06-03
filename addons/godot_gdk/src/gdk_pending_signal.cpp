#include "gdk_pending_signal.h"

namespace godot {

void GDKPendingSignal::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_emit_deferred_completion"), &GDKPendingSignal::_emit_deferred_completion);

    ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::OBJECT, "result")));
}

bool GDKPendingSignal::is_done() const {
    return m_done;
}

bool GDKPendingSignal::request_cancel() {
    if (m_done || m_cancel_requested) {
        return false;
    }

    m_cancel_requested = true;
    return true;
}

void GDKPendingSignal::invoke_cancel_handler() {
    if (m_cancel_handler) {
        m_cancel_handler();
    }
}

bool GDKPendingSignal::was_cancel_requested() const {
    return m_cancel_requested;
}

Signal GDKPendingSignal::get_completed_signal() const {
    return Signal(const_cast<GDKPendingSignal *>(this), StringName("completed"));
}

void GDKPendingSignal::cancel() {
    if (!request_cancel()) {
        return;
    }

    invoke_cancel_handler();
}

void GDKPendingSignal::complete(const Ref<GDKResult> &p_result) {
    if (m_done) {
        return;
    }

    Ref<GDKPendingSignal> self_guard(this);
    Ref<GDKResult> final_result = p_result;
    if (!final_result.is_valid()) {
        final_result = GDKResult::error_result(E_FAIL, "internal_error", "Async request completed without a result.");
    }

    if (m_cancel_requested && final_result->get_hresult() != static_cast<int64_t>(E_ABORT)) {
        final_result = GDKResult::cancelled();
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

void GDKPendingSignal::complete_deferred(const Ref<GDKResult> &p_result) {
    if (m_done || m_deferred_completion_queued) {
        return;
    }

    if (m_self_ref.is_null()) {
        m_self_ref = Ref<GDKPendingSignal>(this);
    }

    m_deferred_result = p_result;
    m_deferred_completion_queued = true;
    call_deferred("_emit_deferred_completion");
}

void GDKPendingSignal::_emit_deferred_completion() {
    if (m_done) {
        return;
    }

    Ref<GDKResult> result = m_deferred_result;
    m_deferred_completion_queued = false;
    m_deferred_result.unref();
    complete(result);
}

void GDKPendingSignal::set_cancel_handler(std::function<void()> p_handler) {
    m_cancel_handler = std::move(p_handler);
}

void GDKPendingSignal::clear_cancel_handler() {
    m_cancel_handler = nullptr;
}

void GDKPendingSignal::set_release_handler(std::function<void(GDKPendingSignal *)> p_handler) {
    m_release_handler = std::move(p_handler);
}

void GDKPendingSignal::clear_release_handler() {
    m_release_handler = nullptr;
}

} // namespace godot
