using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbyJoinConfig : PlayFabObject
{
    internal PlayFabLobbyJoinConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbyJoinConfig From(GodotObject o) => o == null ? null : new PlayFabLobbyJoinConfig(o);

    public Godot.Collections.Dictionary MemberProperties => GetDict("member_properties");
}
