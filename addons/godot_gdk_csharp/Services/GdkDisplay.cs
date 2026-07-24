using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Services;

/// <summary><c>GDK.display</c> — HDR mode probe/enable and idle-timeout deferrals.</summary>
public sealed class GdkDisplay : GdkServiceBase
{
    internal GdkDisplay(GodotObject o) : base(o) { }

    public GdkResult TryEnableHdrMode(int preference) =>
        GdkResult.From(Call("try_enable_hdr_mode", preference).AsGodotObject());

    public GdkResult AcquireTimeoutDeferral() =>
        GdkResult.From(Call("acquire_timeout_deferral").AsGodotObject());
}
