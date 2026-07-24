using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabMatchmakingMember : PlayFabObject
{
    internal PlayFabMatchmakingMember(GodotObject o) : base(o)
    {
    }

    public static PlayFabMatchmakingMember From(GodotObject o) => o == null ? null : new PlayFabMatchmakingMember(o);

    public PlayFabUser User => PlayFabUser.From(GetObject("user"));

    public Godot.Collections.Dictionary Attributes => GetDict("attributes");
}
