#ifndef GDK_GAME_UI_H
#define GDK_GAME_UI_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class GDK;
class GDKResult;
class GDKRuntime;
class GDKUser;
class GDKUsers;

class GDKGameUI : public RefCounted {
    GDCLASS(GDKGameUI, RefCounted);

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Signal show_message_dialog_async(
            const String &p_title,
            const String &p_message,
            const String &p_first_button = "OK",
            const String &p_second_button = String(),
            const String &p_third_button = String(),
            const String &p_default_button = "first",
            const String &p_cancel_button = "first");
    Ref<GDKResult> set_notification_position_hint(const String &p_position);
    Signal show_player_profile_card_async(const Ref<GDKUser> &p_requesting_user, const String &p_target_xuid);
    Signal show_player_picker_async(
            const Ref<GDKUser> &p_requesting_user,
            const String &p_prompt,
            const PackedStringArray &p_selectable_xuids,
            const PackedStringArray &p_preselected_xuids = PackedStringArray(),
            int64_t p_min_selection_count = 1,
            int64_t p_max_selection_count = 1);
    Signal resolve_privilege_with_ui_async(const Ref<GDKUser> &p_user, int64_t p_privilege);

    GDKRuntime *get_runtime_internal() const;
    GDKUsers *get_users_internal() const;
    Signal make_completed_signal_internal(const Ref<GDKResult> &p_result) const;
    Signal make_error_signal_internal(
            HRESULT p_hresult,
            const String &p_code,
            const String &p_message) const;
};

} // namespace godot

#endif // GDK_GAME_UI_H
