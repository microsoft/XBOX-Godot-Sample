using System;
using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.leaderboards</c> — read-only leaderboard queries and paging.</summary>
public sealed class XboxLeaderboards : XboxServiceBase
{
    internal XboxLeaderboards(GodotObject o) : base(o)
    {
        _o.Connect("leaderboard_updated", Callable.From((Variant a0, Variant a1) =>
            LeaderboardUpdated?.Invoke(a0.AsString(), XboxLeaderboard.From(a1.AsGodotObject()))));
    }

    public event Action<string, XboxLeaderboard> LeaderboardUpdated;

    public Task<XboxResult> GetLeaderboardAsync(XboxUser user, string statName, int maxItems) =>
        CallResultAsync("get_leaderboard_async", user?.Raw, statName, maxItems);

    public Task<XboxResult> GetLeaderboardAroundUserAsync(XboxUser user, string statName, int maxItems) =>
        CallResultAsync("get_leaderboard_around_user_async", user?.Raw, statName, maxItems);

    public Task<XboxResult> GetSocialLeaderboardAsync(XboxUser user, string statName, int maxItems) =>
        CallResultAsync("get_social_leaderboard_async", user?.Raw, statName, maxItems);

    public Task<XboxResult> GetNextPageAsync(XboxLeaderboard leaderboard) =>
        CallResultAsync("get_next_page_async", leaderboard?.Raw);

    public XboxLeaderboard GetCachedLeaderboard(string statName) =>
        XboxLeaderboard.From(Call("get_cached_leaderboard", statName).AsGodotObject());
}
