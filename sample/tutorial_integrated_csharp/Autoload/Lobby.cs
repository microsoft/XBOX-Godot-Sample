using Godot;
using System;
using System.Threading.Tasks;
using GodotGdk;
using GodotGdk.Types;
using GodotPlayFab;
using GodotPlayFab.Types;

public partial class Lobby : Node
{
    public const int XuserPrivilegeMultiplayer = 254;
    private const string MpaJoinRestrictionFollowed = "followed";
    private const string PresenceInLobby = "in_lobby";

    public enum State { Uninitialized, Ready, Hosting, Joining, InLobby, Leaving }

    public event Action<State> StateChanged;
    public event Action<PlayFabLobby> LobbyJoined;
    public event Action LobbyLeft;
    public event Action LobbyDisconnected;
    public event Action<int, string> InvitePendingConfirmation;
    public event Action<int> InvitePendingCleared;

    private State _state = State.Uninitialized;
    private PlayFabLobby _lobby;
    private Auth _auth;
    private bool _gdkSignalsConnected;
    private bool _pfSignalsConnected;
    private string[] _watchedXuids = Array.Empty<string>();
    private GdkSocialGroup _friendsGroup;
    private bool _socialGraphStarted;
    private int _pendingInviteId;
    private string _pendingInviteConnectionString = string.Empty;
    private bool _pendingInviteConfirming;

    public PlayFabLobby CurrentLobby => _state == State.InLobby ? _lobby : null;

    public override void _Ready()
    {
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        if (_auth == null)
        {
            GD.PushError("[Lobby] Auth autoload missing");
            return;
        }
        _ = EnsureReadyAsync();
    }

    public override void _ExitTree()
    {
        if (Gdk.IsAvailable)
        {
            if (_friendsGroup != null)
            {
                Gdk.Social.DestroySocialGroup(_friendsGroup);
                _friendsGroup = null;
            }
            if (_socialGraphStarted && _auth?.XboxUser != null)
            {
                Gdk.Social.StopSocialGraph(_auth.XboxUser);
                _socialGraphStarted = false;
            }
        }
    }

    public State GetState() => _state;
    public bool IsReady() => _state == State.Ready;
    public bool IsInLobby() => _state == State.InLobby;
    public bool IsBusy() => _state == State.Hosting || _state == State.Joining || _state == State.Leaving;
    public PlayFabLobby GetCurrentLobby() => CurrentLobby;

    private void SetState(State next)
    {
        if (_state == next) return;
        bool wasInLobby = _state == State.InLobby;
        _state = next;
        StateChanged?.Invoke(_state);
        if (wasInLobby && _state != State.InLobby) ClearPendingInvite();
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
        if (!Gdk.IsAvailable)
        {
            GD.PushError("[Lobby] GDK extension not loaded");
            return false;
        }
        if (_state != State.Uninitialized) return _state == State.Ready || _state == State.InLobby;

        if (!_gdkSignalsConnected)
        {
            Gdk.MultiplayerActivity.PendingInviteReceived += OnPendingInviteReceived;
            Gdk.MultiplayerActivity.InviteAccepted += OnInviteAccepted;
            Gdk.MultiplayerActivity.ActivitiesUpdated += OnActivitiesUpdated;
            Gdk.Presence.DevicePresenceChanged += OnDevicePresenceChanged;
            Gdk.Presence.TitlePresenceChanged += OnTitlePresenceChanged;
            Gdk.Presence.PresenceChanged += OnPresenceChanged;
            _gdkSignalsConnected = true;
            GD.Print("[Lobby] GDK MPA + presence handlers connected. PlayFab Multiplayer init is lazy.");
        }
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
        if (!_pfSignalsConnected)
        {
            PlayFab.Multiplayer.StateChanged += _ => { };
            PlayFab.Multiplayer.InviteReceived += invite => GD.Print($"[Lobby] invite from {invite?.SenderEntityKey}: {invite?.ConnectionString}");
            PlayFab.Multiplayer.MultiplayerError += result => GD.PushWarning($"[Lobby] multiplayer error: {result.Message} ({result.Code})");
            _pfSignalsConnected = true;
        }
        return true;
    }

    public async Task<bool> CanUseMultiplayerAsync()
    {
        GdkUser user = _auth?.XboxUser;
        if (user == null) return false;
        GdkResult pf = await Gdk.Users.CheckPrivilegeAsync(user, XuserPrivilegeMultiplayer);
        Godot.Collections.Dictionary data = pf.Data.AsGodotDictionary();
        if (pf.Ok && TutorialSupport.DictBool(data, "has_privilege")) return true;
        GD.Print($"[Lobby] multiplayer denied ({TutorialSupport.DictString(data, "deny_reason")}) — resolving with UI");
        GdkResult resolved = await Gdk.Users.ResolvePrivilegeWithUiAsync(user, XuserPrivilegeMultiplayer);
        if (!resolved.Ok)
        {
            GD.PushWarning($"[Lobby] resolve_privilege_with_ui failed: {resolved.Message}");
            return false;
        }
        return TutorialSupport.DictBool(resolved.Data.AsGodotDictionary(), "has_privilege");
    }

