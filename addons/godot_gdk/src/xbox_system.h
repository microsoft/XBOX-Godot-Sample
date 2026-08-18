#ifndef XBOX_SYSTEM_H
#define XBOX_SYSTEM_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class Xbox;
class XboxResult;
class XboxServices;

class XboxSystem : public RefCounted {
    GDCLASS(XboxSystem, RefCounted);

    Xbox *m_owner = nullptr;

    XboxServices *_get_xbox_services() const;

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);

    Ref<XboxResult> get_title_id() const;
    Ref<XboxResult> get_title_id_hex() const;
    Ref<XboxResult> get_sandbox_id() const;
    Ref<XboxResult> get_service_configuration_id() const;
    bool is_xbox_services_initialized() const;
    bool is_feature_available(const String &p_feature_name) const;
};

} // namespace godot

#endif // XBOX_SYSTEM_H
