#ifndef GDK_SYSTEM_H
#define GDK_SYSTEM_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

class GDK;
class GDKResult;
class GDKXboxServices;

class GDKSystem : public RefCounted {
    GDCLASS(GDKSystem, RefCounted);

    GDK *m_owner = nullptr;

    GDKXboxServices *_get_xbox_services() const;

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> get_title_id() const;
    Ref<GDKResult> get_title_id_hex() const;
    Ref<GDKResult> get_sandbox_id() const;
    Ref<GDKResult> get_service_configuration_id() const;
    bool is_xbox_services_initialized() const;
};

} // namespace godot

#endif // GDK_SYSTEM_H
