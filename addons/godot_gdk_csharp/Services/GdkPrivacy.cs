using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.privacy</c> — permission checks and avoid/mute lists.</summary>
public sealed class GdkPrivacy : GdkServiceBase
{
    internal GdkPrivacy(GodotObject o) : base(o) { }

    public Task<GdkResult> CheckPermissionAsync(GdkUser user, string permission, string targetXuid) =>
        CallResultAsync("check_permission_async", user?.Raw, permission, targetXuid);

    public Task<GdkResult> CheckPermissionForAnonymousUserAsync(GdkUser user, string permission, string anonymousUserType) =>
        CallResultAsync("check_permission_for_anonymous_user_async", user?.Raw, permission, anonymousUserType);

    public Task<GdkResult> BatchCheckPermissionAsync(GdkUser user, string permission, string[] targetXuids) =>
        CallResultAsync("batch_check_permission_async", user?.Raw, permission, targetXuids);

    public Task<GdkResult> GetAvoidListAsync(GdkUser user) =>
        CallResultAsync("get_avoid_list_async", user?.Raw);

    public Task<GdkResult> GetMuteListAsync(GdkUser user) =>
        CallResultAsync("get_mute_list_async", user?.Raw);
}
