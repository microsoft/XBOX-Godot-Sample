#ifndef PLAYFAB_SERVICES_H
#define PLAYFAB_SERVICES_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "pch.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class PlayFabServices : public Object {
    GDCLASS(PlayFabServices, Object);

    static PlayFabServices *singleton;

    bool m_initialized = false;
    PFServiceConfigHandle m_service_config_handle = nullptr;
    EntityHandle m_entityHandle;
    String m_title_id;
    String m_endpoint;

protected:
    static void _bind_methods();

public:
    static PlayFabServices *get_singleton();

    PlayFabServices();
    ~PlayFabServices();

    int initialize(const String &p_title_id);
    int AuthenticationLoginWithCustomIDAsync(const String& p_custom_id);
    void shutdown();
    bool is_initialized() const;

    String get_title_id() const;
    String get_endpoint() const;
};

} // namespace godot

#endif // PLAYFAB_SERVICES_H
