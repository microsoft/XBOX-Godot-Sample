#ifndef GDK_XASYNC_CONTEXT_H
#define GDK_XASYNC_CONTEXT_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <XAsync.h>

#include "gdk_async_op.h"
#include "gdk_runtime.h"

namespace godot {

class GDKXAsyncContext {
    XAsyncBlock m_async_block = {};

    static void CALLBACK _completion_thunk(XAsyncBlock *p_async_block);

protected:
    GDKRuntime *m_runtime = nullptr;
    Ref<GDKAsyncOp> m_op;

    virtual void finalize(XAsyncBlock *p_async_block) = 0;

public:
    GDKXAsyncContext(GDKRuntime *p_runtime, const Ref<GDKAsyncOp> &p_op);
    virtual ~GDKXAsyncContext() = default;

    XAsyncBlock *get_async_block();
    GDKRuntime *get_runtime() const;
    Ref<GDKAsyncOp> get_op() const;
    void bind_cancel_handler();
    void clear_cancel_handler();
};

} // namespace godot

#endif // GDK_XASYNC_CONTEXT_H
