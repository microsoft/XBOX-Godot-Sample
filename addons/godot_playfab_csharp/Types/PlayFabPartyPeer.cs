using System;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyPeer : PlayFabObject
{
    internal PlayFabPartyPeer(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyPeer From(GodotObject o) => o == null ? null : new PlayFabPartyPeer(o);

    public MultiplayerPeer AsMultiplayerPeer => _o as MultiplayerPeer;

    private Action<int> _connectionStateChanged;
    private Callable _connectionStateChangedCallable;
    private Action<PlayFabResult> _networkError;
    private Callable _networkErrorCallable;

    public event Action<int> ConnectionStateChanged
    {
        add => AddSignal(ref _connectionStateChanged, value, "connection_state_changed", ref _connectionStateChangedCallable,
            () => Callable.From((Variant a0) =>
                _connectionStateChanged?.Invoke(a0.AsInt32())));
        remove => RemoveSignal(ref _connectionStateChanged, value, "connection_state_changed", ref _connectionStateChangedCallable);
    }

    public event Action<PlayFabResult> NetworkError
    {
        add => AddSignal(ref _networkError, value, "network_error", ref _networkErrorCallable,
            () => Callable.From((Variant a0) =>
                _networkError?.Invoke(PlayFabResult.From(a0.AsGodotObject()))));
        remove => RemoveSignal(ref _networkError, value, "network_error", ref _networkErrorCallable);
    }

    public PlayFabPartyNetwork GetNetwork() =>
        PlayFabPartyNetwork.From(Call("get_network").AsGodotObject());

    public PlayFabUser GetLocalUser() =>
        PlayFabUser.From(Call("get_local_user").AsGodotObject());

    public string GetDescriptor() =>
        Call("get_descriptor").AsString();

    public Godot.Collections.Dictionary GetPeerEntityKey(int peer_id) =>
        Call("get_peer_entity_key", peer_id).AsGodotDictionary();

    public PlayFabPartyMember GetPeerMember(int peer_id) =>
        PlayFabPartyMember.From(Call("get_peer_member", peer_id).AsGodotObject());

    public Godot.Collections.Array GetPeers() =>
        Call("get_peers").AsGodotArray();

    public void CloseWithReason(string reason = "") =>
        Call("close_with_reason", reason);
}
