using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>
/// An active user sign-out deferral. Hold it while flushing per-user state
/// (such as a save) in response to a pending sign-out; call
/// <see cref="Release"/> as soon as that work is done.
/// </summary>
public sealed class GdkUserSignOutDeferral : GdkObject
{
    internal GdkUserSignOutDeferral(GodotObject o) : base(o) { }
    public static GdkUserSignOutDeferral From(GodotObject o) => o == null ? null : new GdkUserSignOutDeferral(o);

    public bool IsValid => Call("is_valid").AsBool();
    public void Release() => Call("release");
}
