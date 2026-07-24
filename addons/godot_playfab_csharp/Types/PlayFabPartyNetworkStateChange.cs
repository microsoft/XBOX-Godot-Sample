using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyNetworkStateChange : PlayFabObject
{
    internal PlayFabPartyNetworkStateChange(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyNetworkStateChange From(GodotObject o) => o == null ? null : new PlayFabPartyNetworkStateChange(o);

    public int Kind => GetInt32("kind");

    public PlayFabPartyNetwork Network => PlayFabPartyNetwork.From(GetObject("network"));

    public PlayFabResult Result => PlayFabResult.From(GetObject("result"));

    public PlayFabUser User => PlayFabUser.From(GetObject("user"));

    public int PeerId => GetInt32("peer_id");

    public int State => GetInt32("state");

    public string Reason => GetString("reason");

    public int GetKind() =>
        Call("get_kind").AsInt32();

    public PlayFabPartyNetwork GetNetwork() =>
        PlayFabPartyNetwork.From(Call("get_network").AsGodotObject());

    public PlayFabResult GetResult() =>
        PlayFabResult.From(Call("get_result").AsGodotObject());

    public PlayFabUser GetUser() =>
        PlayFabUser.From(Call("get_user").AsGodotObject());

    public int GetPeerId() =>
        Call("get_peer_id").AsInt32();

    public int GetState() =>
        Call("get_state").AsInt32();

    public string GetReason() =>
        Call("get_reason").AsString();
}
