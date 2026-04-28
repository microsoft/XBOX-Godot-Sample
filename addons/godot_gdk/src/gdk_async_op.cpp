#include "gdk_async_op.h"

namespace godot {

void GDKAsyncOp::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_done"), &GDKAsyncOp::is_done);
    ClassDB::bind_method(D_METHOD("cancel"), &GDKAsyncOp::cancel);
    ClassDB::bind_method(D_METHOD("get_result"), &GDKAsyncOp::get_result);

    ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::OBJECT, "result")));
}

bool GDKAsyncOp::is_done() const {
    return m_done;
}

bool GDKAsyncOp::request_cancel() {
    if (m_done || m_cancel_requested) {
        return false;
    }

    m_cancel_requested = true;
    return true;
}

void GDKAsyncOp::invoke_cancel_handler() {
    if (m_cancel_handler) {
        m_cancel_handler();
    }
}

bool GDKAsyncOp::cancel() {
    if (!request_cancel()) {
        return false;
    }

    invoke_cancel_handler();

    return true;
}

Ref<GDKResult> GDKAsyncOp::get_result() const {
    return m_result;
}

bool GDKAsyncOp::was_cancel_requested() const {
    return m_cancel_requested;
}

void GDKAsyncOp::complete(const Ref<GDKResult> &p_result) {
    if (m_done) {
        return;
    }

    Ref<GDKResult> final_result = p_result;
    if (!final_result.is_valid()) {
        final_result = GDKResult::error_result(E_FAIL, "internal_error", "Async operation completed without a result.");
    }

    if (m_cancel_requested && final_result->get_hresult() != static_cast<int64_t>(E_ABORT)) {
        final_result = GDKResult::cancelled();
    }

    m_result = final_result;
    m_done = true;
    m_cancel_handler = nullptr;

    emit_signal("completed", m_result);

    if (m_release_handler) {
        auto release_handler = m_release_handler;
        m_release_handler = nullptr;
        release_handler(this);
    }
}

void GDKAsyncOp::set_cancel_handler(std::function<void()> p_handler) {
    m_cancel_handler = std::move(p_handler);
}

void GDKAsyncOp::clear_cancel_handler() {
    m_cancel_handler = nullptr;
}

void GDKAsyncOp::set_release_handler(std::function<void(GDKAsyncOp *)> p_handler) {
    m_release_handler = std::move(p_handler);
}

void GDKAsyncOp::clear_release_handler() {
    m_release_handler = nullptr;
}

} // namespace godot
