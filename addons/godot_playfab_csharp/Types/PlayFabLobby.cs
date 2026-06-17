using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobby : PlayFabObject
{
    internal PlayFabLobby(GodotObject o) : base(o)
    {
        _o.Connect("state_changed", Callable.From((Variant a0) =>
            StateChanged?.Invoke(PlayFabLobbyStateChange.From(a0.AsGodotObject()))));
    }

    public static PlayFabLobby From(GodotObject o) => o == null ? null : new PlayFabLobby(o);

    public event Action<PlayFabLobbyStateChange> StateChanged;

    public const int MEMBERADDED = 1;

    public const int MEMBERREMOVED = 2;

    public const int MEMBERUPDATED = 3;

    public const int PROPERTIESUPDATED = 4;

    public const int OWNERCHANGED = 5;

    public const int DISCONNECTED = 6;

    public string LobbyId => GetString("lobby_id");

    public string ConnectionString => GetString("connection_string");

    public Godot.Collections.Dictionary OwnerEntityKey => GetDict("owner_entity_key");

    public int MaxMemberCount => GetInt32("max_member_count");

    public int MemberCount => GetInt32("member_count");

    public Godot.Collections.Array Members => GetArray("members");

    public Godot.Collections.Dictionary Properties => GetDict("properties");

    public Godot.Collections.Dictionary SearchProperties => GetDict("search_properties");

    public Task<PlayFabResult> SetPropertiesAsync(Godot.Collections.Dictionary properties) =>
        CallResultAsync("set_properties_async", properties ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetMemberPropertiesAsync(Godot.Collections.Dictionary properties) =>
        CallResultAsync("set_member_properties_async", properties ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> LeaveAsync() =>
        CallResultAsync("leave_async");

    public bool IsOwner(PlayFabUser user) =>
        Call("is_owner", user?.Raw).AsBool();
}
