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
        _o.Connect("device_association_changed", Callable.From((Variant a0, Variant a1, Variant a2) =>
            DeviceAssociationChanged?.Invoke(a0.AsString(), a1.AsInt64(), a2.AsInt64())));
        _o.Connect("default_audio_endpoint_changed", Callable.From((Variant a0, Variant a1, Variant a2) =>
            DefaultAudioEndpointChanged?.Invoke(a0.AsInt64(), a1.AsInt32(), a2.AsString())));
    }

    /// <summary>Mirrors the native <c>GDKUsers.AudioEndpointKind</c> enum.</summary>
    public enum AudioEndpointKind
    {
        CommunicationRender = 0,
        CommunicationCapture = 1,
    }

    /// <summary>Raised for every user lifecycle event (added/removed/changed).</summary>
    public event Action<GdkUser, string> UserChanged;

    /// <summary>
    /// Raised when the platform re-pairs a device (typically a controller) with
    /// a different user, and once per existing pairing at startup. A local id of
    /// 0 means "no user".
    /// </summary>
    public event Action<string, long, long> DeviceAssociationChanged;

    /// <summary>Raised when a user's default communication audio endpoint changes.</summary>
    public event Action<long, int, string> DefaultAudioEndpointChanged;

    public Task<GdkResult> AddDefaultUserAsync() => CallResultAsync("add_default_user_async");

    public Task<GdkResult> AddUserWithUiAsync() => CallResultAsync("add_user_with_ui_async");

    public Task<GdkResult> AddUserByIdWithUiAsync(string xuid) =>
        CallResultAsync("add_user_by_id_with_ui_async", xuid);

    public GdkUser GetPrimaryUser() => GdkUser.From(Call("get_primary_user").AsGodotObject());

    public Godot.Collections.Array GetUsers() => Call("get_users").AsGodotArray();

    public GdkResult GetMaxUsers() => GdkResult.From(Call("get_max_users").AsGodotObject());

    public bool IsSignOutAvailable() => Call("is_sign_out_available").AsBool();

    public Task<GdkResult> SignOutAsync(GdkUser user) => CallResultAsync("sign_out_async", user?.Raw);

    public GdkResult AcquireSignOutDeferral() =>
        GdkResult.From(Call("acquire_sign_out_deferral").AsGodotObject());

    public GdkResult FindUserByXuid(string xuid) =>
        GdkResult.From(Call("find_user_by_xuid", xuid).AsGodotObject());

    public GdkResult FindUserByLocalId(long localId) =>
        GdkResult.From(Call("find_user_by_local_id", localId).AsGodotObject());

    public GdkResult FindUserForDevice(string deviceId) =>
        GdkResult.From(Call("find_user_for_device", deviceId).AsGodotObject());

    public Task<GdkResult> FindControllerForUserWithUiAsync(GdkUser user) =>
        CallResultAsync("find_controller_for_user_with_ui_async", user?.Raw);

    public Godot.Collections.Array GetDeviceAssociations() => Call("get_device_associations").AsGodotArray();

    public string[] GetDevicesForUser(GdkUser user) =>
        Call("get_devices_for_user", user?.Raw).AsStringArray();

    public GdkResult GetDefaultAudioEndpoint(GdkUser user, AudioEndpointKind kind = AudioEndpointKind.CommunicationRender) =>
        GdkResult.From(Call("get_default_audio_endpoint", user?.Raw, (int)kind).AsGodotObject());

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
