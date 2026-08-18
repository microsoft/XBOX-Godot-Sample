#ifndef XBOX_STRING_VERIFY_H
#define XBOX_STRING_VERIFY_H

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

class Xbox;
class XboxResult;
class XboxRuntime;
class XboxUser;
class XboxServices;

class XboxStringVerify : public RefCounted {
    GDCLASS(XboxStringVerify, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;

    XboxRuntime *_get_runtime() const;
    XboxServices *_get_xbox_services() const;
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant()) const;
    Ref<XboxResult> _ensure_ready_user(const Ref<XboxUser> &p_user) const;

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();

    Signal verify_string_async(const Ref<XboxUser> &p_user, const String &p_text);
    Signal verify_strings_async(const Ref<XboxUser> &p_user, const PackedStringArray &p_strings);

    void on_user_removed(const Ref<XboxUser> &p_user);
};

} // namespace godot

#endif // XBOX_STRING_VERIFY_H
