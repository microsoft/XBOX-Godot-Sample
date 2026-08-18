using System;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Services;

namespace GodotXbox;

/// <summary>
/// Static entry point to the Microsoft GDK runtime, mirroring the native
/// <c>GDK</c> engine singleton. Resolves the singleton lazily so the facade can
/// be referenced before the GDExtension finishes loading.
/// </summary>
public static class Xbox
{
    private static GodotObject _singleton;
    private static bool _signalsConnected;

    private const string SingletonNameSetting = "gdk/runtime/singleton_name";
    private const string DefaultSingletonName = "GDK";

    // Native class the singleton must be an instance of. The singleton *name*
    // is configurable; the class it resolves to is not.
    private const string SingletonClassName = "Xbox";

    /// <summary>
    /// Engine singleton name configured by <c>gdk/runtime/singleton_name</c>,
    /// falling back to <c>GDK</c>. Mirrors the resolution in the addon's
    /// <c>register_types.cpp</c>.
    /// </summary>
    public static string SingletonName
    {
        get
        {
            string configured = ProjectSettings
                .GetSetting(SingletonNameSetting, DefaultSingletonName)
                .AsString()
                .Trim();
            return string.IsNullOrEmpty(configured) ? DefaultSingletonName : configured;
        }
    }

    // Resolves the singleton by configured name, retrying under the default
    // name because the native side falls back to "GDK" when the configured name
    // is unusable (for example when it collides with an existing singleton).
    // Candidates are class-checked: a configured name that collides with an
    // unrelated engine singleton (say `Input`) still resolves through
    // Engine.HasSingleton, and returning it would report IsAvailable = true
    // while handing callers the wrong object.
    private static GodotObject ResolveSingleton()
    {
        string configured = SingletonName;
        if (Engine.HasSingleton(configured))
        {
            GodotObject candidate = Engine.GetSingleton(configured);
            if (candidate != null && candidate.IsClass(SingletonClassName))
            {
                return candidate;
            }
        }

        if (configured != DefaultSingletonName && Engine.HasSingleton(DefaultSingletonName))
        {
            GodotObject fallback = Engine.GetSingleton(DefaultSingletonName);
            if (fallback != null && fallback.IsClass(SingletonClassName))
            {
                return fallback;
            }
        }

        return null;
    }

    /// <summary>True when the <c>godot_gdk</c> GDExtension is loaded.</summary>
    public static bool IsAvailable => ResolveSingleton() != null;

    internal static GodotObject Singleton
    {
        get
        {
            if (_singleton != null && GodotObject.IsInstanceValid(_singleton))
            {
                return _singleton;
            }

            // Re-resolving: the previous singleton is gone (first access, an
            // extension reload, or an editor restart). Drop per-instance state
            // so root signals reconnect and cached service wrappers rebind to
            // the new native instance instead of returning stale ones.
            ResetSingletonState();
            _singleton = ResolveSingleton();
            EnsureSignalsConnected();
            return _singleton;
        }
    }

    private static GodotObject Require()
    {
        return Singleton
            ?? throw new InvalidOperationException(
                "GDK singleton is not registered. Is the godot_gdk GDExtension built and loaded?");
    }

    private static GodotObject Service(string member) => Require().Get(member).AsGodotObject();

    // --- Lifecycle ---
    public static bool IsInitialized => Singleton != null && Singleton.Call("is_initialized").AsBool();

    public static XboxResult Initialize() => XboxResult.From(Require().Call("initialize").AsGodotObject());

    public static XboxResult Initialize(Variant config) =>
        XboxResult.From(Require().Call("initialize", config).AsGodotObject());

    public static void Shutdown() => Singleton?.Call("shutdown");

    public static void Dispatch() => Singleton?.Call("dispatch");

    // --- Root signals (connected once, on first singleton resolution) ---
    public static event Action Initialized;
    public static event Action ShutdownCompleted;
    public static event Action<XboxResult> RuntimeError;

    private static void EnsureSignalsConnected()
    {
        if (_signalsConnected || _singleton == null)
        {
            return;
        }

        _signalsConnected = true;
        _singleton.Connect("initialized", Callable.From(() => Initialized?.Invoke()));
        _singleton.Connect("shutdown_completed", Callable.From(() => ShutdownCompleted?.Invoke()));
        _singleton.Connect("runtime_error",
            Callable.From((GodotObject r) => RuntimeError?.Invoke(XboxResult.From(r))));
    }

    // --- Service namespaces (lazily wrapped, cached) ---
    private static XboxUsers _users;
    public static XboxUsers Users => _users ??= new XboxUsers(Service("users"));

