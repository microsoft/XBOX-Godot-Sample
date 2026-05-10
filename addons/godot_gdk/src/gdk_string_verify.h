#ifndef GDK_STRING_VERIFY_H
#define GDK_STRING_VERIFY_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <XUser.h>
#include <xsapi-c/services_c.h>

namespace godot {

class GDK;
class GDKResult;
class GDKRuntime;
class GDKUser;
class GDKXboxServices;

class GDKStringVerify : public RefCounted {
    GDCLASS(GDKStringVerify, RefCounted);

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;

    GDKRuntime *_get_runtime() const;
    GDKXboxServices *_get_xbox_services() const;
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant()) const;
    Ref<GDKResult> _ensure_ready_user(const Ref<GDKUser> &p_user) const;

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Signal verify_string_async(const Ref<GDKUser> &p_user, const String &p_text);
    Signal verify_strings_async(const Ref<GDKUser> &p_user, const PackedStringArray &p_strings);

    void on_user_removed(const Ref<GDKUser> &p_user);
};

} // namespace godot

#endif // GDK_STRING_VERIFY_H
