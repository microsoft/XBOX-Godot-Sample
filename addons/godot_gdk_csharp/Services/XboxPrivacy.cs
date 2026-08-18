using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.privacy</c> — permission checks and avoid/mute lists.</summary>
public sealed class XboxPrivacy : XboxServiceBase
{
    internal XboxPrivacy(GodotObject o) : base(o) { }

    public Task<XboxResult> CheckPermissionAsync(XboxUser user, string permission, string targetXuid) =>
        CallResultAsync("check_permission_async", user?.Raw, permission, targetXuid);

    public Task<XboxResult> CheckPermissionForAnonymousUserAsync(XboxUser user, string permission, string anonymousUserType) =>
        CallResultAsync("check_permission_for_anonymous_user_async", user?.Raw, permission, anonymousUserType);

    public Task<XboxResult> BatchCheckPermissionAsync(XboxUser user, string permission, string[] targetXuids) =>
        CallResultAsync("batch_check_permission_async", user?.Raw, permission, targetXuids);

    public Task<XboxResult> GetAvoidListAsync(XboxUser user) =>
        CallResultAsync("get_avoid_list_async", user?.Raw);

    public Task<XboxResult> GetMuteListAsync(XboxUser user) =>
        CallResultAsync("get_mute_list_async", user?.Raw);
}
