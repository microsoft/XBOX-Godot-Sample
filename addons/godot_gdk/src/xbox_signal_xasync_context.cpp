#include "xbox_signal_xasync_context.h"

namespace godot {

XboxSignalXAsyncContext::XboxSignalXAsyncContext(XboxRuntime *p_runtime, const Ref<XboxPendingSignal> &p_pending_signal) :
        m_runtime(p_runtime),
        m_pending_signal(p_pending_signal) {
    m_async_block.queue = m_runtime != nullptr ? m_runtime->get_task_queue() : nullptr;
    m_async_block.context = this;
    m_async_block.callback = _completion_thunk;
}

XAsyncBlock *XboxSignalXAsyncContext::get_async_block() {
    return &m_async_block;
}

XboxRuntime *XboxSignalXAsyncContext::get_runtime() const {
    return m_runtime;
}

Ref<XboxPendingSignal> XboxSignalXAsyncContext::get_pending_signal() const {
    return m_pending_signal;
}

void XboxSignalXAsyncContext::bind_cancel_handler() {
    if (m_pending_signal.is_valid()) {
        m_pending_signal->set_cancel_handler([this]() {
            XAsyncCancel(&m_async_block);
        });
    }
}

void XboxSignalXAsyncContext::clear_cancel_handler() {
    if (m_pending_signal.is_valid()) {
        m_pending_signal->clear_cancel_handler();
    }
}

void CALLBACK XboxSignalXAsyncContext::_completion_thunk(XAsyncBlock *p_async_block) {
    auto *context = static_cast<XboxSignalXAsyncContext *>(p_async_block->context);

    context->clear_cancel_handler();
    context->finalize(p_async_block);
    delete context;
}

} // namespace godot
