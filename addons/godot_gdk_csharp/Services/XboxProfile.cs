using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.profile</c> — Xbox Live profile lookups.</summary>
public sealed class XboxProfile : XboxServiceBase
{
    internal XboxProfile(GodotObject o) : base(o) { }

    public Task<XboxResult> GetProfileAsync(XboxUser user, string xuid) =>
        CallResultAsync("get_profile_async", user?.Raw, xuid);

    public Task<XboxResult> GetProfilesAsync(XboxUser user, string[] xuids) =>
        CallResultAsync("get_profiles_async", user?.Raw, xuids);

    public Task<XboxResult> GetProfilesForSocialGroupAsync(XboxUser user, string socialGroup) =>
        CallResultAsync("get_profiles_for_social_group_async", user?.Raw, socialGroup);
}
