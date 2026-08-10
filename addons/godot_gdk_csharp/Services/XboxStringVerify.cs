using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.string_verify</c> — text string verification.</summary>
public sealed class XboxStringVerify : XboxServiceBase
{
    internal XboxStringVerify(GodotObject o) : base(o) { }

    public Task<XboxResult> VerifyStringAsync(XboxUser user, string text) =>
        CallResultAsync("verify_string_async", user?.Raw, text);

    public Task<XboxResult> VerifyStringsAsync(XboxUser user, string[] strings) =>
        CallResultAsync("verify_strings_async", user?.Raw, strings);
}
