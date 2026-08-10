#ifndef XBOX_LAUNCHER_H
#define XBOX_LAUNCHER_H

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

class Xbox;
class XboxResult;
class XboxUser;

class XboxLauncher : public RefCounted {
    GDCLASS(XboxLauncher, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;

    static bool try_parse_uri_scheme_internal(const String &p_uri, String *r_scheme);
    static bool is_supported_scheme_internal(const String &p_scheme);
    static bool is_disallowed_scheme_internal(const String &p_scheme);

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);
    Ref<XboxResult> on_runtime_initialized();
    void shutdown();

    Ref<XboxResult> launch_uri(const String &p_uri, const Ref<XboxUser> &p_user = Ref<XboxUser>());
};

} // namespace godot

#endif // XBOX_LAUNCHER_H
