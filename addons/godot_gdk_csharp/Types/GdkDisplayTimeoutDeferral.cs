using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>
/// An active display-idle-timeout deferral. Hold it to keep the display awake
/// (e.g. during a cutscene); call <see cref="Release"/> to end it.
/// </summary>
public sealed class GdkDisplayTimeoutDeferral : GdkObject
{
    internal GdkDisplayTimeoutDeferral(GodotObject o) : base(o) { }
    public static GdkDisplayTimeoutDeferral From(GodotObject o) => o == null ? null : new GdkDisplayTimeoutDeferral(o);

    public bool IsValid => Call("is_valid").AsBool();
    public void Release() => Call("release");
}
