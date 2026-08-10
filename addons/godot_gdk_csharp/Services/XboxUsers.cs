using System;
using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.users</c> — sign-in, the active user roster, and privileges.</summary>
public sealed class XboxUsers : XboxServiceBase
{
    internal XboxUsers(GodotObject o) : base(o)
    {
        _o.Connect("user_changed", Callable.From((Variant a0, Variant a1) =>
            UserChanged?.Invoke(XboxUser.From(a0.AsGodotObject()), a1.AsString())));
    }

    /// <summary>Raised for every user lifecycle event (added/removed/changed).</summary>
    public event Action<XboxUser, string> UserChanged;

    public Task<XboxResult> AddDefaultUserAsync() => CallResultAsync("add_default_user_async");

    public Task<XboxResult> AddUserWithUiAsync() => CallResultAsync("add_user_with_ui_async");

    public XboxUser GetPrimaryUser() => XboxUser.From(Call("get_primary_user").AsGodotObject());

    public Godot.Collections.Array GetUsers() => Call("get_users").AsGodotArray();

    public Task<XboxResult> CheckPrivilegeAsync(XboxUser user, int privilege) =>
        CallResultAsync("check_privilege_async", user?.Raw, privilege);

    public Task<XboxResult> ResolvePrivilegeWithUiAsync(XboxUser user, int privilege) =>
        CallResultAsync("resolve_privilege_with_ui_async", user?.Raw, privilege);

    public Task<XboxResult> ResolveIssueWithUiAsync(XboxUser user, string url) =>
        CallResultAsync("resolve_issue_with_ui_async", user?.Raw, url);

    public Task<XboxResult> GetGamerPictureAsync(XboxUser user, string size) =>
        CallResultAsync("get_gamer_picture_async", user?.Raw, size);

    public Task<XboxResult> GetTokenAndSignatureAsync(
        XboxUser user, string httpMethod, string url,
        Godot.Collections.Dictionary headers = null, byte[] body = null, bool forceRefresh = false) =>
        CallResultAsync("get_token_and_signature_async", user?.Raw, httpMethod, url,
            headers ?? new Godot.Collections.Dictionary(), body ?? System.Array.Empty<byte>(), forceRefresh);
}
