using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabMultiplayerConfig : PlayFabObject
{
    internal PlayFabMultiplayerConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabMultiplayerConfig From(GodotObject o) => o == null ? null : new PlayFabMultiplayerConfig(o);
}
