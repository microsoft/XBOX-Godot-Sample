using System;
using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.stats</c> — title-managed and user statistics.</summary>
public sealed class XboxStats : XboxServiceBase
{
    internal XboxStats(GodotObject o) : base(o)
    {
        _o.Connect("stats_updated", Callable.From((Variant a0, Variant a1) =>
            StatsUpdated?.Invoke(XboxUser.From(a0.AsGodotObject()), a1.AsGodotDictionary())));
        _o.Connect("stat_changed", Callable.From((Variant a0, Variant a1, Variant a2) =>
            StatChanged?.Invoke(XboxUser.From(a0.AsGodotObject()), a1.AsString(), a2)));
        _o.Connect("stats_flushed", Callable.From((Variant a0, Variant a1) =>
            StatsFlushed?.Invoke(XboxUser.From(a0.AsGodotObject()), XboxResult.From(a1.AsGodotObject()))));
    }

    public event Action<XboxUser, Godot.Collections.Dictionary> StatsUpdated;
    public event Action<XboxUser, string, Variant> StatChanged;
    public event Action<XboxUser, XboxResult> StatsFlushed;

    public Task<XboxResult> QueryUserStatsAsync(XboxUser user, string[] statNames) =>
        CallResultAsync("query_user_stats_async", user?.Raw, statNames);

    public Task<XboxResult> QueryUsersStatsAsync(XboxUser user, string[] xuids, string[] statNames) =>
        CallResultAsync("query_users_stats_async", user?.Raw, xuids, statNames);

    public XboxResult SetStatInteger(XboxUser user, string statName, long value) =>
        XboxResult.From(Call("set_stat_integer", user?.Raw, statName, value).AsGodotObject());

    public XboxResult SetStatNumber(XboxUser user, string statName, double value) =>
        XboxResult.From(Call("set_stat_number", user?.Raw, statName, value).AsGodotObject());

    public Task<XboxResult> FlushStatsAsync(XboxUser user) =>
        CallResultAsync("flush_stats_async", user?.Raw);

    public XboxResult TrackStats(XboxUser user, string[] statNames) =>
        XboxResult.From(Call("track_stats", user?.Raw, statNames).AsGodotObject());

    public XboxResult StopTrackingStats(XboxUser user, string[] statNames) =>
        XboxResult.From(Call("stop_tracking_stats", user?.Raw, statNames).AsGodotObject());

    public Godot.Collections.Dictionary GetCachedStats(XboxUser user) =>
        Call("get_cached_stats", user?.Raw).AsGodotDictionary();

    public Task<XboxResult> GetSingleStatAsync(XboxUser user, string statName) =>
        CallResultAsync("get_single_stat_async", user?.Raw, statName);

    public Task<XboxResult> WriteStatsAsync(XboxUser user, Godot.Collections.Dictionary stats) =>
        CallResultAsync("write_stats_async", user?.Raw, stats);

    public Task<XboxResult> DeleteStatsAsync(XboxUser user, string[] statNames) =>
        CallResultAsync("delete_stats_async", user?.Raw, statNames);
}
