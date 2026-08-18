#ifndef XBOX_SIGNAL_XASYNC_CONTEXT_H
#define XBOX_SIGNAL_XASYNC_CONTEXT_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <XAsync.h>

#include "xbox_pending_signal.h"
#include "xbox_runtime.h"

namespace godot {

class XboxSignalXAsyncContext {
    XAsyncBlock m_async_block = {};

    static void CALLBACK _completion_thunk(XAsyncBlock *p_async_block);

protected:
    XboxRuntime *m_runtime = nullptr;
    Ref<XboxPendingSignal> m_pending_signal;

    virtual void finalize(XAsyncBlock *p_async_block) = 0;

public:
    XboxSignalXAsyncContext(XboxRuntime *p_runtime, const Ref<XboxPendingSignal> &p_pending_signal);
    virtual ~XboxSignalXAsyncContext() = default;

    XAsyncBlock *get_async_block();
    XboxRuntime *get_runtime() const;
    Ref<XboxPendingSignal> get_pending_signal() const;
    void bind_cancel_handler();
    void clear_cancel_handler();
};

} // namespace godot

#endif // XBOX_SIGNAL_XASYNC_CONTEXT_H
