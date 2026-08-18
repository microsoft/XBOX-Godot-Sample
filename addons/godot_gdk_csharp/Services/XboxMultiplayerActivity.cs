using System;
using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.multiplayer_activity</c> — activities, invites, and recent players.</summary>
public sealed class XboxMultiplayerActivity : XboxServiceBase
{
    internal XboxMultiplayerActivity(GodotObject o) : base(o)
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

    public Task<XboxResult> SetActivityAsync(
        XboxUser user, string connectionString, string joinRestriction,
        int maxPlayers, int currentPlayers, string groupId, bool allowCrossPlatformJoin) =>
        CallResultAsync("set_activity_async", user?.Raw, connectionString, joinRestriction,
            maxPlayers, currentPlayers, groupId, allowCrossPlatformJoin);

    public Task<XboxResult> GetActivitiesAsync(XboxUser user, string[] xuids) =>
        CallResultAsync("get_activities_async", user?.Raw, xuids);

    public XboxMultiplayerActivityInfo GetCachedActivity(string xuid) =>
        XboxMultiplayerActivityInfo.From(Call("get_cached_activity", xuid).AsGodotObject());

    public Task<XboxResult> DeleteActivityAsync(XboxUser user) =>
        CallResultAsync("delete_activity_async", user?.Raw);

    public Task<XboxResult> SendInvitesAsync(XboxUser user, string[] xuids, bool allowCrossPlatformJoin, string connectionString) =>
        CallResultAsync("send_invites_async", user?.Raw, xuids, allowCrossPlatformJoin, connectionString);

    public Task<XboxResult> ShowInviteUiAsync(XboxUser user) =>
        CallResultAsync("show_invite_ui_async", user?.Raw);

    public XboxResult UpdateRecentPlayers(XboxUser user, string[] xuids, string encounterType) =>
        XboxResult.From(Call("update_recent_players", user?.Raw, xuids, encounterType).AsGodotObject());

    public Task<XboxResult> FlushRecentPlayersAsync(XboxUser user) =>
        CallResultAsync("flush_recent_players_async", user?.Raw);

    public XboxResult AcceptPendingInvite(string inviteUri) =>
        XboxResult.From(Call("accept_pending_invite", inviteUri).AsGodotObject());
}
