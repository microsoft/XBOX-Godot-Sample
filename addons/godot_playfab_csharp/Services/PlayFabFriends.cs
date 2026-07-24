using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabFriends : PlayFabServiceBase
{
    internal PlayFabFriends(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> AddFriendAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("add_friend_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetFriendsListAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_friends_list_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RemoveFriendAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("remove_friend_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetFriendTagsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("set_friend_tags_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
