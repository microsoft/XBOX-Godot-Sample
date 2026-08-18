#ifndef XBOX_EVENTS_H
#define XBOX_EVENTS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class Xbox;
class XboxResult;
class XboxRuntime;
class XboxUser;
class XboxServices;

class XboxEvents : public RefCounted {
    GDCLASS(XboxEvents, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;
    String m_play_session_id;

    XboxRuntime *_get_runtime() const;
    XboxServices *_get_xbox_services() const;
    void _ensure_play_session_id();

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();

    Ref<XboxResult> write_event(
            const Ref<XboxUser> &p_user,
            const String &p_event_name,
            const Dictionary &p_dimensions,
            const Dictionary &p_measurements);
    void set_play_session_id(const String &p_play_session_id);
    String get_play_session_id();
};

} // namespace godot

#endif // XBOX_EVENTS_H
