using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabPlayerData : PlayFabServiceBase
{
    internal PlayFabPlayerData(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> DeletePlayerCustomPropertiesAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_player_custom_properties_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayerCustomPropertyAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_player_custom_property_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetUserDataAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_user_data_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetUserPublisherDataAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_user_publisher_data_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetUserPublisherReadOnlyDataAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_user_publisher_read_only_data_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetUserReadOnlyDataAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_user_read_only_data_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ListPlayerCustomPropertiesAsync(PlayFabUser user) =>
        CallResultAsync("list_player_custom_properties_async", user?.Raw);

    public Task<PlayFabResult> UpdatePlayerCustomPropertiesAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_player_custom_properties_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateUserDataAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_user_data_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateUserPublisherDataAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_user_publisher_data_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
