#ifndef GODOT_PLAYFAB_USER_H
#define GODOT_PLAYFAB_USER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XUser.h>
#include <playfab/core/PFEntity.h>
#include <playfab/core/PFLocalUser.h>

namespace godot {

class PlayFabUser : public RefCounted {
    GDCLASS(PlayFabUser, RefCounted);

    XUserLocalId m_local_id = {};
    String m_entity_id;
    String m_entity_type;
    PFEntityHandle m_entity_handle = nullptr;
    PFLocalUserHandle m_local_user_handle = nullptr;

    HRESULT populate_from_user_handle(XUserHandle p_user_handle);
    HRESULT populate_from_entity_handle(PFEntityHandle p_entity_handle);

protected:
    static void _bind_methods();

public:
    PlayFabUser();
    ~PlayFabUser();

    uint64_t get_local_id() const;
    Dictionary get_entity_key() const;

    HRESULT adopt_session(XUserHandle p_user_handle, PFEntityHandle p_entity_handle, PFServiceConfigHandle p_service_config_handle);
    bool matches_local_id(XUserLocalId p_local_id) const;
    PFEntityHandle get_entity_handle() const;
    HRESULT duplicate_local_user_handle(PFLocalUserHandle *r_local_user_handle) const;
    void clear();
};

} // namespace godot

#endif // GODOT_PLAYFAB_USER_H
