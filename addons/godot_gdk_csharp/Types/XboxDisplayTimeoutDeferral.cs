using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>
/// An active display-idle-timeout deferral. Hold it to keep the display awake
/// (e.g. during a cutscene); call <see cref="Release"/> to end it.
/// </summary>
public sealed class XboxDisplayTimeoutDeferral : XboxObject
{
    internal XboxDisplayTimeoutDeferral(GodotObject o) : base(o) { }
    public static XboxDisplayTimeoutDeferral From(GodotObject o) => o == null ? null : new XboxDisplayTimeoutDeferral(o);

    public bool IsValid => Call("is_valid").AsBool();
    public void Release() => Call("release");
}
