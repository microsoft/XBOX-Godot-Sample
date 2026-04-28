#include "gdk_xasync_context.h"

namespace godot {

GDKXAsyncContext::GDKXAsyncContext(GDKRuntime *p_runtime, const Ref<GDKAsyncOp> &p_op) :
        m_runtime(p_runtime),
        m_op(p_op) {
    m_async_block.queue = m_runtime != nullptr ? m_runtime->get_task_queue() : nullptr;
    m_async_block.context = this;
    m_async_block.callback = _completion_thunk;
}

XAsyncBlock *GDKXAsyncContext::get_async_block() {
    return &m_async_block;
}

GDKRuntime *GDKXAsyncContext::get_runtime() const {
    return m_runtime;
}

Ref<GDKAsyncOp> GDKXAsyncContext::get_op() const {
    return m_op;
}

void GDKXAsyncContext::bind_cancel_handler() {
    if (m_op.is_valid()) {
        m_op->set_cancel_handler([this]() {
            XAsyncCancel(&m_async_block);
        });
    }
}

void GDKXAsyncContext::clear_cancel_handler() {
    if (m_op.is_valid()) {
        m_op->clear_cancel_handler();
    }
}

void CALLBACK GDKXAsyncContext::_completion_thunk(XAsyncBlock *p_async_block) {
    auto *context = static_cast<GDKXAsyncContext *>(p_async_block->context);

    context->clear_cancel_handler();
    context->finalize(p_async_block);
    delete context;
}

} // namespace godot