    public async Task<string[]> FilterInvitableAsync(string[] xuids)
    {
        GdkUser user = _auth?.XboxUser;
        if (user == null || xuids == null || xuids.Length == 0) return Array.Empty<string>();
        GdkResult pf = await Gdk.Privacy.BatchCheckPermissionAsync(user, "play_multiplayer", xuids);
        if (!pf.Ok)
        {
            GD.PushWarning($"[Lobby] permission batch failed: {pf.Message}");
            return Array.Empty<string>();
        }
        Godot.Collections.Array result = pf.Data.AsGodotArray();
        var allowed = new System.Collections.Generic.List<string>();
        foreach (Variant entryValue in result)
        {
            Godot.Collections.Dictionary entry = entryValue.AsGodotDictionary();
            if (TutorialSupport.DictBool(entry, "allowed")) allowed.Add(TutorialSupport.DictString(entry, "target_xuid"));
        }
        return allowed.ToArray();
    }

    public async Task<Godot.Collections.Array> GetFriendsAsync()
    {
        GdkUser user = _auth?.XboxUser;
        if (user == null || !Gdk.IsAvailable) return new Godot.Collections.Array();
        if (!_socialGraphStarted)
        {
            GdkResult sg = Gdk.Social.StartSocialGraph(user);
            if (!sg.Ok)
            {
                GD.PushWarning($"[Lobby] start_social_graph failed: {sg.Message}");
                return new Godot.Collections.Array();
            }
            _socialGraphStarted = true;
        }
        if (_friendsGroup == null)
        {
            GdkResult f = await Gdk.Social.GetFriendsAsync(user);
            if (!f.Ok)
            {
                GD.PushWarning($"[Lobby] get_friends failed: {f.Message}");
                return new Godot.Collections.Array();
            }
            _friendsGroup = f.DataAs<GdkSocialGroup>();
        }
        GdkResult users = Gdk.Social.GetGroupUsers(_friendsGroup);
        if (!users.Ok)
        {
            GD.PushWarning($"[Lobby] get_group_users failed: {users.Message}");
            return new Godot.Collections.Array();
        }
        return users.Data.AsGodotArray();
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
        if (!await EnsureMultiplayerInitializedAsync() || !await CanUseMultiplayerAsync())
        {
            SetState(State.Ready);
            return false;
        }
        PlayFabUser user = _auth.PlayFabUser;
        var search = new Godot.Collections.Dictionary { ["string_key1"] = "casual" };
        var lobbyProps = new Godot.Collections.Dictionary { ["map"] = "harbor", ["mode"] = "deathmatch" };
        var memberProps = new Godot.Collections.Dictionary { ["loadout"] = "rifle", ["xuid"] = LocalXuid() };
        PlayFabLobbyConfig config = TutorialSupport.LobbyConfig(4, PlayFabLobbyConfig.ACCESSPOLICYPUBLIC,
            PlayFabLobbyConfig.OWNERMIGRATIONAUTOMATIC, search, lobbyProps, memberProps);
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
        await PublishActivityAsync();
        await PublishLobbyPresenceAsync();
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
        if (!await EnsureMultiplayerInitializedAsync() || !await CanUseMultiplayerAsync())
        {
            SetState(State.Ready);
            return false;
        }
        var memberProps = new Godot.Collections.Dictionary { ["loadout"] = "shotgun", ["xuid"] = LocalXuid() };
        PlayFabLobbyJoinConfig config = TutorialSupport.LobbyJoinConfig(memberProps);
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
        await PublishActivityAsync();
        await PublishLobbyPresenceAsync();
        LobbyJoined?.Invoke(_lobby);
        return true;
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
        await ClearActivityAsync();
        PlayFabResult pf = await _lobby.LeaveAsync();
        if (pf.Ok) GD.Print("[Lobby] left lobby"); else GD.PushWarning($"[Lobby] leave failed: {pf.Message}");
        await ClearLobbyPresenceAsync();
        DetachLobby();
        SetState(State.Ready);
        LobbyLeft?.Invoke();
        return true;
    }

