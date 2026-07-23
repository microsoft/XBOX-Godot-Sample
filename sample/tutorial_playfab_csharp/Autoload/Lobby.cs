using Godot;
using System;
using System.Threading.Tasks;
using GodotPlayFab;
using GodotPlayFab.Types;

/// <summary>PlayFab-only lobby state machine used by the P3/P4 tutorial scenes.</summary>
public partial class Lobby : Node
{
    public enum State { Uninitialized, Ready, Hosting, Joining, InLobby, Leaving }

    public event Action<State> StateChanged;
    public event Action<PlayFabLobby> LobbyJoined;
    public event Action LobbyLeft;
    public event Action LobbyDisconnected;
    public event Action<string, string> InviteReceived;

    private State _state = State.Uninitialized;
    private PlayFabLobby _lobby;
    private PlayFabAuth _auth;
    private bool _pfMultiplayerSignalsConnected;

    public PlayFabLobby CurrentLobby => _state == State.InLobby ? _lobby : null;

    public override void _Ready()
    {
        _auth = GetNodeOrNull<PlayFabAuth>("/root/PlayFabAuth");
        if (_auth == null)
        {
            GD.PushError("[Lobby] PlayFabAuth autoload missing");
            return;
        }
        _ = EnsureReadyAsync();
    }

    public State GetState() => _state;
    public bool IsReady() => _state == State.Ready;
    public bool IsInLobby() => _state == State.InLobby;
    public bool IsBusy() => _state == State.Hosting || _state == State.Joining || _state == State.Leaving;
    public PlayFabLobby GetCurrentLobby() => CurrentLobby;

    private void SetState(State next)
    {
        if (_state == next) return;
        _state = next;
        StateChanged?.Invoke(_state);
    }

    private async Task<bool> EnsureReadyAsync()
    {
        if (_state == State.Ready || _state == State.InLobby) return true;
        if (IsBusy()) return false;
        if (!await _auth.SignInAsync())
        {
            GD.PushWarning($"[Lobby] sign-in failed ({_auth.GetLastErrorStage()}) — staying UNINITIALIZED");
            return false;
        }
        if (_state != State.Uninitialized) return _state == State.Ready || _state == State.InLobby;
        SetState(State.Ready);
        return true;
    }

    private async Task<bool> EnsureMultiplayerInitializedAsync()
    {
        if (!PlayFab.IsAvailable)
        {
            GD.PushError("[Lobby] PlayFab extension not loaded");
            return false;
        }

        if (!PlayFab.Multiplayer.IsInitialized())
        {
            PlayFabResult init = await PlayFab.Multiplayer.InitializeAsync();
            if (!init.Ok)
            {
                GD.PushWarning($"[Lobby] PlayFab.multiplayer init failed: {init.Message}");
                return false;
            }
            GD.Print("[Lobby] PlayFab.multiplayer initialized lazily");
        }

        if (!_pfMultiplayerSignalsConnected)
        {
            PlayFab.Multiplayer.InviteReceived += OnInviteReceived;
            PlayFab.Multiplayer.MultiplayerError += result => GD.PushWarning($"[Lobby] multiplayer error: {result.Message} ({result.Code})");
            _pfMultiplayerSignalsConnected = true;
        }
        return true;
    }

    public async Task<bool> HostLobbyAsync()
    {
        if (!await EnsureReadyAsync()) return false;
        if (_state == State.InLobby)
        {
            GD.PushWarning("[Lobby] host_lobby rejected — already in a lobby; leave first");
            return false;
        }
        if (_state != State.Ready)
        {
            GD.PushWarning($"[Lobby] host_lobby rejected — busy (state={(int)_state})");
            return false;
        }

        SetState(State.Hosting);
        if (!await EnsureMultiplayerInitializedAsync())
        {
            SetState(State.Ready);
            return false;
        }

        PlayFabUser user = _auth.PlayFabUser;
        var search = new Godot.Collections.Dictionary { ["string_key1"] = "casual" };
        var lobbyProps = new Godot.Collections.Dictionary { ["map"] = "harbor", ["mode"] = "deathmatch" };
        var memberProps = new Godot.Collections.Dictionary { ["loadout"] = "rifle" };
        PlayFabLobbyConfig config = TutorialSupport.LobbyConfig(
            4,
            PlayFabLobbyConfig.ACCESSPOLICYPUBLIC,
            PlayFabLobbyConfig.OWNERMIGRATIONAUTOMATIC,
            search,
            lobbyProps,
            memberProps);

        PlayFabResult result = await PlayFab.Multiplayer.CreateLobbyAsync(user, config);
        if (!result.Ok)
        {
            GD.PushWarning($"[Lobby] create_lobby failed: {result.Message} ({result.Code})");
            SetState(State.Ready);
            return false;
        }

        AttachLobby(result.DataAs<PlayFabLobby>());
        SetState(State.InLobby);
        GD.Print($"[Lobby] Lobby created: id={_lobby.LobbyId} max={_lobby.MaxMemberCount}");
        GD.Print("[Lobby] connection string ready — copy to second client");
        GD.Print(_lobby.ConnectionString);
        LobbyJoined?.Invoke(_lobby);
        return true;
    }

    public Task<bool> JoinLobbyWithStringAsync(string connectionString) => JoinLobbyAsync(connectionString);

