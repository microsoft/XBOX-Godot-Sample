#include "gdk_runtime.h"

#include <algorithm>

#include "gdk_async_op.h"
#include "gdk_dispatch_op.h"
#include "gdk_result.h"

namespace godot {

GDKRuntime::GDKRuntime() {
    clear_last_error();
}

GDKRuntime::~GDKRuntime() {
    shutdown();
}

Ref<GDKResult> GDKRuntime::initialize() {
    if (m_initialized) {
        Ref<GDKResult> result = GDKResult::error_result(E_FAIL, "already_initialized", "GDK runtime is already initialized.");
        set_last_error(result);
        return result;
    }

    HRESULT hr = XGameRuntimeInitialize();
    if (FAILED(hr)) {
        Ref<GDKResult> result = GDKResult::hresult_error(hr, "Failed to initialize GDK runtime.", "runtime_initialize_failed");
        set_last_error(result);
        return result;
    }

    hr = XTaskQueueCreate(
        XTaskQueueDispatchMode::ThreadPool,
        XTaskQueueDispatchMode::Manual,
        &m_task_queue);
    if (FAILED(hr)) {
        XGameRuntimeUninitialize();

        Ref<GDKResult> result = GDKResult::hresult_error(hr, "Failed to create the shared XTaskQueue.", "task_queue_create_failed");
        set_last_error(result);
        return result;
    }

    m_initialized = true;
    m_shutting_down = false;
    clear_last_error();
    return GDKResult::ok_result();
}

void GDKRuntime::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_shutting_down = true;

    std::vector<Ref<GDKAsyncOp>> active_ops = m_active_ops;
    for (const Ref<GDKAsyncOp> &op : active_ops) {
        if (op.is_valid()) {
            op->cancel();
        }
    }

    if (m_task_queue) {
        bool terminated = false;
        HRESULT terminate_hr = XTaskQueueTerminate(m_task_queue, false, &terminated, _queue_terminated);
        if (SUCCEEDED(terminate_hr)) {
            while (!terminated) {
                XTaskQueueDispatch(m_task_queue, XTaskQueuePort::Completion, 10);
            }
        }

        XTaskQueueCloseHandle(m_task_queue);
        m_task_queue = nullptr;
    }

    for (Ref<GDKAsyncOp> &op : m_active_ops) {
        if (op.is_valid()) {
            op->clear_cancel_handler();
            op->clear_release_handler();
        }
    }
    m_active_ops.clear();

    XGameRuntimeUninitialize();

    m_initialized = false;
    m_shutting_down = false;
    clear_last_error();
}

int GDKRuntime::dispatch() {
    if (!m_initialized || !m_task_queue) {
        return 0;
    }

    int dispatched = 0;
    while (XTaskQueueDispatch(m_task_queue, XTaskQueuePort::Completion, 0)) {
        ++dispatched;
    }

    return dispatched;
}

bool GDKRuntime::is_initialized() const {
    return m_initialized;
}

bool GDKRuntime::is_shutting_down() const {
    return m_shutting_down;
}

bool GDKRuntime::is_available() const {
    return true;
}

XTaskQueueHandle GDKRuntime::get_task_queue() const {
    return m_task_queue;
}

void GDKRuntime::retain_op(const Ref<GDKAsyncOp> &p_op) {
    if (!p_op.is_valid() || p_op->is_done()) {
        return;
    }

    p_op->set_release_handler([this](GDKAsyncOp *p_completed_op) {
        release_op(p_completed_op);
    });
    m_active_ops.push_back(p_op);
}

void GDKRuntime::release_op(GDKAsyncOp *p_op) {
    m_active_ops.erase(
        std::remove_if(
            m_active_ops.begin(),
            m_active_ops.end(),
            [p_op](const Ref<GDKAsyncOp> &candidate) {
                return candidate.is_null() || candidate.operator->() == p_op;
            }),
        m_active_ops.end());
}

Ref<GDKAsyncOp> GDKRuntime::make_completed_async_op(const Ref<GDKResult> &p_result) {
    Ref<GDKAsyncOp> op;
    op.instantiate();
    op->complete(p_result);
    return op;
}

Ref<GDKAsyncOp> GDKRuntime::make_error_async_op(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    Ref<GDKResult> result = GDKResult::error_result(p_hresult, p_code, p_message, p_data);
    set_last_error(result);
    return make_completed_async_op(result);
}

Ref<GDKDispatchOp> GDKRuntime::make_completed_dispatch_op(const Ref<GDKResult> &p_result) {
    Ref<GDKDispatchOp> op;
    op.instantiate();
    op->complete(p_result);
    return op;
}

Ref<GDKDispatchOp> GDKRuntime::make_error_dispatch_op(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    Ref<GDKResult> result = GDKResult::error_result(p_hresult, p_code, p_message, p_data);
    set_last_error(result);
    return make_completed_dispatch_op(result);
}

Ref<GDKResult> GDKRuntime::get_last_error() const {
    return m_last_error;
}

void GDKRuntime::set_last_error(const Ref<GDKResult> &p_result) {
    m_last_error = p_result;
}

void GDKRuntime::clear_last_error() {
    m_last_error = GDKResult::ok_result();
}

void CALLBACK GDKRuntime::_queue_terminated(void *p_context) {
    bool *terminated = static_cast<bool *>(p_context);
    if (terminated != nullptr) {
        *terminated = true;
    }
}

} // namespace godot
