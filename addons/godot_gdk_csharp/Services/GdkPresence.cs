using System;
using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.presence</c> — set/clear/query presence and presence tracking.</summary>
public sealed class GdkPresence : GdkServiceBase
{
    internal GdkPresence(GodotObject o) : base(o)
    {
        _o.Connect("presence_changed", Callable.From((Variant a0, Variant a1) =>
            PresenceChanged?.Invoke(a0.AsString(), GdkPresenceRecord.From(a1.AsGodotObject()))));
        _o.Connect("local_presence_set", Callable.From((Variant a0) =>
            LocalPresenceSet?.Invoke(GdkUser.From(a0.AsGodotObject()))));
        _o.Connect("device_presence_changed", Callable.From((Variant a0) =>
            DevicePresenceChanged?.Invoke(a0.AsString())));
        _o.Connect("title_presence_changed", Callable.From((Variant a0, Variant a1) =>
            TitlePresenceChanged?.Invoke(a0.AsString(), a1.AsInt32())));
    }

    public event Action<string, GdkPresenceRecord> PresenceChanged;
    public event Action<GdkUser> LocalPresenceSet;
    public event Action<string> DevicePresenceChanged;
    public event Action<string, int> TitlePresenceChanged;

    public Task<GdkResult> SetPresenceAsync(GdkUser user, string state, Godot.Collections.Dictionary richPresence = null) =>
        CallResultAsync("set_presence_async", user?.Raw, state, richPresence ?? new Godot.Collections.Dictionary());

    public Task<GdkResult> ClearPresenceAsync(GdkUser user) =>
        CallResultAsync("clear_presence_async", user?.Raw);

    public Task<GdkResult> GetPresenceAsync(string[] xuids) =>
        CallResultAsync("get_presence_async", xuids);

    public Task<GdkResult> GetPresenceForSocialGroupAsync(GdkUser user, string socialGroup) =>
        CallResultAsync("get_presence_for_social_group_async", user?.Raw, socialGroup);

    public GdkResult TrackPresence(GdkUser user, string[] xuids, long[] titleIds) =>
        GdkResult.From(Call("track_presence", user?.Raw, xuids, titleIds).AsGodotObject());

    public GdkResult StopTrackingPresence(GdkUser user, string[] xuids, long[] titleIds) =>
        GdkResult.From(Call("stop_tracking_presence", user?.Raw, xuids, titleIds).AsGodotObject());

    public GdkPresenceRecord GetCachedPresence(string xuid) =>
        GdkPresenceRecord.From(Call("get_cached_presence", xuid).AsGodotObject());
}
