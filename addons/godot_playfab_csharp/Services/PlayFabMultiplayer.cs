using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabMultiplayer : PlayFabServiceBase
{
    internal PlayFabMultiplayer(GodotObject o) : base(o)
    {
        _o.Connect("state_changed", Callable.From((Variant a0) =>
            StateChanged?.Invoke(PlayFabMultiplayerStateChange.From(a0.AsGodotObject()))));
        _o.Connect("invite_received", Callable.From((Variant a0) =>
            InviteReceived?.Invoke(PlayFabLobbyInvite.From(a0.AsGodotObject()))));
        _o.Connect("multiplayer_error", Callable.From((Variant a0) =>
            MultiplayerError?.Invoke(PlayFabResult.From(a0.AsGodotObject()))));
    }

    public event Action<PlayFabMultiplayerStateChange> StateChanged;
    public event Action<PlayFabLobbyInvite> InviteReceived;
    public event Action<PlayFabResult> MultiplayerError;

    public Task<PlayFabResult> InitializeAsync(PlayFabMultiplayerConfig config = null) =>
        CallResultAsync("initialize_async", config?.Raw);

    public Task<PlayFabResult> ShutdownAsync() =>
        CallResultAsync("shutdown_async");

    public bool IsInitialized() =>
        Call("is_initialized").AsBool();

    public Task<PlayFabResult> CreateLobbyAsync(PlayFabUser user, PlayFabLobbyConfig config = null) =>
        CallResultAsync("create_lobby_async", user?.Raw, config?.Raw);

    public Task<PlayFabResult> JoinLobbyAsync(PlayFabUser user, string connection_string, PlayFabLobbyJoinConfig config = null) =>
        CallResultAsync("join_lobby_async", user?.Raw, connection_string, config?.Raw);

    public Task<PlayFabResult> JoinArrangedLobbyAsync(PlayFabUser user, string connection_string, PlayFabLobbyJoinConfig config = null) =>
        CallResultAsync("join_arranged_lobby_async", user?.Raw, connection_string, config?.Raw);

    public Task<PlayFabResult> FindLobbiesAsync(PlayFabUser user, PlayFabLobbySearchConfig search = null) =>
        CallResultAsync("find_lobbies_async", user?.Raw, search?.Raw);

    public Task<PlayFabResult> CreateMatchTicketAsync(PlayFabUser user, PlayFabMatchmakingTicketConfig config) =>
        CallResultAsync("create_match_ticket_async", user?.Raw, config?.Raw);

    public Godot.Collections.Array GetLobbies() =>
        Call("get_lobbies").AsGodotArray();

    public PlayFabLobby GetLobby(string lobby_id) =>
        PlayFabLobby.From(Call("get_lobby", lobby_id).AsGodotObject());

    public Godot.Collections.Array GetMatchTickets() =>
        Call("get_match_tickets").AsGodotArray();
}
