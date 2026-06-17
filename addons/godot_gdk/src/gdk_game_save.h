#ifndef GDK_GAME_SAVE_H
#define GDK_GAME_SAVE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class GDK;
class GDKResult;
class GDKRuntime;
class GDKUser;
class GDKXboxServices;

class GDKGameSave : public RefCounted {
    GDCLASS(GDKGameSave, RefCounted);

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;

    GDKRuntime *_get_runtime() const;
    GDKXboxServices *_get_xbox_services() const;
    Ref<GDKResult> _resolve_configuration_id(String *r_configuration_id) const;
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) const;

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Signal get_folder_async(const Ref<GDKUser> &p_user);
    Ref<GDKResult> get_remaining_quota(const Ref<GDKUser> &p_user);
};

} // namespace godot

#endif // GDK_GAME_SAVE_H
