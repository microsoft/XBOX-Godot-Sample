using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyChatStateChange : PlayFabObject
{
    internal PlayFabPartyChatStateChange(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyChatStateChange From(GodotObject o) => o == null ? null : new PlayFabPartyChatStateChange(o);

    public int Kind => GetInt32("kind");

    public PlayFabPartyChatControl ChatControl => PlayFabPartyChatControl.From(GetObject("chat_control"));

    public PlayFabResult Result => PlayFabResult.From(GetObject("result"));

    public string Reason => GetString("reason");

    public int GetKind() =>
        Call("get_kind").AsInt32();

    public PlayFabPartyChatControl GetChatControl() =>
        PlayFabPartyChatControl.From(Call("get_chat_control").AsGodotObject());

    public PlayFabResult GetResult() =>
        PlayFabResult.From(Call("get_result").AsGodotObject());

    public string GetReason() =>
        Call("get_reason").AsString();
}
