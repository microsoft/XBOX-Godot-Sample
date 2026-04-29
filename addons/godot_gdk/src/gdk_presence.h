#ifndef GDK_PRESENCE_H
#define GDK_PRESENCE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XUser.h>
#include <xsapi-c/services_c.h>

namespace godot {

class GDK;
class GDKAsyncOp;
class GDKResult;
class GDKRuntime;
class GDKUser;
class GDKXboxServices;

class GDKPresenceRecord : public RefCounted {
    GDCLASS(GDKPresenceRecord, RefCounted);

public:
    enum UserState {
        USER_STATE_UNKNOWN = 0,
        USER_STATE_ONLINE,
        USER_STATE_AWAY,
        USER_STATE_OFFLINE,
    };

private:
    String m_xuid;
    UserState m_user_state = USER_STATE_UNKNOWN;
    Array m_title_records;

protected:
    static void _bind_methods();

public:
    String get_xuid() const;
    UserState get_user_state() const;
    String get_user_state_name() const;
    bool is_online() const;
    Array get_title_records() const;

    void set_data(const String &p_xuid, UserState p_user_state, const Array &p_title_records);
    HRESULT populate_from_presence_record(XblPresenceRecordHandle p_record_handle);
    void populate_from_social_manager_record(uint64_t p_xuid, const XblSocialManagerPresenceRecord &p_record);
};

class GDKPresence : public RefCounted {
    GDCLASS(GDKPresence, RefCounted);

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;
    std::vector<Ref<GDKPresenceRecord>> m_cached_presence;

    GDKRuntime *_get_runtime() const;
    GDKXboxServices *_get_xbox_services() const;
    Ref<GDKAsyncOp> _make_error_async_op(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant()) const;
    Ref<GDKUser> _get_presence_calling_user() const;
    Ref<GDKResult> _parse_query_xuids(const PackedStringArray &p_xuids, std::vector<uint64_t> *r_xuids) const;
    Ref<GDKPresenceRecord> _find_cached_presence(const String &p_xuid) const;
    void _remove_cached_presence(const String &p_xuid);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Ref<GDKAsyncOp> set_presence_async(const Ref<GDKUser> &p_user, const String &p_state, const Dictionary &p_rich_presence = Dictionary());
    Ref<GDKAsyncOp> clear_presence_async(const Ref<GDKUser> &p_user);
    Ref<GDKAsyncOp> get_presence_async(const PackedStringArray &p_xuids);
    Ref<GDKPresenceRecord> get_cached_presence(const String &p_xuid) const;

    void cache_presence_record(const Ref<GDKPresenceRecord> &p_record, bool p_emit_signal = true);
    void on_user_removed(const Ref<GDKUser> &p_user);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKPresenceRecord::UserState);

#endif // GDK_PRESENCE_H