    public async Task<bool> JoinLobbyAsync(string connectionString)
    {
        if (!await EnsureReadyAsync()) return false;
        if (_state == State.InLobby)
        {
            GD.PushWarning("[Lobby] join_lobby rejected — already in a lobby; leave first");
            return false;
        }
        if (_state != State.Ready)
        {
            GD.PushWarning($"[Lobby] join_lobby rejected — busy (state={(int)_state})");
            return false;
        }

        SetState(State.Joining);
        if (!await EnsureMultiplayerInitializedAsync())
        {
            SetState(State.Ready);
            return false;
        }

        PlayFabLobbyJoinConfig config = TutorialSupport.LobbyJoinConfig(new Godot.Collections.Dictionary { ["loadout"] = "shotgun" });
        PlayFabResult result = await PlayFab.Multiplayer.JoinLobbyAsync(_auth.PlayFabUser, connectionString, config);
        if (!result.Ok)
        {
            GD.PushWarning($"[Lobby] join_lobby failed: {result.Message} ({result.Code})");
            SetState(State.Ready);
            return false;
        }

        AttachLobby(result.DataAs<PlayFabLobby>());
        SetState(State.InLobby);
        GD.Print($"[Lobby] Joined lobby id={_lobby.LobbyId} with {_lobby.MemberCount} member(s)");
        LobbyJoined?.Invoke(_lobby);
        return true;
    }

    public async Task<Godot.Collections.Array> FindLobbiesAsync()
    {
        if (!await EnsureReadyAsync()) return new Godot.Collections.Array();
        if (!await EnsureMultiplayerInitializedAsync()) return new Godot.Collections.Array();

        PlayFabLobbySearchConfig search = TutorialSupport.LobbySearchConfig("string_key1 eq 'casual'");
        PlayFabResult result = await PlayFab.Multiplayer.FindLobbiesAsync(_auth.PlayFabUser, search);
        if (!result.Ok)
        {
            GD.PushWarning($"[Lobby] find_lobbies failed: {result.Message} ({result.Code})");
            return new Godot.Collections.Array();
        }
        return result.Data.AsGodotArray();
    }

    public async Task PushLoadoutChangeAsync(string loadout)
    {
        if (!IsInLobby()) return;
        PlayFabResult pf = await _lobby.SetMemberPropertiesAsync(new Godot.Collections.Dictionary { ["loadout"] = loadout });
        if (!pf.Ok) GD.PushWarning($"[Lobby] member props failed: {pf.Message}");
    }

    public async Task ChangeMapAsync(string newMap)
    {
        if (!IsInLobby()) return;
        PlayFabUser pfUser = _auth.PlayFabUser;
        if (!_lobby.IsOwner(pfUser)) return;
        PlayFabResult pf = await _lobby.SetPropertiesAsync(new Godot.Collections.Dictionary { ["map"] = newMap });
        if (!pf.Ok) GD.PushWarning($"[Lobby] lobby props failed: {pf.Message}");
    }

    public async Task<bool> LeaveLobbyAsync()
    {
        if (_state != State.InLobby)
        {
            if (IsBusy()) GD.PushWarning($"[Lobby] leave_lobby rejected — busy (state={(int)_state})");
            return false;
        }

        SetState(State.Leaving);
        PlayFabResult pf = await _lobby.LeaveAsync();
        if (pf.Ok) GD.Print("[Lobby] left lobby"); else GD.PushWarning($"[Lobby] leave failed: {pf.Message}");
        DetachLobby();
        SetState(State.Ready);
        LobbyLeft?.Invoke();
        return true;
    }

    private void AttachLobby(PlayFabLobby lobby)
    {
        DetachLobby();
        _lobby = lobby;
        if (_lobby != null) _lobby.StateChanged += OnLobbyStateChanged;
    }

    private void DetachLobby()
    {
        if (_lobby != null) _lobby.StateChanged -= OnLobbyStateChanged;
        _lobby = null;
    }

    private void OnInviteReceived(PlayFabLobbyInvite invite)
    {
        GD.Print($"[Lobby] invite from {invite?.SenderUserId}: {invite?.ConnectionString}");
        InviteReceived?.Invoke(invite?.ConnectionString ?? string.Empty, invite?.SenderUserId ?? string.Empty);
    }

    private void OnLobbyStateChanged(PlayFabLobbyStateChange change)
    {
        switch (change.Kind)
        {
            case PlayFabLobby.MEMBERADDED:
                GD.Print($"[Lobby] member added: {change.Member?.UserId} (local={change.Member?.IsLocal})");
                break;
            case PlayFabLobby.MEMBERREMOVED:
                GD.Print($"[Lobby] member removed: {change.Member?.UserId}");
                break;
            case PlayFabLobby.MEMBERUPDATED:
                GD.Print($"[Lobby] member updated: {change.Member?.UserId}");
                break;
            case PlayFabLobby.PROPERTIESUPDATED:
                GD.Print($"[Lobby] lobby properties: {change.Properties}");
                break;
            case PlayFabLobby.OWNERCHANGED:
                GD.Print($"[Lobby] owner changed: {change.Lobby?.OwnerEntityKey}");
                break;
            case PlayFabLobby.DISCONNECTED:
                _ = HandleDisconnectedAsync();
                break;
        }
    }

    private async Task HandleDisconnectedAsync()
    {
        await Task.Yield();
        if (_state != State.InLobby)
        {
            GD.PushWarning($"[Lobby] DISCONNECTED received in state={(int)_state} — ignoring");
            return;
        }
        if (!IsInsideTree()) return;
        GD.PushWarning("[Lobby] disconnected from lobby");
        DetachLobby();
        LobbyDisconnected?.Invoke();
        SetState(State.Ready);
    }
}

