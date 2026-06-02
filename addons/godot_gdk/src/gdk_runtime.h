#ifndef GDK_RUNTIME_H
#define GDK_RUNTIME_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <XGameRuntimeInit.h>
#include <XTaskQueue.h>

namespace godot {

class GDKPendingSignal;
class GDKResult;

class GDKRuntime {
    bool m_initialized = false;
    bool m_shutting_down = false;
    // XGameRuntimeInitialize(WithOptions) is documented as a process-lifetime
    // call (paired with one XGameRuntimeUninitialize before process exit).
    // Repeatedly cycling Initialize/Uninitialize within a single process - as
    // the test harness does across many gdk.initialize()/gdk.shutdown() rounds
    // - leaks GDK runtime threads and triggers a SIGSEGV at process exit
    // during DLL unload (fault module ntdll.dll). Track the global-init state
    // separately from the per-session m_initialized flag so the runtime is
    // initialized at most once per process and XGameRuntimeUninitialize runs
    // only from ~GDKRuntime(). The init call resolves to
    // XGameRuntimeInitializeWithOptions(File, "<abs path>") when
    // res://MicrosoftGame.config is on disk (so unpackaged dev runs get
    // package identity from the project-root config), and falls back to
    // XGameRuntimeInitialize() for packaged GDK launches that get identity
    // from the registered package.
    bool m_xgame_runtime_initialized = false;
    XTaskQueueHandle m_task_queue = nullptr;
    std::vector<Ref<GDKPendingSignal>> m_active_pending_signals;

    static void CALLBACK _queue_terminated(void *p_context);

public:
    GDKRuntime();
    ~GDKRuntime();

    Ref<GDKResult> initialize();
    void shutdown();
    int dispatch();

    bool is_initialized() const;
    bool is_shutting_down() const;
    bool is_available() const;
    XTaskQueueHandle get_task_queue() const;

    void retain_pending_signal(const Ref<GDKPendingSignal> &p_pending_signal);
    void release_pending_signal(GDKPendingSignal *p_pending_signal);
    Ref<GDKPendingSignal> make_pending_signal();
    Signal make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant());
};

} // namespace godot

#endif // GDK_RUNTIME_H
