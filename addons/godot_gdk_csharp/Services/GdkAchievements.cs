using System;
using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.achievements</c> — query and update player achievements.</summary>
public sealed class GdkAchievements : GdkServiceBase
{
    internal GdkAchievements(GodotObject o) : base(o)
    {
        _o.Connect("achievement_unlocked", Callable.From((Variant a0, Variant a1) =>
            AchievementUnlocked?.Invoke(GdkUser.From(a0.AsGodotObject()), a1.AsString())));
        _o.Connect("achievements_updated", Callable.From((Variant a0) =>
            AchievementsUpdated?.Invoke(GdkUser.From(a0.AsGodotObject()))));
        _o.Connect("runtime_error", Callable.From((Variant a0) =>
            RuntimeError?.Invoke(GdkResult.From(a0.AsGodotObject()))));
    }

    public event Action<GdkUser, string> AchievementUnlocked;
    public event Action<GdkUser> AchievementsUpdated;
    public event Action<GdkResult> RuntimeError;

    public Task<GdkResult> QueryPlayerAchievementsAsync(GdkUser user) =>
        CallResultAsync("query_player_achievements_async", user?.Raw);

    public Task<GdkResult> UpdateAchievementAsync(GdkUser user, string achievementId, int percentComplete) =>
        CallResultAsync("update_achievement_async", user?.Raw, achievementId, percentComplete);

    public Godot.Collections.Array GetCachedAchievements(GdkUser user) =>
        Call("get_cached_achievements", user?.Raw).AsGodotArray();
}
