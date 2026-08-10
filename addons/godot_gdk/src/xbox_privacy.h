#ifndef XBOX_PRIVACY_H
#define XBOX_PRIVACY_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
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

class XboxPrivacy : public RefCounted {
    GDCLASS(XboxPrivacy, RefCounted);

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
    int dispatch();

    Signal check_permission_async(const Ref<XboxUser> &p_user, const String &p_permission, const String &p_target_xuid);
    Signal check_permission_for_anonymous_user_async(const Ref<XboxUser> &p_user, const String &p_permission, const String &p_anonymous_user_type);
    Signal batch_check_permission_async(const Ref<XboxUser> &p_user, const String &p_permission, const PackedStringArray &p_target_xuids);
    Signal get_avoid_list_async(const Ref<XboxUser> &p_user);
    Signal get_mute_list_async(const Ref<XboxUser> &p_user);

    void on_user_removed(const Ref<XboxUser> &p_user);
};

} // namespace godot

#endif // XBOX_PRIVACY_H
