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
#include "gdk_capture.h"
#include "gdk_error_reporting.h"
#include "gdk_game_ui.h"
#include "gdk_launcher.h"
#include "gdk_accessibility.h"
#include "gdk_leaderboards.h"
#include "gdk_multiplayer_activity.h"
#include "gdk_package.h"
#include "gdk_presence.h"
#include "gdk_profile.h"
#include "gdk_privacy.h"
#include "gdk_social.h"
#include "gdk_stats.h"
#include "gdk_string_verify.h"
#include "gdk_system.h"
#include "gdk_title_storage.h"
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
    Ref<GDKGameUI> m_game_ui;
    Ref<GDKAccessibility> m_accessibility;
    Ref<GDKAchievements> m_achievements;
    Ref<GDKPackage> m_package;
    Ref<GDKStats> m_stats;
    Ref<GDKLeaderboards> m_leaderboards;
    Ref<GDKPrivacy> m_privacy;
    Ref<GDKPresence> m_presence;
    Ref<GDKSocial> m_social;
    Ref<GDKProfile> m_profile;
    Ref<GDKStringVerify> m_string_verify;
    Ref<GDKTitleStorage> m_title_storage;
    Ref<GDKErrorReporting> m_error_reporting;
    Ref<GDKLauncher> m_launcher;
    Ref<GDKMultiplayerActivity> m_multiplayer_activity;
    Ref<GDKCapture> m_capture;
    Ref<GDKSystem> m_system;

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
    Ref<GDKGameUI> get_game_ui() const;
    Ref<GDKAccessibility> get_accessibility() const;
    Ref<GDKAchievements> get_achievements() const;
    Ref<GDKPackage> get_package() const;
    Ref<GDKStats> get_stats() const;
    Ref<GDKLeaderboards> get_leaderboards() const;
    Ref<GDKPrivacy> get_privacy() const;
    Ref<GDKPresence> get_presence() const;
    Ref<GDKSocial> get_social() const;
    Ref<GDKProfile> get_profile() const;
    Ref<GDKStringVerify> get_string_verify() const;
    Ref<GDKTitleStorage> get_title_storage() const;
    Ref<GDKErrorReporting> get_error_reporting() const;
    Ref<GDKLauncher> get_launcher() const;
    Ref<GDKMultiplayerActivity> get_multiplayer_activity() const;
    Ref<GDKCapture> get_capture() const;
    Ref<GDKSystem> get_system() const;

    GDKRuntime *get_runtime() const;
    GDKXboxServices *get_xbox_services() const;
    void emit_runtime_error(const Ref<GDKResult> &p_result);
    void notify_user_removed(const Ref<GDKUser> &p_user);
};

} // namespace godot

#endif // GDK_H
