using Godot;

namespace GodotPlayFab.Internal;

/// <summary>
/// Base class for the <c>PlayFab.&lt;service&gt;</c> namespace wrappers. The async
/// helper and signal subscription helper are inherited from
/// <see cref="PlayFabObject"/>; this type exists to mark service-namespace
/// wrappers distinctly from value-type wrappers.
/// </summary>
public abstract class PlayFabServiceBase : PlayFabObject
{
    protected PlayFabServiceBase(GodotObject o) : base(o)
    {
    }
}
