using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabLocalization : PlayFabServiceBase
{
    internal PlayFabLocalization(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> GetLanguageListAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_language_list_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
