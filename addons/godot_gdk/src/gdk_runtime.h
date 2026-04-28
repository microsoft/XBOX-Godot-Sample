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

class GDKAsyncOp;
class GDKDispatchOp;
class GDKResult;

class GDKRuntime {
    bool m_initialized = false;
    bool m_shutting_down = false;
    XTaskQueueHandle m_task_queue = nullptr;
    std::vector<Ref<GDKAsyncOp>> m_active_ops;
    Ref<GDKResult> m_last_error;

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

    void retain_op(const Ref<GDKAsyncOp> &p_op);
    void release_op(GDKAsyncOp *p_op);
    Ref<GDKAsyncOp> make_completed_async_op(const Ref<GDKResult> &p_result);
    Ref<GDKAsyncOp> make_error_async_op(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant());
    Ref<GDKDispatchOp> make_completed_dispatch_op(const Ref<GDKResult> &p_result);
    Ref<GDKDispatchOp> make_error_dispatch_op(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant());

    Ref<GDKResult> get_last_error() const;
    void set_last_error(const Ref<GDKResult> &p_result);
    void clear_last_error();
};

} // namespace godot

#endif // GDK_RUNTIME_H