    private static XboxGameUi _gameUi;
    public static XboxGameUi GameUi => _gameUi ??= new XboxGameUi(Service("game_ui"));

    private static XboxAchievements _achievements;
    public static XboxAchievements Achievements => _achievements ??= new XboxAchievements(Service("achievements"));

    private static XboxPackage _package;
    public static XboxPackage Package => _package ??= new XboxPackage(Service("package"));

    private static XboxStats _stats;
    public static XboxStats Stats => _stats ??= new XboxStats(Service("stats"));

    private static XboxLeaderboards _leaderboards;
    public static XboxLeaderboards Leaderboards => _leaderboards ??= new XboxLeaderboards(Service("leaderboards"));

    private static XboxPrivacy _privacy;
    public static XboxPrivacy Privacy => _privacy ??= new XboxPrivacy(Service("privacy"));

    private static XboxAccessibility _accessibility;
    public static XboxAccessibility Accessibility => _accessibility ??= new XboxAccessibility(Service("accessibility"));

    private static XboxPresence _presence;
    public static XboxPresence Presence => _presence ??= new XboxPresence(Service("presence"));

    private static XboxSocial _social;
    public static XboxSocial Social => _social ??= new XboxSocial(Service("social"));

    private static XboxStore _store;
    public static XboxStore Store => _store ??= new XboxStore(Service("store"));

    private static XboxProfile _profile;
    public static XboxProfile Profile => _profile ??= new XboxProfile(Service("profile"));

    private static XboxStringVerify _stringVerify;
    public static XboxStringVerify StringVerify => _stringVerify ??= new XboxStringVerify(Service("string_verify"));

    private static XboxTitleStorage _titleStorage;
    public static XboxTitleStorage TitleStorage => _titleStorage ??= new XboxTitleStorage(Service("title_storage"));

    private static XboxErrorReporting _errorReporting;
    public static XboxErrorReporting ErrorReporting => _errorReporting ??= new XboxErrorReporting(Service("error_reporting"));

    private static XboxLauncher _launcher;
    public static XboxLauncher Launcher => _launcher ??= new XboxLauncher(Service("launcher"));

    private static XboxMultiplayerActivity _multiplayerActivity;
    public static XboxMultiplayerActivity MultiplayerActivity =>
        _multiplayerActivity ??= new XboxMultiplayerActivity(Service("multiplayer_activity"));

    private static XboxCapture _capture;
    public static XboxCapture Capture => _capture ??= new XboxCapture(Service("capture"));

    private static XboxSystem _system;
    public static XboxSystem System => _system ??= new XboxSystem(Service("system"));

    private static XboxDisplay _display;
    public static XboxDisplay Display => _display ??= new XboxDisplay(Service("display"));

    private static XboxActivation _activation;
    public static XboxActivation Activation => _activation ??= new XboxActivation(Service("activation"));

    // The following services are reached via getter methods on the native
    // singleton (no backing property member), so resolve them through Call.
    private static GodotObject ServiceCall(string getter) => Require().Call(getter).AsGodotObject();

    private static XboxGameChat _gameChat;
    public static XboxGameChat GameChat => _gameChat ??= new XboxGameChat(ServiceCall("get_game_chat"));

    private static XboxSpeechSynthesizer _speech;
    public static XboxSpeechSynthesizer Speech => _speech ??= new XboxSpeechSynthesizer(ServiceCall("get_speech"));

    private static XboxGameSave _gameSave;
    public static XboxGameSave GameSave => _gameSave ??= new XboxGameSave(ServiceCall("get_game_save"));

    private static XboxEvents _events;
    public static XboxEvents Events => _events ??= new XboxEvents(ServiceCall("get_events"));

    // Invalidate all singleton-bound cached state. Called from the Singleton
    // getter when the native instance is re-resolved. Keep this in sync with
    // the cached wrapper fields above.
    private static void ResetSingletonState()
    {
        _signalsConnected = false;
        _users = null;
        _gameUi = null;
        _achievements = null;
        _package = null;
        _stats = null;
        _leaderboards = null;
        _privacy = null;
        _accessibility = null;
        _presence = null;
        _social = null;
        _store = null;
        _profile = null;
        _stringVerify = null;
        _titleStorage = null;
        _errorReporting = null;
        _launcher = null;
        _multiplayerActivity = null;
        _capture = null;
        _system = null;
        _display = null;
        _activation = null;
        _gameChat = null;
        _speech = null;
        _gameSave = null;
        _events = null;
    }
}
