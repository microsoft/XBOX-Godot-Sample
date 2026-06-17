using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyChat : PlayFabObject
{
    internal PlayFabPartyChat(GodotObject o) : base(o)
    {
        _o.Connect("state_changed", Callable.From((Variant a0) =>
            StateChanged?.Invoke(PlayFabPartyChatStateChange.From(a0.AsGodotObject()))));
    }

    public static PlayFabPartyChat From(GodotObject o) => o == null ? null : new PlayFabPartyChat(o);

    public event Action<PlayFabPartyChatStateChange> StateChanged;

    public PlayFabPartyChatControl GetLocalChatControl(PlayFabUser user) =>
        PlayFabPartyChatControl.From(Call("get_local_chat_control", user?.Raw).AsGodotObject());

    public Godot.Collections.Array GetChatControls() =>
        Call("get_chat_controls").AsGodotArray();
}
