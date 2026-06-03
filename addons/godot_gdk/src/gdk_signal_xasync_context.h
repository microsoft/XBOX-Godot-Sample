#ifndef GDK_SIGNAL_XASYNC_CONTEXT_H
#define GDK_SIGNAL_XASYNC_CONTEXT_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <XAsync.h>

#include "gdk_pending_signal.h"
#include "gdk_runtime.h"

namespace godot {

class GDKSignalXAsyncContext {
    XAsyncBlock m_async_block = {};

    static void CALLBACK _completion_thunk(XAsyncBlock *p_async_block);

protected:
    GDKRuntime *m_runtime = nullptr;
    Ref<GDKPendingSignal> m_pending_signal;

    virtual void finalize(XAsyncBlock *p_async_block) = 0;

public:
    GDKSignalXAsyncContext(GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal);
    virtual ~GDKSignalXAsyncContext() = default;

    XAsyncBlock *get_async_block();
    GDKRuntime *get_runtime() const;
    Ref<GDKPendingSignal> get_pending_signal() const;
    void bind_cancel_handler();
    void clear_cancel_handler();
};

} // namespace godot

#endif // GDK_SIGNAL_XASYNC_CONTEXT_H
