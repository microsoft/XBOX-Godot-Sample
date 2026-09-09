using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobby : PlayFabObject
{
    internal PlayFabLobby(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobby From(GodotObject o) => o == null ? null : new PlayFabLobby(o);

    private Action<PlayFabLobbyStateChange> _stateChanged;
    private Callable _stateChangedCallable;

    public event Action<PlayFabLobbyStateChange> StateChanged
    {
        add => AddSignal(ref _stateChanged, value, "state_changed", ref _stateChangedCallable,
            () => Callable.From((Variant a0) =>
                _stateChanged?.Invoke(PlayFabLobbyStateChange.From(a0.AsGodotObject()))));
        remove => RemoveSignal(ref _stateChanged, value, "state_changed", ref _stateChangedCallable);
    }

    public const int MEMBERADDED = 1;

    public const int MEMBERREMOVED = 2;

    public const int MEMBERUPDATED = 3;

    public const int PROPERTIESUPDATED = 4;

    public const int OWNERCHANGED = 5;

    public const int DISCONNECTED = 6;

    public const int MEMBERCONNECTIONCHANGED = 7;

    public const int SEARCHPROPERTIESUPDATED = 8;

    public const int CONFIGURATIONUPDATED = 9;

    public const int DISCONNECTING = 10;

    public const int UPDATECOMPLETED = 11;

    public const int MEMBERSHIPLOCKUNLOCKED = 0;

    public const int MEMBERSHIPLOCKLOCKED = 1;

    public const int MEMBERREMOVEDLOCALUSERLEFTLOBBY = 0;

    public const int MEMBERREMOVEDLOCALUSERFORCIBLYREMOVED = 1;

    public const int MEMBERREMOVEDREMOTEUSERLEFTLOBBY = 2;

    public const int DISCONNECTINGNOLOCALMEMBERS = 0;

    public const int DISCONNECTINGLOBBYDELETED = 1;

    public const int DISCONNECTINGCONNECTIONINTERRUPTION = 2;

    public const int DISCONNECTINGLOBBYSERVERLEFT = 3;

    public string LobbyId => GetString("lobby_id");

    public string ConnectionString => GetString("connection_string");

    public Godot.Collections.Dictionary OwnerEntityKey => GetDict("owner_entity_key");

    public int MaxMemberCount => GetInt32("max_member_count");

    public int MemberCount => GetInt32("member_count");

    public Godot.Collections.Array Members => GetArray("members");

    public Godot.Collections.Dictionary Properties => GetDict("properties");

    public Godot.Collections.Dictionary SearchProperties => GetDict("search_properties");

    public int AccessPolicy => GetInt32("access_policy");

    public int OwnerMigrationPolicy => GetInt32("owner_migration_policy");

    public int MembershipLock => GetInt32("membership_lock");

    public bool RestrictInvitesToLobbyOwner => GetBool("restrict_invites_to_lobby_owner");

    public int DisconnectingReason => GetInt32("disconnecting_reason");

    public Task<PlayFabResult> SetPropertiesAsync(Godot.Collections.Dictionary properties) =>
        CallResultAsync("set_properties_async", properties ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetMemberPropertiesAsync(Godot.Collections.Dictionary properties) =>
        CallResultAsync("set_member_properties_async", properties ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetSearchPropertiesAsync(Godot.Collections.Dictionary searchProperties) =>
        CallResultAsync("set_search_properties_async", searchProperties ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetMembershipLockAsync(int membershipLock) =>
        CallResultAsync("set_membership_lock_async", membershipLock);

    public Task<PlayFabResult> PostUpdateAsync(PlayFabLobbyUpdateConfig update) =>
        CallResultAsync("post_update_async", update?.Raw);

    public Task<PlayFabResult> LeaveAsync() =>
        CallResultAsync("leave_async");

    public bool IsOwner(PlayFabUser user) =>
        Call("is_owner", user?.Raw).AsBool();

    public bool IsDisconnected() =>
        Call("is_disconnected").AsBool();

    public PlayFabLobbyMember FindMember(Godot.Collections.Dictionary entityKey) =>
        PlayFabLobbyMember.From(Call("find_member", entityKey ?? new Godot.Collections.Dictionary()).AsGodotObject());
}
