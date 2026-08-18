using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Services;

/// <summary><c>GDK.display</c> — HDR mode probe/enable and idle-timeout deferrals.</summary>
public sealed class XboxDisplay : XboxServiceBase
{
    internal XboxDisplay(GodotObject o) : base(o) { }

    public XboxResult TryEnableHdrMode(int preference) =>
        XboxResult.From(Call("try_enable_hdr_mode", preference).AsGodotObject());

    public XboxResult AcquireTimeoutDeferral() =>
        XboxResult.From(Call("acquire_timeout_deferral").AsGodotObject());
}
