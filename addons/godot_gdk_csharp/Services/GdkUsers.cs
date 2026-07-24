using System;
using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.users</c> — sign-in, the active user roster, and privileges.</summary>
public sealed class GdkUsers : GdkServiceBase
{
    internal GdkUsers(GodotObject o) : base(o)
    {
        _o.Connect("user_changed", Callable.From((Variant a0, Variant a1) =>
            UserChanged?.Invoke(GdkUser.From(a0.AsGodotObject()), a1.AsString())));
    }

    /// <summary>Raised for every user lifecycle event (added/removed/changed).</summary>
    public event Action<GdkUser, string> UserChanged;

    public Task<GdkResult> AddDefaultUserAsync() => CallResultAsync("add_default_user_async");

    public Task<GdkResult> AddUserWithUiAsync() => CallResultAsync("add_user_with_ui_async");

    public GdkUser GetPrimaryUser() => GdkUser.From(Call("get_primary_user").AsGodotObject());

    public Godot.Collections.Array GetUsers() => Call("get_users").AsGodotArray();

    public Task<GdkResult> CheckPrivilegeAsync(GdkUser user, int privilege) =>
        CallResultAsync("check_privilege_async", user?.Raw, privilege);

    public Task<GdkResult> ResolvePrivilegeWithUiAsync(GdkUser user, int privilege) =>
        CallResultAsync("resolve_privilege_with_ui_async", user?.Raw, privilege);

    public Task<GdkResult> ResolveIssueWithUiAsync(GdkUser user, string url) =>
        CallResultAsync("resolve_issue_with_ui_async", user?.Raw, url);

    public Task<GdkResult> GetGamerPictureAsync(GdkUser user, string size) =>
        CallResultAsync("get_gamer_picture_async", user?.Raw, size);

    public Task<GdkResult> GetTokenAndSignatureAsync(
        GdkUser user, string httpMethod, string url,
        Godot.Collections.Dictionary headers = null, byte[] body = null, bool forceRefresh = false) =>
        CallResultAsync("get_token_and_signature_async", user?.Raw, httpMethod, url,
            headers ?? new Godot.Collections.Dictionary(), body ?? System.Array.Empty<byte>(), forceRefresh);
}
