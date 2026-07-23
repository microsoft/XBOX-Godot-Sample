using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyNetwork : PlayFabObject
{
    internal PlayFabPartyNetwork(GodotObject o) : base(o)
    {
        _o.Connect("state_changed", Callable.From((Variant a0) =>
            StateChanged?.Invoke(PlayFabPartyNetworkStateChange.From(a0.AsGodotObject()))));
    }

    public static PlayFabPartyNetwork From(GodotObject o) => o == null ? null : new PlayFabPartyNetwork(o);

    public event Action<PlayFabPartyNetworkStateChange> StateChanged;

    public string NetworkId => GetString("network_id");

    public string Descriptor => GetString("descriptor");

    public int State => GetInt32("state");

    public PlayFabUser LocalUser => PlayFabUser.From(GetObject("local_user"));

    public PlayFabPartyPeer LocalPeer => PlayFabPartyPeer.From(GetObject("local_peer"));

    public PlayFabPartyChatControl LocalChatControl => PlayFabPartyChatControl.From(GetObject("local_chat_control"));

    public bool IsHost => GetBool("is_host");

    public string GetNetworkId() =>
        Call("get_network_id").AsString();

    public string GetDescriptor() =>
        Call("get_descriptor").AsString();

    public int GetState() =>
        Call("get_state").AsInt32();

    public PlayFabUser GetLocalUser() =>
        PlayFabUser.From(Call("get_local_user").AsGodotObject());

    public PlayFabPartyPeer GetLocalPeer() =>
        PlayFabPartyPeer.From(Call("get_local_peer").AsGodotObject());

    public PlayFabPartyChatControl GetLocalChatControl() =>
        PlayFabPartyChatControl.From(Call("get_local_chat_control").AsGodotObject());

    public bool IsHostNetwork() =>
        Call("is_host_network").AsBool();

    public Task<PlayFabResult> LeaveAsync() =>
        CallResultAsync("leave_async");
}
