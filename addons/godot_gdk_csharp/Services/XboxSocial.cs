using System;
using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.social</c> — social graph, groups, and reputation feedback.</summary>
public sealed class XboxSocial : XboxServiceBase
{
    internal XboxSocial(GodotObject o) : base(o)
    {
        _o.Connect("social_graph_changed", Callable.From((Variant a0) =>
            SocialGraphChanged?.Invoke(XboxUser.From(a0.AsGodotObject()))));
        _o.Connect("social_group_updated", Callable.From((Variant a0) =>
            SocialGroupUpdated?.Invoke(XboxSocialGroup.From(a0.AsGodotObject()))));
        _o.Connect("social_user_changed", Callable.From((Variant a0, Variant a1) =>
            SocialUserChanged?.Invoke(a0.AsString(), XboxSocialUser.From(a1.AsGodotObject()))));
        _o.Connect("runtime_error", Callable.From((Variant a0) =>
            RuntimeError?.Invoke(XboxResult.From(a0.AsGodotObject()))));
    }

    public event Action<XboxUser> SocialGraphChanged;
    public event Action<XboxSocialGroup> SocialGroupUpdated;
    public event Action<string, XboxSocialUser> SocialUserChanged;
    public event Action<XboxResult> RuntimeError;

    public XboxResult StartSocialGraph(XboxUser user) =>
        XboxResult.From(Call("start_social_graph", user?.Raw).AsGodotObject());

    public void StopSocialGraph(XboxUser user) => Call("stop_social_graph", user?.Raw);

    public Task<XboxResult> GetFriendsAsync(XboxUser user) =>
        CallResultAsync("get_friends_async", user?.Raw);

    public XboxResult CreateSocialGroup(XboxUser user, XboxSocialFilter filter) =>
        XboxResult.From(Call("create_social_group", user?.Raw, filter?.Raw).AsGodotObject());

    public XboxResult CreateSocialGroupFromXuids(XboxUser user, string[] xuids) =>
        XboxResult.From(Call("create_social_group_from_xuids", user?.Raw, xuids).AsGodotObject());

    public void DestroySocialGroup(XboxSocialGroup group) => Call("destroy_social_group", group?.Raw);

    public XboxResult GetGroupUsers(XboxSocialGroup group) =>
        XboxResult.From(Call("get_group_users", group?.Raw).AsGodotObject());

    public Task<XboxResult> SubmitReputationFeedbackAsync(
        XboxUser user, string targetXuid, string feedbackType, string reason, string evidenceId) =>
        CallResultAsync("submit_reputation_feedback_async", user?.Raw, targetXuid, feedbackType, reason, evidenceId);

    public Task<XboxResult> SubmitBatchReputationFeedbackAsync(XboxUser user, Godot.Collections.Array feedbackItems) =>
        CallResultAsync("submit_batch_reputation_feedback_async", user?.Raw, feedbackItems);

    public XboxResult UpdateSocialUserGroup(XboxSocialGroup group, string[] xuids) =>
        XboxResult.From(Call("update_social_user_group", group?.Raw, xuids).AsGodotObject());

    public XboxResult SetRichPresencePolling(XboxUser user, bool enabled) =>
        XboxResult.From(Call("set_rich_presence_polling", user?.Raw, enabled).AsGodotObject());
}
