#include "gdk_runtime.h"

#include <algorithm>

#include "gdk_pending_signal.h"
#include "gdk_result.h"

namespace godot {

namespace {

#if defined(_GAMING_DESKTOP)
constexpr bool GDK_PLATFORM_AVAILABLE = true;
#else
constexpr bool GDK_PLATFORM_AVAILABLE = false;
#endif

} // namespace

GDKRuntime::GDKRuntime() {
}

GDKRuntime::~GDKRuntime() {
    shutdown();

    // XGameRuntimeUninitialize is the matching half of the process-lifetime
    // pair: call it exactly once, when the GDK extension is being torn down
    // at process exit (~GDKRuntime is reached via memdelete(gdk_singleton) in
    // uninitialize_gdk_extension). Do NOT call it from shutdown() above -
    // tests cycle gdk.initialize()/gdk.shutdown() many times per process, and
    // doing Initialize/Uninitialize on every cycle leaks GDK background
    // threads that race DLL unload at process exit.
    if (m_xgame_runtime_initialized) {
        XGameRuntimeUninitialize();
        m_xgame_runtime_initialized = false;
    }
}

Ref<GDKResult> GDKRuntime::initialize() {
    if (m_initialized) {
        Ref<GDKResult> result = GDKResult::error_result(E_FAIL, "already_initialized", "GDK runtime is already initialized.");
        return result;
    }

    if (!m_xgame_runtime_initialized) {
        HRESULT hr = XGameRuntimeInitialize();
        if (FAILED(hr)) {
            Ref<GDKResult> result = GDKResult::hresult_error(hr, "Failed to initialize GDK runtime.", "runtime_initialize_failed");
            return result;
        }
        m_xgame_runtime_initialized = true;
    }

    HRESULT hr = XTaskQueueCreate(
        XTaskQueueDispatchMode::ThreadPool,
        XTaskQueueDispatchMode::Manual,
        &m_task_queue);
    if (FAILED(hr)) {
        // Leave m_xgame_runtime_initialized true so the next initialize()
        // attempt skips redundant XGameRuntimeInitialize. The matching
        // Uninitialize runs from ~GDKRuntime().
        Ref<GDKResult> result = GDKResult::hresult_error(hr, "Failed to create the shared XTaskQueue.", "task_queue_create_failed");
        return result;
    }

    m_initialized = true;
    m_shutting_down = false;
    return GDKResult::ok_result();
}

void GDKRuntime::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_shutting_down = true;

    std::vector<Ref<GDKPendingSignal>> active_pending_signals = m_active_pending_signals;
    for (const Ref<GDKPendingSignal> &pending_signal : active_pending_signals) {
        if (pending_signal.is_valid()) {
            pending_signal->cancel();
            if (!pending_signal->is_done()) {
                // Complete synchronously. `complete_deferred` would queue via
                // call_deferred(), but when shutdown is invoked from SceneTree
                // teardown (e.g. the bootstrap _exit_tree path) there may be
                // no idle frame left to drain the deferred call, leaving
                // awaiters stranded. Calling `complete` here resolves them
                // immediately; any deferred emit that was already queued by
                // the cancel handler no-ops via GDKPendingSignal::complete's
                // m_done check.
                pending_signal->complete(GDKResult::cancelled("GDK runtime shutdown cancelled the async request."));
            }
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

    for (Ref<GDKPendingSignal> &pending_signal : m_active_pending_signals) {
        if (pending_signal.is_valid()) {
            pending_signal->clear_cancel_handler();
            pending_signal->clear_release_handler();
        }
    }
    m_active_pending_signals.clear();

    // NOTE: XGameRuntimeUninitialize is intentionally NOT called here. It is
    // the process-lifetime peer of XGameRuntimeInitialize and runs once from
    // ~GDKRuntime() at extension teardown. See class-level comment.

    m_initialized = false;
    m_shutting_down = false;
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
    return GDK_PLATFORM_AVAILABLE;
}

XTaskQueueHandle GDKRuntime::get_task_queue() const {
    return m_task_queue;
}

void GDKRuntime::retain_pending_signal(const Ref<GDKPendingSignal> &p_pending_signal) {
    if (!p_pending_signal.is_valid() || p_pending_signal->is_done()) {
        return;
    }

    p_pending_signal->set_release_handler([this](GDKPendingSignal *p_completed_signal) {
        release_pending_signal(p_completed_signal);
    });
    m_active_pending_signals.push_back(p_pending_signal);
}

void GDKRuntime::release_pending_signal(GDKPendingSignal *p_pending_signal) {
    m_active_pending_signals.erase(
        std::remove_if(
            m_active_pending_signals.begin(),
            m_active_pending_signals.end(),
            [p_pending_signal](const Ref<GDKPendingSignal> &candidate) {
                return candidate.is_null() || candidate.operator->() == p_pending_signal;
            }),
        m_active_pending_signals.end());
}

Ref<GDKPendingSignal> GDKRuntime::make_pending_signal() {
    Ref<GDKPendingSignal> pending_signal;
    pending_signal.instantiate();
    retain_pending_signal(pending_signal);
    return pending_signal;
}

Signal GDKRuntime::make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    Ref<GDKPendingSignal> pending_signal = make_pending_signal();
    Ref<GDKResult> result = GDKResult::error_result(p_hresult, p_code, p_message, p_data);
    pending_signal->complete_deferred(result);
    return pending_signal->get_completed_signal();
}

void CALLBACK GDKRuntime::_queue_terminated(void *p_context) {
    bool *terminated = static_cast<bool *>(p_context);
    if (terminated != nullptr) {
        *terminated = true;
    }
}

} // namespace godot
