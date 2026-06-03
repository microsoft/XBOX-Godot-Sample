#ifndef GDK_H
#define GDK_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "gdk_achievement.h"
#include "gdk_multiplayer_activity.h"
#include "gdk_presence.h"
#include "gdk_social.h"
#include "gdk_user.h"

namespace godot {

class GDKResult;
class GDKRuntime;
class GDKXboxServices;

class GDK : public Object {
    GDCLASS(GDK, Object);

    static GDK *singleton;

    GDKRuntime *m_runtime = nullptr;
    GDKXboxServices *m_xbox_services = nullptr;
    Ref<GDKUsers> m_users;
    Ref<GDKAchievements> m_achievements;
    Ref<GDKPresence> m_presence;
    Ref<GDKSocial> m_social;
    Ref<GDKMultiplayerActivity> m_multiplayer_activity;

protected:
    static void _bind_methods();

public:
    static GDK *get_singleton();

    GDK();
    ~GDK();

    Ref<GDKResult> initialize(const Variant &p_config = Variant());
    void shutdown();
    bool is_available() const;
    bool is_initialized() const;
    int64_t dispatch();
    Ref<GDKResult> get_last_error() const;
    Ref<GDKUsers> get_users() const;
    Ref<GDKAchievements> get_achievements() const;
    Ref<GDKPresence> get_presence() const;
    Ref<GDKSocial> get_social() const;
    Ref<GDKMultiplayerActivity> get_multiplayer_activity() const;

    GDKRuntime *get_runtime() const;
    GDKXboxServices *get_xbox_services() const;
    void emit_runtime_error(const Ref<GDKResult> &p_result);
    void notify_user_removed(const Ref<GDKUser> &p_user);
};

} // namespace godot

#endif // GDK_H
