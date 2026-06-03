#ifndef GODOT_PLAYFAB_PENDING_SIGNAL_H
#define GODOT_PLAYFAB_PENDING_SIGNAL_H

#include <functional>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "playfab_result.h"

namespace godot {

class PlayFabPendingSignal : public RefCounted {
    GDCLASS(PlayFabPendingSignal, RefCounted);

    bool m_done = false;
    bool m_cancel_requested = false;
    bool m_deferred_completion_queued = false;
    Ref<PlayFabResult> m_result;
    Ref<PlayFabResult> m_deferred_result;
    Ref<PlayFabPendingSignal> m_self_ref;
    std::function<void()> m_cancel_handler;
    std::function<void(PlayFabPendingSignal *)> m_release_handler;

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
    void complete(const Ref<PlayFabResult> &p_result);
    void complete_deferred(const Ref<PlayFabResult> &p_result);

    void set_cancel_handler(std::function<void()> p_handler);
    void clear_cancel_handler();
    void set_release_handler(std::function<void(PlayFabPendingSignal *)> p_handler);
    void clear_release_handler();
};

} // namespace godot

#endif // GODOT_PLAYFAB_PENDING_SIGNAL_H
