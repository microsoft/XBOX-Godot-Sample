using System;
using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.achievements</c> — query and update player achievements.</summary>
public sealed class XboxAchievements : XboxServiceBase
{
    internal XboxAchievements(GodotObject o) : base(o)
    {
        _o.Connect("achievement_unlocked", Callable.From((Variant a0, Variant a1) =>
            AchievementUnlocked?.Invoke(XboxUser.From(a0.AsGodotObject()), a1.AsString())));
        _o.Connect("achievements_updated", Callable.From((Variant a0) =>
            AchievementsUpdated?.Invoke(XboxUser.From(a0.AsGodotObject()))));
        _o.Connect("runtime_error", Callable.From((Variant a0) =>
            RuntimeError?.Invoke(XboxResult.From(a0.AsGodotObject()))));
    }

    public event Action<XboxUser, string> AchievementUnlocked;
    public event Action<XboxUser> AchievementsUpdated;
    public event Action<XboxResult> RuntimeError;

    public Task<XboxResult> QueryPlayerAchievementsAsync(XboxUser user) =>
        CallResultAsync("query_player_achievements_async", user?.Raw);

    public Task<XboxResult> UpdateAchievementAsync(XboxUser user, string achievementId, int percentComplete) =>
        CallResultAsync("update_achievement_async", user?.Raw, achievementId, percentComplete);

    public Godot.Collections.Array GetCachedAchievements(XboxUser user) =>
        Call("get_cached_achievements", user?.Raw).AsGodotArray();

    public XboxResult GetAchievementsByState(XboxUser user, string progressState) =>
        XboxResult.From(Call("get_achievements_by_state", user?.Raw, progressState).AsGodotObject());
}
