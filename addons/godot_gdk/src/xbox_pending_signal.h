#ifndef XBOX_PENDING_SIGNAL_H
#define XBOX_PENDING_SIGNAL_H

#include <functional>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "xbox_result.h"

namespace godot {

class XboxPendingSignal : public RefCounted {
    GDCLASS(XboxPendingSignal, RefCounted);

    bool m_done = false;
    bool m_cancel_requested = false;
    bool m_deferred_completion_queued = false;
    Ref<XboxResult> m_result;
    Ref<XboxResult> m_deferred_result;
    Ref<XboxPendingSignal> m_self_ref;
    std::function<void()> m_cancel_handler;
    std::function<void(XboxPendingSignal *)> m_release_handler;

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
    void complete(const Ref<XboxResult> &p_result);
    void complete_deferred(const Ref<XboxResult> &p_result);

    void set_cancel_handler(std::function<void()> p_handler);
    void clear_cancel_handler();
    void set_release_handler(std::function<void(XboxPendingSignal *)> p_handler);
    void clear_release_handler();
};

} // namespace godot

#endif // XBOX_PENDING_SIGNAL_H
