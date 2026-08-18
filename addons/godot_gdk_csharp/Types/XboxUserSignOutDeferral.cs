using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>
/// An active user sign-out deferral. Hold it while flushing per-user state
/// (such as a save) in response to a pending sign-out; call
/// <see cref="Release"/> as soon as that work is done.
/// </summary>
public sealed class XboxUserSignOutDeferral : XboxObject
{
    internal XboxUserSignOutDeferral(GodotObject o) : base(o) { }
    public static XboxUserSignOutDeferral From(GodotObject o) => o == null ? null : new XboxUserSignOutDeferral(o);

    public bool IsValid => Call("is_valid").AsBool();
    public void Release() => Call("release");
}
