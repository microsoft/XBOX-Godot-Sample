#ifndef XBOX_GAME_SAVE_H
#define XBOX_GAME_SAVE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class Xbox;
class XboxResult;
class XboxRuntime;
class XboxUser;
class XboxServices;

class XboxGameSave : public RefCounted {
    GDCLASS(XboxGameSave, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;

    XboxRuntime *_get_runtime() const;
    XboxServices *_get_xbox_services() const;
    Ref<XboxResult> _resolve_configuration_id(String *r_configuration_id) const;
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) const;

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();

    Signal get_folder_async(const Ref<XboxUser> &p_user);
    Signal get_remaining_quota_async(const Ref<XboxUser> &p_user);
};

} // namespace godot

#endif // XBOX_GAME_SAVE_H
