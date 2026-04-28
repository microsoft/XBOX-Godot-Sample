#ifndef GDK_ASYNC_OP_H
#define GDK_ASYNC_OP_H

#include <functional>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "gdk_result.h"

namespace godot {

class GDKAsyncOp : public RefCounted {
    GDCLASS(GDKAsyncOp, RefCounted);

    bool m_done = false;
    bool m_cancel_requested = false;
    Ref<GDKResult> m_result;
    std::function<void()> m_cancel_handler;
    std::function<void(GDKAsyncOp *)> m_release_handler;

protected:
    static void _bind_methods();
    bool request_cancel();
    void invoke_cancel_handler();

public:
    bool is_done() const;
    virtual bool cancel();
    Ref<GDKResult> get_result() const;

    bool was_cancel_requested() const;
    void complete(const Ref<GDKResult> &p_result);
    void set_cancel_handler(std::function<void()> p_handler);
    void clear_cancel_handler();
    void set_release_handler(std::function<void(GDKAsyncOp *)> p_handler);
    void clear_release_handler();
};

} // namespace godot

#endif // GDK_ASYNC_OP_H
