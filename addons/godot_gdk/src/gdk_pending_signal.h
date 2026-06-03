#ifndef GDK_PENDING_SIGNAL_H
#define GDK_PENDING_SIGNAL_H

#include <functional>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "gdk_result.h"

namespace godot {

class GDKPendingSignal : public RefCounted {
    GDCLASS(GDKPendingSignal, RefCounted);

    bool m_done = false;
    bool m_cancel_requested = false;
    bool m_deferred_completion_queued = false;
    Ref<GDKResult> m_result;
    Ref<GDKResult> m_deferred_result;
    Ref<GDKPendingSignal> m_self_ref;
    std::function<void()> m_cancel_handler;
    std::function<void(GDKPendingSignal *)> m_release_handler;

    void _emit_deferred_completion();

protected:
    static void _bind_methods();
    bool request_cancel();
    void invoke_cancel_handler();

public:
    bool is_done() const;
    bool was_cancel_requested() const;
    Signal get_completed_signal() const;

    void cancel();
    void complete(const Ref<GDKResult> &p_result);
    void complete_deferred(const Ref<GDKResult> &p_result);

    void set_cancel_handler(std::function<void()> p_handler);
    void clear_cancel_handler();
    void set_release_handler(std::function<void(GDKPendingSignal *)> p_handler);
    void clear_release_handler();
};

} // namespace godot

#endif // GDK_PENDING_SIGNAL_H
