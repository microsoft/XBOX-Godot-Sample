using System;
using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.presence</c> — set/clear/query presence and presence tracking.</summary>
public sealed class XboxPresence : XboxServiceBase
{
    internal XboxPresence(GodotObject o) : base(o)
    {
        _o.Connect("presence_changed", Callable.From((Variant a0, Variant a1) =>
            PresenceChanged?.Invoke(a0.AsString(), XboxPresenceRecord.From(a1.AsGodotObject()))));
        _o.Connect("local_presence_set", Callable.From((Variant a0) =>
            LocalPresenceSet?.Invoke(XboxUser.From(a0.AsGodotObject()))));
        _o.Connect("device_presence_changed", Callable.From((Variant a0) =>
            DevicePresenceChanged?.Invoke(a0.AsString())));
        _o.Connect("title_presence_changed", Callable.From((Variant a0, Variant a1) =>
            TitlePresenceChanged?.Invoke(a0.AsString(), a1.AsInt32())));
    }

    public event Action<string, XboxPresenceRecord> PresenceChanged;
    public event Action<XboxUser> LocalPresenceSet;
    public event Action<string> DevicePresenceChanged;
    public event Action<string, int> TitlePresenceChanged;

    public Task<XboxResult> SetPresenceAsync(XboxUser user, string state, Godot.Collections.Dictionary richPresence = null) =>
        CallResultAsync("set_presence_async", user?.Raw, state, richPresence ?? new Godot.Collections.Dictionary());

    public Task<XboxResult> ClearPresenceAsync(XboxUser user) =>
        CallResultAsync("clear_presence_async", user?.Raw);

    public Task<XboxResult> GetPresenceAsync(string[] xuids) =>
        CallResultAsync("get_presence_async", xuids);

    public Task<XboxResult> GetPresenceForSocialGroupAsync(XboxUser user, string socialGroup) =>
        CallResultAsync("get_presence_for_social_group_async", user?.Raw, socialGroup);

    public XboxResult TrackPresence(XboxUser user, string[] xuids, long[] titleIds) =>
        XboxResult.From(Call("track_presence", user?.Raw, xuids, titleIds).AsGodotObject());

    public XboxResult StopTrackingPresence(XboxUser user, string[] xuids, long[] titleIds) =>
        XboxResult.From(Call("stop_tracking_presence", user?.Raw, xuids, titleIds).AsGodotObject());

    public XboxPresenceRecord GetCachedPresence(string xuid) =>
        XboxPresenceRecord.From(Call("get_cached_presence", xuid).AsGodotObject());
}
