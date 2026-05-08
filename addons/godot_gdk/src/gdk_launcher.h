#ifndef GDK_LAUNCHER_H
#define GDK_LAUNCHER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XLauncher.h>

namespace godot {

class GDK;
class GDKResult;
class GDKUser;

class GDKLauncher : public RefCounted {
    GDCLASS(GDKLauncher, RefCounted);

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;

    static bool try_parse_uri_scheme_internal(const String &p_uri, String *r_scheme);
    static bool is_supported_scheme_internal(const String &p_scheme);
    static bool is_disallowed_scheme_internal(const String &p_scheme);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);
    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Ref<GDKResult> launch_uri(const String &p_uri, const Ref<GDKUser> &p_user = Ref<GDKUser>());
};

} // namespace godot

#endif // GDK_LAUNCHER_H
