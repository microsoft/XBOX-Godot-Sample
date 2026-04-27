#ifndef GDK_CORE_H
#define GDK_CORE_H

// Windows headers must come before GDK headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

// GDK headers
#include <XGameRuntimeInit.h>
#include <XTaskQueue.h>

namespace godot {

class GDKCore : public Object {
    GDCLASS(GDKCore, Object);

    static GDKCore *singleton;

    bool m_initialized = false;
    XTaskQueueHandle m_task_queue = nullptr;

protected:
    static void _bind_methods();

public:
    static GDKCore *get_singleton();

    GDKCore();
    ~GDKCore();

    // Lifecycle
    Error initialize();
    void shutdown();
    bool is_initialized() const;

    // Must be called each frame to dispatch async callbacks
    void tick();

    // Info
    String get_version() const;

    // Accessors for other modules
    XTaskQueueHandle get_task_queue() const { return m_task_queue; }
};

} // namespace godot

#endif // GDK_CORE_H