    public async Task<bool> InviteFriendAsync(string xuid)
    {
        if (!IsInLobby())
        {
            GD.PushWarning("[MPA] Cannot invite — not in a lobby");
            return false;
        }
        string[] allowed = await FilterInvitableAsync(new[] { xuid });
        if (allowed.Length == 0)
        {
            GD.PushWarning($"[MPA] Invite blocked by play_multiplayer permission for {xuid}");
            return false;
        }
        GdkResult result = await Gdk.MultiplayerActivity.SendInvitesAsync(_auth.XboxUser, allowed, false, _lobby.ConnectionString);
        if (result.Ok)
        {
            GD.Print($"[MPA] Sent invite to {allowed[0]}");
            return true;
        }
        GD.PushWarning($"[MPA] send_invites failed: {result.Message} ({result.Code})");
        return false;
    }

    public async Task OpenInvitePickerAsync()
    {
        if (!IsInLobby())
        {
            GD.PushWarning("[MPA] Cannot open picker — not in a lobby");
            return;
        }
        GdkResult result = await Gdk.MultiplayerActivity.ShowInviteUiAsync(_auth.XboxUser);
        if (!result.Ok) GD.PushWarning($"[MPA] show_invite_ui failed: {result.Message}");
    }

    public async Task TrackFriendActivitiesAsync(string[] xuids)
    {
        _watchedXuids = xuids ?? Array.Empty<string>();
        GdkUser user = _auth.XboxUser;
        GdkResult activities = await Gdk.MultiplayerActivity.GetActivitiesAsync(user, _watchedXuids);
        if (!activities.Ok) GD.PushWarning($"[MPA] get_activities failed: {activities.Message}");
        Gdk.Presence.TrackPresence(user, _watchedXuids, Array.Empty<long>());
        GdkResult presence = await Gdk.Presence.GetPresenceAsync(_watchedXuids);
        if (!presence.Ok) GD.PushWarning($"[Pres] get_presence failed: {presence.Message}");
        foreach (string xuid in _watchedXuids)
        {
            PrintActivity(xuid);
            PrintPresence(xuid);
        }
    }

    public void StopTrackingFriends()
    {
        if (_watchedXuids.Length == 0) return;
        Gdk.Presence.StopTrackingPresence(_auth.XboxUser, _watchedXuids, Array.Empty<long>());
        _watchedXuids = Array.Empty<string>();
    }

    public async Task<bool> ConfirmPendingInviteAsync(int inviteId)
    {
        if (string.IsNullOrEmpty(_pendingInviteConnectionString) || inviteId != _pendingInviteId)
        {
            GD.PushWarning($"[MPA] confirm_pending_invite(id={inviteId}) rejected — stale or empty");
            return false;
        }
        if (_pendingInviteConfirming) return false;
        _pendingInviteConfirming = true;
        string cs = _pendingInviteConnectionString;
        int id = _pendingInviteId;
        _pendingInviteConnectionString = string.Empty;
        if (_state == State.InLobby && !await LeaveLobbyAsync())
        {
            _pendingInviteConfirming = false;
            InvitePendingCleared?.Invoke(id);
            return false;
        }
        bool ok = await JoinLobbyAsync(cs);
        _pendingInviteConfirming = false;
        InvitePendingCleared?.Invoke(id);
        return ok;
    }

    public void RejectPendingInvite(int inviteId)
    {
        if (inviteId == _pendingInviteId) ClearPendingInvite();
    }

    private string LocalXuid() => _auth?.XboxUser?.Xuid ?? string.Empty;

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

    private async Task PublishActivityAsync(bool allowCrossPlatformJoin = false)
    {
        if (_lobby == null || _auth.XboxUser == null) return;
        GdkResult result = await Gdk.MultiplayerActivity.SetActivityAsync(_auth.XboxUser, _lobby.ConnectionString,
            MpaJoinRestrictionFollowed, _lobby.MaxMemberCount, _lobby.MemberCount, string.Empty, allowCrossPlatformJoin);
        if (!result.Ok) GD.PushWarning($"[MPA] set_activity failed: {result.Message} ({result.Code})");
        else GD.Print($"[MPA] Activity advertised: max={_lobby.MaxMemberCount} current={_lobby.MemberCount} cross_platform={allowCrossPlatformJoin}");
    }

    private async Task ClearActivityAsync()
    {
        if (_auth.XboxUser == null) return;
        GdkResult result = await Gdk.MultiplayerActivity.DeleteActivityAsync(_auth.XboxUser);
        if (result.Ok) GD.Print("[MPA] Activity cleared"); else GD.PushWarning($"[MPA] delete_activity failed: {result.Message}");
    }

    private async Task PublishLobbyPresenceAsync()
    {
        if (string.IsNullOrEmpty(PresenceInLobby) || _auth.XboxUser == null) return;
        GdkResult result = await Gdk.Presence.SetPresenceAsync(_auth.XboxUser, PresenceInLobby);
        if (!result.Ok) GD.PushWarning($"[Lobby] presence write failed: {result.Message}");
    }

    private async Task ClearLobbyPresenceAsync()
    {
        if (_auth.XboxUser == null) return;
        GdkResult result = await Gdk.Presence.ClearPresenceAsync(_auth.XboxUser);
        if (!result.Ok) GD.PushWarning($"[Lobby] presence clear failed: {result.Message}");
    }

