using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabCloudScript : PlayFabServiceBase
{
    internal PlayFabCloudScript(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> ExecuteCloudScriptAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("execute_cloud_script_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ExecuteEntityCloudScriptAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("execute_entity_cloud_script_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ExecuteFunctionAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("execute_function_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
