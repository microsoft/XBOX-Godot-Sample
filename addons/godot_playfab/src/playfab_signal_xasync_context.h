#ifndef GODOT_PLAYFAB_SIGNAL_XASYNC_CONTEXT_H
#define GODOT_PLAYFAB_SIGNAL_XASYNC_CONTEXT_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <XAsync.h>

#include "playfab_pending_signal.h"
#include "playfab_runtime.h"

namespace godot {

class PlayFabSignalXAsyncContext {
    XAsyncBlock m_async_block = {};

    static void CALLBACK _completion_thunk(XAsyncBlock *p_async_block);

protected:
    PlayFabRuntime *m_runtime = nullptr;
    Ref<PlayFabPendingSignal> m_pending_signal;

    virtual void finalize(XAsyncBlock *p_async_block) = 0;

public:
    PlayFabSignalXAsyncContext(PlayFabRuntime *p_runtime, const Ref<PlayFabPendingSignal> &p_pending_signal);
    virtual ~PlayFabSignalXAsyncContext() = default;

    XAsyncBlock *get_async_block();
    PlayFabRuntime *get_runtime() const;
    Ref<PlayFabPendingSignal> get_pending_signal() const;
    void bind_cancel_handler();
    void clear_cancel_handler();
};

} // namespace godot

#endif // GODOT_PLAYFAB_SIGNAL_XASYNC_CONTEXT_H
