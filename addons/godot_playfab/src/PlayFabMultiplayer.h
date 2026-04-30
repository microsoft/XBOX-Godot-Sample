#ifndef PLAYFAB_MULTIPLAYER_H
#define PLAYFAB_MULTIPLAYER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "pch.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class PlayFabMultiplayer : public Object {
    GDCLASS(PlayFabMultiplayer, Object);

    static PlayFabMultiplayer *singleton;

    bool m_initialized = false;
    PFMultiplayerHandle m_multiplayer_handle = nullptr;
    String m_title_id;

protected:
    static void _bind_methods();

public:
    static PlayFabMultiplayer *get_singleton();

    PlayFabMultiplayer();
    ~PlayFabMultiplayer();

    int initialize(const String &p_title_id);
    void shutdown();
    bool is_initialized() const;

    PFMultiplayerHandle get_multiplayer_handle() const;
};

} // namespace godot

#endif // PLAYFAB_MULTIPLAYER_H
