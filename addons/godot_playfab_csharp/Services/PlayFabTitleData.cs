using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabTitleData : PlayFabServiceBase
{
    internal PlayFabTitleData(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> GetPublisherDataAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_publisher_data_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetTimeAsync(PlayFabUser user) =>
        CallResultAsync("get_time_async", user?.Raw);

    public Task<PlayFabResult> GetTitleDataAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_title_data_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetTitleNewsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_title_news_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
