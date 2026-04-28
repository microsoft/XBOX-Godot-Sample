#ifndef GDK_DISPATCH_OP_H
#define GDK_DISPATCH_OP_H

#include <godot_cpp/variant/string.hpp>

#include "gdk_async_op.h"

namespace godot {

class GDKDispatchOp : public GDKAsyncOp {
    GDCLASS(GDKDispatchOp, GDKAsyncOp);

    String m_cancel_message = "Dispatch operation cancelled.";

protected:
    static void _bind_methods();

public:
    bool cancel() override;
    void set_cancel_message(const String &p_message);
};

} // namespace godot

#endif // GDK_DISPATCH_OP_H
