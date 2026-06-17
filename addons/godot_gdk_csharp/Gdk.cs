using System;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Services;

namespace GodotGdk;

/// <summary>
/// Static entry point to the Microsoft GDK runtime, mirroring the native
/// <c>GDK</c> engine singleton. Resolves the singleton lazily so the facade can
/// be referenced before the GDExtension finishes loading.
/// </summary>
public static class Gdk
{
    private static GodotObject _singleton;
    private static bool _signalsConnected;

    /// <summary>True when the <c>godot_gdk</c> GDExtension is loaded.</summary>
    public static bool IsAvailable => Engine.HasSingleton("GDK");

    internal static GodotObject Singleton
    {
        get
        {
            if (_singleton != null && GodotObject.IsInstanceValid(_singleton))
            {
                return _singleton;
            }

            _singleton = Engine.HasSingleton("GDK") ? Engine.GetSingleton("GDK") : null;
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

    public static GdkResult Initialize() => GdkResult.From(Require().Call("initialize").AsGodotObject());

    public static GdkResult Initialize(Variant config) =>
        GdkResult.From(Require().Call("initialize", config).AsGodotObject());

    public static void Shutdown() => Singleton?.Call("shutdown");

    public static void Dispatch() => Singleton?.Call("dispatch");

    // --- Root signals (connected once, on first singleton resolution) ---
    public static event Action Initialized;
    public static event Action ShutdownCompleted;
    public static event Action<GdkResult> RuntimeError;

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
            Callable.From((GodotObject r) => RuntimeError?.Invoke(GdkResult.From(r))));
    }

    // --- Service namespaces (lazily wrapped, cached) ---
    private static GdkUsers _users;
    public static GdkUsers Users => _users ??= new GdkUsers(Service("users"));

    private static GdkGameUi _gameUi;
    public static GdkGameUi GameUi => _gameUi ??= new GdkGameUi(Service("game_ui"));

    private static GdkAchievements _achievements;
    public static GdkAchievements Achievements => _achievements ??= new GdkAchievements(Service("achievements"));

    private static GdkPackage _package;
    public static GdkPackage Package => _package ??= new GdkPackage(Service("package"));

    private static GdkStats _stats;
    public static GdkStats Stats => _stats ??= new GdkStats(Service("stats"));

    private static GdkLeaderboards _leaderboards;
    public static GdkLeaderboards Leaderboards => _leaderboards ??= new GdkLeaderboards(Service("leaderboards"));

    private static GdkPrivacy _privacy;
    public static GdkPrivacy Privacy => _privacy ??= new GdkPrivacy(Service("privacy"));

    private static GdkAccessibility _accessibility;
    public static GdkAccessibility Accessibility => _accessibility ??= new GdkAccessibility(Service("accessibility"));

    private static GdkPresence _presence;
    public static GdkPresence Presence => _presence ??= new GdkPresence(Service("presence"));

    private static GdkSocial _social;
    public static GdkSocial Social => _social ??= new GdkSocial(Service("social"));

    private static GdkStore _store;
    public static GdkStore Store => _store ??= new GdkStore(Service("store"));

    private static GdkProfile _profile;
    public static GdkProfile Profile => _profile ??= new GdkProfile(Service("profile"));

    private static GdkStringVerify _stringVerify;
    public static GdkStringVerify StringVerify => _stringVerify ??= new GdkStringVerify(Service("string_verify"));

    private static GdkTitleStorage _titleStorage;
    public static GdkTitleStorage TitleStorage => _titleStorage ??= new GdkTitleStorage(Service("title_storage"));

    private static GdkErrorReporting _errorReporting;
    public static GdkErrorReporting ErrorReporting => _errorReporting ??= new GdkErrorReporting(Service("error_reporting"));

    private static GdkLauncher _launcher;
    public static GdkLauncher Launcher => _launcher ??= new GdkLauncher(Service("launcher"));

    private static GdkMultiplayerActivity _multiplayerActivity;
    public static GdkMultiplayerActivity MultiplayerActivity =>
        _multiplayerActivity ??= new GdkMultiplayerActivity(Service("multiplayer_activity"));

    private static GdkCapture _capture;
    public static GdkCapture Capture => _capture ??= new GdkCapture(Service("capture"));

    private static GdkSystem _system;
    public static GdkSystem System => _system ??= new GdkSystem(Service("system"));

    private static GdkDisplay _display;
    public static GdkDisplay Display => _display ??= new GdkDisplay(Service("display"));

    private static GdkActivation _activation;
    public static GdkActivation Activation => _activation ??= new GdkActivation(Service("activation"));
}
