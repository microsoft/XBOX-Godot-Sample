#ifndef XBOX_H
#define XBOX_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class XboxAccessibility;
class XboxAchievements;
class XboxActivation;
class XboxCapture;
class XboxDisplay;
class XboxErrorReporting;
class XboxEvents;
class XboxGameChat;
class XboxGameSave;
class XboxGameUI;
class XboxLauncher;
class XboxLeaderboards;
class XboxMultiplayerActivity;
class XboxNetworking;
class XboxPackage;
class XboxPresence;
class XboxPrivacy;
class XboxProfile;
class XboxResult;
class XboxRuntime;
class XboxSocial;
class XboxSpeechSynthesizer;
class XboxStats;
class XboxStore;
class XboxStringVerify;
class XboxSystem;
class XboxTitleStorage;
class XboxUser;
class XboxUsers;
class XboxServices;

class Xbox : public Object {
    GDCLASS(Xbox, Object);

    static Xbox *singleton;

    XboxRuntime *m_runtime = nullptr;
    XboxServices *m_xbox_services = nullptr;
    Ref<XboxUsers> m_users;
    Ref<XboxGameUI> m_game_ui;
    Ref<XboxAccessibility> m_accessibility;
    Ref<XboxAchievements> m_achievements;
    Ref<XboxPackage> m_package;
    Ref<XboxStats> m_stats;
    Ref<XboxLeaderboards> m_leaderboards;
    Ref<XboxPrivacy> m_privacy;
    Ref<XboxPresence> m_presence;
    Ref<XboxSocial> m_social;
    Ref<XboxStore> m_store;
    Ref<XboxProfile> m_profile;
    Ref<XboxStringVerify> m_string_verify;
    Ref<XboxTitleStorage> m_title_storage;
    Ref<XboxErrorReporting> m_error_reporting;
    Ref<XboxLauncher> m_launcher;
    Ref<XboxMultiplayerActivity> m_multiplayer_activity;
    Ref<XboxCapture> m_capture;
    Ref<XboxSystem> m_system;
    Ref<XboxDisplay> m_display;
    Ref<XboxActivation> m_activation;
    Ref<XboxSpeechSynthesizer> m_speech;
    Ref<XboxEvents> m_events;
    Ref<XboxGameSave> m_game_save;
    Ref<XboxGameChat> m_game_chat;
    Ref<XboxNetworking> m_networking;

protected:
    static void _bind_methods();

public:
    static Xbox *get_singleton();

    Xbox();
    ~Xbox();

    Ref<XboxResult> initialize(const Variant &p_config = Variant());
    void shutdown();
    bool is_available() const;
    bool is_initialized() const;
    int64_t dispatch();
    Ref<XboxUsers> get_users() const;
    Ref<XboxGameUI> get_game_ui() const;
    Ref<XboxAccessibility> get_accessibility() const;
    Ref<XboxAchievements> get_achievements() const;
    Ref<XboxPackage> get_package() const;
    Ref<XboxStats> get_stats() const;
    Ref<XboxLeaderboards> get_leaderboards() const;
    Ref<XboxPrivacy> get_privacy() const;
    Ref<XboxPresence> get_presence() const;
    Ref<XboxSocial> get_social() const;
    Ref<XboxStore> get_store() const;
    Ref<XboxProfile> get_profile() const;
    Ref<XboxStringVerify> get_string_verify() const;
    Ref<XboxTitleStorage> get_title_storage() const;
    Ref<XboxErrorReporting> get_error_reporting() const;
    Ref<XboxLauncher> get_launcher() const;
    Ref<XboxMultiplayerActivity> get_multiplayer_activity() const;
    Ref<XboxCapture> get_capture() const;
    Ref<XboxSystem> get_system() const;
    Ref<XboxDisplay> get_display() const;
    Ref<XboxActivation> get_activation() const;
    Ref<XboxSpeechSynthesizer> get_speech() const;
    Ref<XboxEvents> get_events() const;
    Ref<XboxGameSave> get_game_save() const;
    Ref<XboxGameChat> get_game_chat() const;
    Ref<XboxNetworking> get_networking() const;

    XboxRuntime *get_runtime() const;
    XboxServices *get_xbox_services() const;
    void emit_runtime_error(const Ref<XboxResult> &p_result);
    void notify_user_removed(const Ref<XboxUser> &p_user);
};

} // namespace godot

#endif // XBOX_H
