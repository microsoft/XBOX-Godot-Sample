using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabEvents : PlayFabServiceBase
{
    internal PlayFabEvents(GodotObject o) : base(o)
    {
    }
}
