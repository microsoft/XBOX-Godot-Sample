using System;
using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.leaderboards</c> — read-only leaderboard queries and paging.</summary>
public sealed class GdkLeaderboards : GdkServiceBase
{
    internal GdkLeaderboards(GodotObject o) : base(o)
    {
        _o.Connect("leaderboard_updated", Callable.From((Variant a0, Variant a1) =>
            LeaderboardUpdated?.Invoke(a0.AsString(), GdkLeaderboard.From(a1.AsGodotObject()))));
    }

    public event Action<string, GdkLeaderboard> LeaderboardUpdated;

    public Task<GdkResult> GetLeaderboardAsync(GdkUser user, string statName, int maxItems) =>
        CallResultAsync("get_leaderboard_async", user?.Raw, statName, maxItems);

    public Task<GdkResult> GetLeaderboardAroundUserAsync(GdkUser user, string statName, int maxItems) =>
        CallResultAsync("get_leaderboard_around_user_async", user?.Raw, statName, maxItems);

    public Task<GdkResult> GetSocialLeaderboardAsync(GdkUser user, string statName, int maxItems) =>
        CallResultAsync("get_social_leaderboard_async", user?.Raw, statName, maxItems);

    public Task<GdkResult> GetNextPageAsync(GdkLeaderboard leaderboard) =>
        CallResultAsync("get_next_page_async", leaderboard?.Raw);

    public GdkLeaderboard GetCachedLeaderboard(string statName) =>
        GdkLeaderboard.From(Call("get_cached_leaderboard", statName).AsGodotObject());
}
