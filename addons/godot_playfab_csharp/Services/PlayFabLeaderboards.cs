using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabLeaderboards : PlayFabServiceBase
{
    internal PlayFabLeaderboards(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> SubmitScoreAsync(PlayFabUser user, string leaderboard_name, int score, Godot.Collections.Array additional_scores = null, string metadata = "") =>
        CallResultAsync("submit_score_async", user?.Raw, leaderboard_name, score, additional_scores ?? new Godot.Collections.Array(), metadata);

    public Task<PlayFabResult> GetLeaderboardAsync(PlayFabUser user, string leaderboard_name, int start_position = 1, int page_size = 10, int version = -1) =>
        CallResultAsync("get_leaderboard_async", user?.Raw, leaderboard_name, start_position, page_size, version);

    public Task<PlayFabResult> GetLeaderboardAroundUserAsync(PlayFabUser user, string leaderboard_name, int max_surrounding_entries = 10, int version = -1) =>
        CallResultAsync("get_leaderboard_around_user_async", user?.Raw, leaderboard_name, max_surrounding_entries, version);

    public Task<PlayFabResult> GetFriendLeaderboardAsync(PlayFabUser user, string leaderboard_name, bool include_xbox_friends = true, int version = -1) =>
        CallResultAsync("get_friend_leaderboard_async", user?.Raw, leaderboard_name, include_xbox_friends, version);
}
