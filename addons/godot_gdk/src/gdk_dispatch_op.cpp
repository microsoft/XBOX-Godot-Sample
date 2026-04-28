#include "gdk_dispatch_op.h"

#include "gdk_result.h"

namespace godot {

void GDKDispatchOp::_bind_methods() {}

bool GDKDispatchOp::cancel() {
    if (!request_cancel()) {
        return false;
    }

    invoke_cancel_handler();
    complete(GDKResult::cancelled(m_cancel_message));
    return true;
}

void GDKDispatchOp::set_cancel_message(const String &p_message) {
    if (!p_message.is_empty()) {
        m_cancel_message = p_message;
    }
}

} // namespace godot
