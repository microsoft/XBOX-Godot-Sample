using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.profile</c> — Xbox Live profile lookups.</summary>
public sealed class GdkProfile : GdkServiceBase
{
    internal GdkProfile(GodotObject o) : base(o) { }

    public Task<GdkResult> GetProfileAsync(GdkUser user, string xuid) =>
        CallResultAsync("get_profile_async", user?.Raw, xuid);

    public Task<GdkResult> GetProfilesAsync(GdkUser user, string[] xuids) =>
        CallResultAsync("get_profiles_async", user?.Raw, xuids);

    public Task<GdkResult> GetProfilesForSocialGroupAsync(GdkUser user, string socialGroup) =>
        CallResultAsync("get_profiles_for_social_group_async", user?.Raw, socialGroup);
}
