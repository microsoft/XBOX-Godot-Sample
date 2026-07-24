using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.string_verify</c> — text string verification.</summary>
public sealed class GdkStringVerify : GdkServiceBase
{
    internal GdkStringVerify(GodotObject o) : base(o) { }

    public Task<GdkResult> VerifyStringAsync(GdkUser user, string text) =>
        CallResultAsync("verify_string_async", user?.Raw, text);

    public Task<GdkResult> VerifyStringsAsync(GdkUser user, string[] strings) =>
        CallResultAsync("verify_strings_async", user?.Raw, strings);
}
