#ifndef GDK_PROFILE_H
#define GDK_PROFILE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
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

class GDKUserProfile : public RefCounted {
    GDCLASS(GDKUserProfile, RefCounted);

    String m_xuid;
    String m_app_display_name;
    String m_app_display_picture_resize_uri;
    String m_game_display_name;
    String m_game_display_picture_resize_uri;
    String m_gamerscore;
    String m_gamertag;
    String m_modern_gamertag;
    String m_modern_gamertag_suffix;
    String m_unique_modern_gamertag;

protected:
    static void _bind_methods();

public:
    String get_xuid() const;
    String get_app_display_name() const;
    String get_app_display_picture_resize_uri() const;
    String get_game_display_name() const;
    String get_game_display_picture_resize_uri() const;
    String get_gamerscore() const;
    String get_gamertag() const;
    String get_modern_gamertag() const;
    String get_modern_gamertag_suffix() const;
    String get_unique_modern_gamertag() const;

    void populate_from_native(const XblUserProfile &p_profile);
};

class GDKProfile : public RefCounted {
    GDCLASS(GDKProfile, RefCounted);

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

    Signal get_profile_async(const Ref<GDKUser> &p_user, const String &p_xuid);
    Signal get_profiles_async(const Ref<GDKUser> &p_user, const PackedStringArray &p_xuids);
    Signal get_profiles_for_social_group_async(const Ref<GDKUser> &p_user, const String &p_social_group);

    void on_user_removed(const Ref<GDKUser> &p_user);
};

} // namespace godot

#endif // GDK_PROFILE_H
