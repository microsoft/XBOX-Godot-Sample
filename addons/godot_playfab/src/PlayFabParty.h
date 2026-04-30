#ifndef PLAYFAB_PARTY_H
#define PLAYFAB_PARTY_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "pch.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class PlayFabParty : public Object {
    GDCLASS(PlayFabParty, Object);

    static PlayFabParty *singleton;

    Party::PartyManager& m_party_manager = Party::PartyManager::GetSingleton();
    bool m_initialized = false;
    String m_title_id;

protected:
    static void _bind_methods();

public:
    static PlayFabParty *get_singleton();

    PlayFabParty();
    ~PlayFabParty();

    int initialize(const String &p_title_id);
    int create_local_user();
    void shutdown();
    bool is_initialized() const;
};

} // namespace godot

#endif // PLAYFAB_PARTY_H