    private void OnLobbyStateChanged(PlayFabLobbyStateChange change)
    {
        switch (change.Kind)
        {
            case PlayFabLobby.MEMBERADDED:
                GD.Print($"[Lobby] member added: {change.Member?.UserId} (local={change.Member?.IsLocal})");
                _ = PublishActivityAsync();
                break;
            case PlayFabLobby.MEMBERREMOVED:
                GD.Print($"[Lobby] member removed: {change.Member?.UserId}");
                _ = PublishActivityAsync();
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
        if (_state != State.InLobby)
        {
            GD.PushWarning($"[Lobby] DISCONNECTED received in state={(int)_state} — ignoring");
            return;
        }
        if (!IsInsideTree()) return;
        GD.PushWarning("[Lobby] disconnected from lobby");
        await ClearActivityAsync();
        await ClearLobbyPresenceAsync();
        DetachLobby();
        LobbyDisconnected?.Invoke();
        SetState(State.Ready);
    }

    private void OnPendingInviteReceived(Godot.Collections.Dictionary invite) =>
        GD.Print($"[MPA] Pending invite (not yet accepted): {TutorialSupport.DictString(invite, "raw_uri")}");

    private void OnInviteAccepted(Godot.Collections.Dictionary invite)
    {
        GD.Print($"[MPA] Invite accepted from Game Bar: scheme={TutorialSupport.DictString(invite, "scheme")} action={TutorialSupport.DictString(invite, "action")}");
        string cs = TutorialSupport.DictString(invite, "connectionstring");
        if (string.IsNullOrEmpty(cs))
        {
            GD.PushWarning($"[MPA] Invite did not carry a connection string: {TutorialSupport.DictString(invite, "raw_uri")}");
            return;
        }
        if (IsBusy())
        {
            GD.PushWarning($"[MPA] Invite accepted while busy (state={(int)_state}) — ignoring");
            return;
        }
        if (_state != State.InLobby)
        {
            _ = JoinLobbyAsync(cs);
            return;
        }
        if (_pendingInviteConfirming)
        {
            GD.PushWarning("[MPA] Invite arrived while confirming another — dropping new invite");
            return;
        }
        _pendingInviteId++;
        _pendingInviteConnectionString = cs;
        InvitePendingConfirmation?.Invoke(_pendingInviteId, cs);
        GD.Print($"[MPA] Invite pending confirmation (id={_pendingInviteId}) — waiting for UI accept/reject");
    }

    private void ClearPendingInvite()
    {
        if (string.IsNullOrEmpty(_pendingInviteConnectionString)) return;
        int id = _pendingInviteId;
        _pendingInviteConnectionString = string.Empty;
        InvitePendingCleared?.Invoke(id);
    }

    private void OnActivitiesUpdated(string[] xuids)
    {
        GD.Print($"[MPA] Activity updated for friends: {string.Join(",", xuids ?? Array.Empty<string>())}");
        if (xuids == null) return;
        foreach (string xuid in xuids) PrintActivity(xuid);
    }

    private void OnDevicePresenceChanged(string xuid)
    {
        if (Array.IndexOf(_watchedXuids, xuid) >= 0) _ = Gdk.Presence.GetPresenceAsync(new[] { xuid });
    }

    private void OnTitlePresenceChanged(string xuid, int titleId)
    {
        if (Array.IndexOf(_watchedXuids, xuid) >= 0) _ = Gdk.Presence.GetPresenceAsync(new[] { xuid });
    }

    private void OnPresenceChanged(string xuid, GdkPresenceRecord record) => PrintPresence(xuid);

    private void PrintActivity(string xuid)
    {
        GdkMultiplayerActivityInfo info = Gdk.MultiplayerActivity.GetCachedActivity(xuid);
        if (info == null)
        {
            GD.Print($"[MPA] Friend {xuid} is offline / not in a session");
            return;
        }
        string conn = info.ConnectionString;
        GD.Print(string.IsNullOrEmpty(conn) ? $"[MPA] Friend {xuid} cleared their session" : $"[MPA] Friend {xuid} is in session: {conn}");
    }

    private void PrintPresence(string xuid)
    {
        GdkPresenceRecord record = Gdk.Presence.GetCachedPresence(xuid);
        if (record == null)
        {
            GD.Print($"[Pres] {xuid}: (unknown)");
            return;
        }
        string rich = string.Empty;
        Godot.Collections.Array titles = record.TitleRecords;
        if (titles.Count > 0) rich = TutorialSupport.DictString(titles[0].AsGodotDictionary(), "rich_presence_string");
        GD.Print($"[Pres] {xuid}: state={record.UserStateName} rich={rich}");
    }
}

