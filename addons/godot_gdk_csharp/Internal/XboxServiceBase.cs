using System.Threading.Tasks;
using Godot;

namespace GodotXbox.Internal;

/// <summary>
/// Base class for the <c>GDK.&lt;service&gt;</c> namespace wrappers. Adds the
/// async helper (Signal → <see cref="Task{XboxResult}"/>); the signal subscription
/// helper is inherited from <see cref="XboxObject"/>.
/// </summary>
public abstract class XboxServiceBase : XboxObject
{
    protected XboxServiceBase(GodotObject o) : base(o)
    {
    }

    protected Task<XboxResult> CallResultAsync(string method, params Variant[] args)
    {
        Signal completion = _o.Call(method, args).AsSignal();
        return SignalBridge.AwaitResult(completion);
    }
}
