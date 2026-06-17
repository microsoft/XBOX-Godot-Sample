using System;
using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.multiplayer_activity</c> — activities, invites, and recent players.</summary>
public sealed class GdkMultiplayerActivity : GdkServiceBase
{
    internal GdkMultiplayerActivity(GodotObject o) : base(o)
    {
        _o.Connect("activities_updated", Callable.From((Variant a0) =>
            ActivitiesUpdated?.Invoke(a0.AsStringArray())));
        _o.Connect("pending_invite_received", Callable.From((Variant a0) =>
            PendingInviteReceived?.Invoke(a0.AsGodotDictionary())));
        _o.Connect("invite_accepted", Callable.From((Variant a0) =>
            InviteAccepted?.Invoke(a0.AsGodotDictionary())));
    }

    public event Action<string[]> ActivitiesUpdated;
    public event Action<Godot.Collections.Dictionary> PendingInviteReceived;
    public event Action<Godot.Collections.Dictionary> InviteAccepted;

    public Task<GdkResult> SetActivityAsync(
        GdkUser user, string connectionString, string joinRestriction,
        int maxPlayers, int currentPlayers, string groupId, bool allowCrossPlatformJoin) =>
        CallResultAsync("set_activity_async", user?.Raw, connectionString, joinRestriction,
            maxPlayers, currentPlayers, groupId, allowCrossPlatformJoin);

    public Task<GdkResult> GetActivitiesAsync(GdkUser user, string[] xuids) =>
        CallResultAsync("get_activities_async", user?.Raw, xuids);

    public GdkMultiplayerActivityInfo GetCachedActivity(string xuid) =>
        GdkMultiplayerActivityInfo.From(Call("get_cached_activity", xuid).AsGodotObject());

    public Task<GdkResult> DeleteActivityAsync(GdkUser user) =>
        CallResultAsync("delete_activity_async", user?.Raw);

    public Task<GdkResult> SendInvitesAsync(GdkUser user, string[] xuids, bool allowCrossPlatformJoin, string connectionString) =>
        CallResultAsync("send_invites_async", user?.Raw, xuids, allowCrossPlatformJoin, connectionString);

    public Task<GdkResult> ShowInviteUiAsync(GdkUser user) =>
        CallResultAsync("show_invite_ui_async", user?.Raw);

    public GdkResult UpdateRecentPlayers(GdkUser user, string[] xuids, string encounterType) =>
        GdkResult.From(Call("update_recent_players", user?.Raw, xuids, encounterType).AsGodotObject());

    public Task<GdkResult> FlushRecentPlayersAsync(GdkUser user) =>
        CallResultAsync("flush_recent_players_async", user?.Raw);

    public GdkResult AcceptPendingInvite(string inviteUri) =>
        GdkResult.From(Call("accept_pending_invite", inviteUri).AsGodotObject());
}
