using System.Threading.Tasks;
using Godot;

namespace GodotGdk.Internal;

/// <summary>
/// Base class for the <c>GDK.&lt;service&gt;</c> namespace wrappers. Adds the
/// async helper (Signal → <see cref="Task{GdkResult}"/>); the signal subscription
/// helper is inherited from <see cref="GdkObject"/>.
/// </summary>
public abstract class GdkServiceBase : GdkObject
{
    protected GdkServiceBase(GodotObject o) : base(o)
    {
    }

    protected Task<GdkResult> CallResultAsync(string method, params Variant[] args)
    {
        Signal completion = _o.Call(method, args).AsSignal();
        return SignalBridge.AwaitResult(completion);
    }
}
