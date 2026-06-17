using System;
using Godot;
using GodotPlayFab.Services;

namespace GodotPlayFab;

/// <summary>
/// Static entry point to the PlayFab runtime, mirroring the native
/// <c>PlayFab</c> engine singleton. Resolves the singleton lazily so the facade
/// can be referenced before the GDExtension finishes loading.
/// </summary>
public static class PlayFab
{
    private static GodotObject _singleton;
    private static bool _signalsConnected;

    /// <summary>True when the <c>godot_playfab</c> GDExtension is loaded.</summary>
    public static bool IsAvailable => Engine.HasSingleton("PlayFab");

    internal static GodotObject Singleton
    {
        get
        {
            if (_singleton != null && GodotObject.IsInstanceValid(_singleton))
            {
                return _singleton;
            }

            _singleton = Engine.HasSingleton("PlayFab") ? Engine.GetSingleton("PlayFab") : null;
            EnsureSignalsConnected();
            return _singleton;
        }
    }

    private static GodotObject Require()
    {
        return Singleton
            ?? throw new InvalidOperationException(
                "PlayFab singleton is not registered. Is the godot_playfab GDExtension built and loaded?");
    }

    private static GodotObject Service(string member) => Require().Get(member).AsGodotObject();

    // --- Lifecycle ---
    public static bool IsInitialized => Singleton != null && Singleton.Call("is_initialized").AsBool();

    public static PlayFabResult Initialize() => PlayFabResult.From(Require().Call("initialize").AsGodotObject());

    public static void Shutdown() => Singleton?.Call("shutdown");

    public static int Dispatch() => Singleton?.Call("dispatch").AsInt32() ?? 0;

    public static string TitleId => Singleton?.Call("get_title_id").AsString() ?? string.Empty;

    public static string Endpoint => Singleton?.Call("get_endpoint").AsString() ?? string.Empty;

    // --- Root signals (connected once, on first singleton resolution) ---
    public static event Action Initialized;
    public static event Action ShutdownCompleted;

    private static void EnsureSignalsConnected()
    {
        if (_signalsConnected || _singleton == null)
        {
            return;
        }

        _signalsConnected = true;
        _singleton.Connect("initialized", Callable.From(() => Initialized?.Invoke()));
        _singleton.Connect("shutdown_completed", Callable.From(() => ShutdownCompleted?.Invoke()));
    }

    // --- Service namespaces (lazily wrapped, cached) ---
    private static PlayFabUsers _users;
    public static PlayFabUsers Users => _users ??= new PlayFabUsers(Service("users"));

    private static PlayFabGameSaves _gameSaves;
    public static PlayFabGameSaves GameSaves => _gameSaves ??= new PlayFabGameSaves(Service("game_saves"));

    private static PlayFabLeaderboards _leaderboards;
    public static PlayFabLeaderboards Leaderboards => _leaderboards ??= new PlayFabLeaderboards(Service("leaderboards"));

    private static PlayFabMultiplayer _multiplayer;
    public static PlayFabMultiplayer Multiplayer => _multiplayer ??= new PlayFabMultiplayer(Service("multiplayer"));

    private static PlayFabParty _party;
    public static PlayFabParty Party => _party ??= new PlayFabParty(Service("party"));

    private static PlayFabAccounts _accounts;
    public static PlayFabAccounts Accounts => _accounts ??= new PlayFabAccounts(Service("accounts"));

    private static PlayFabCatalog _catalog;
    public static PlayFabCatalog Catalog => _catalog ??= new PlayFabCatalog(Service("catalog"));

    private static PlayFabCloudScript _cloudScript;
    public static PlayFabCloudScript CloudScript => _cloudScript ??= new PlayFabCloudScript(Service("cloud_script"));

    private static PlayFabEntityData _entityData;
    public static PlayFabEntityData EntityData => _entityData ??= new PlayFabEntityData(Service("entity_data"));

    private static PlayFabEvents _events;
    public static PlayFabEvents Events => _events ??= new PlayFabEvents(Service("events"));

    private static PlayFabExperimentation _experimentation;
    public static PlayFabExperimentation Experimentation => _experimentation ??= new PlayFabExperimentation(Service("experimentation"));

    private static PlayFabFriends _friends;
    public static PlayFabFriends Friends => _friends ??= new PlayFabFriends(Service("friends"));

    private static PlayFabGroups _groups;
    public static PlayFabGroups Groups => _groups ??= new PlayFabGroups(Service("groups"));

    private static PlayFabInventory _inventory;
    public static PlayFabInventory Inventory => _inventory ??= new PlayFabInventory(Service("inventory"));

    private static PlayFabLocalization _localization;
    public static PlayFabLocalization Localization => _localization ??= new PlayFabLocalization(Service("localization"));

    private static PlayFabPlayerData _playerData;
    public static PlayFabPlayerData PlayerData => _playerData ??= new PlayFabPlayerData(Service("player_data"));

    private static PlayFabStatistics _statistics;
    public static PlayFabStatistics Statistics => _statistics ??= new PlayFabStatistics(Service("statistics"));

    private static PlayFabTitleData _titleData;
    public static PlayFabTitleData TitleData => _titleData ??= new PlayFabTitleData(Service("title_data"));
}
