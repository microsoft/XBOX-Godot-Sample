using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabLeaderboards : PlayFabServiceBase
{
    /// <summary>
    /// External providers that PlayFab should include in a friend leaderboard.
    /// Ordinary providers can be combined. <see cref="All"/> is a distinct selector
    /// and must be used alone; invalid masks are reported by the native API.
    /// </summary>
    [Flags]
    public enum FriendSources : long
    {
        None = 0x00,
        Steam = 0x01,
        Facebook = 0x02,
        Xbox = 0x04,
        Psn = 0x08,
        All = 0x10,
    }

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

    /// <summary>
    /// Gets a friend leaderboard using the selected external providers. Xbox-containing
    /// selections, including <see cref="FriendSources.All"/>, require an Xbox-backed
    /// PlayFab session. Native validation and service failures are returned as
    /// <see cref="PlayFabResult"/> values.
    /// </summary>
    public Task<PlayFabResult> GetFriendLeaderboardWithSourcesAsync(
        PlayFabUser user,
        string leaderboard_name,
        FriendSources friend_sources,
        int version = -1) =>
        CallResultAsync(
            "get_friend_leaderboard_with_sources_async",
            user?.Raw, leaderboard_name, (long)friend_sources, version);
}
