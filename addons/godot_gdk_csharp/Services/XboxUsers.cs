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
        _o.Connect("device_association_changed", Callable.From((Variant a0, Variant a1, Variant a2) =>
            DeviceAssociationChanged?.Invoke(a0.AsString(), a1.AsInt64(), a2.AsInt64())));
        _o.Connect("default_audio_endpoint_changed", Callable.From((Variant a0, Variant a1, Variant a2) =>
            DefaultAudioEndpointChanged?.Invoke(a0.AsInt64(), a1.AsInt32(), a2.AsString())));
    }

    /// <summary>Mirrors the native <c>XboxUsers.AudioEndpointKind</c> enum.</summary>
    public enum AudioEndpointKind
    {
        CommunicationRender = 0,
        CommunicationCapture = 1,
    }

    /// <summary>Raised for every user lifecycle event (added/removed/changed).</summary>
    public event Action<XboxUser, string> UserChanged;

    /// <summary>
    /// Raised when the platform re-pairs a device (typically a controller) with
    /// a different user, and once per existing pairing at startup. A local id of
    /// 0 means "no user".
    /// </summary>
    public event Action<string, long, long> DeviceAssociationChanged;

    /// <summary>Raised when a user's default communication audio endpoint changes.</summary>
    public event Action<long, int, string> DefaultAudioEndpointChanged;

    public Task<XboxResult> AddDefaultUserAsync() => CallResultAsync("add_default_user_async");

    public Task<XboxResult> AddUserWithUiAsync() => CallResultAsync("add_user_with_ui_async");

    public Task<XboxResult> AddUserByIdWithUiAsync(string xuid) =>
        CallResultAsync("add_user_by_id_with_ui_async", xuid);

    public XboxUser GetPrimaryUser() => XboxUser.From(Call("get_primary_user").AsGodotObject());

    public Godot.Collections.Array GetUsers() => Call("get_users").AsGodotArray();

    public XboxResult GetMaxUsers() => XboxResult.From(Call("get_max_users").AsGodotObject());

    public bool IsSignOutAvailable() => Call("is_sign_out_available").AsBool();

    public Task<XboxResult> SignOutAsync(XboxUser user) => CallResultAsync("sign_out_async", user?.Raw);

    public XboxResult AcquireSignOutDeferral() =>
        XboxResult.From(Call("acquire_sign_out_deferral").AsGodotObject());

    public XboxResult FindUserByXuid(string xuid) =>
        XboxResult.From(Call("find_user_by_xuid", xuid).AsGodotObject());

    public XboxResult FindUserByLocalId(long localId) =>
        XboxResult.From(Call("find_user_by_local_id", localId).AsGodotObject());

    public XboxResult FindUserForDevice(string deviceId) =>
        XboxResult.From(Call("find_user_for_device", deviceId).AsGodotObject());

    public Task<XboxResult> FindControllerForUserWithUiAsync(XboxUser user) =>
        CallResultAsync("find_controller_for_user_with_ui_async", user?.Raw);

    public Godot.Collections.Array GetDeviceAssociations() => Call("get_device_associations").AsGodotArray();

    public string[] GetDevicesForUser(XboxUser user) =>
        Call("get_devices_for_user", user?.Raw).AsStringArray();

    public XboxResult GetDefaultAudioEndpoint(XboxUser user, AudioEndpointKind kind = AudioEndpointKind.CommunicationRender) =>
        XboxResult.From(Call("get_default_audio_endpoint", user?.Raw, (int)kind).AsGodotObject());

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
