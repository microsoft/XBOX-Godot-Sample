using System.Threading.Tasks;
using Godot;

namespace GodotPlayFab.Internal;

/// <summary>
/// Bridges the addon's one-shot completion <see cref="Signal"/> values into
/// C# <see cref="Task"/>s. Every <c>*_async</c> method returns a Signal that
/// fires exactly once on the main thread during the per-frame dispatch tick;
/// this awaits that Signal generically via its owner and name.
/// </summary>
internal static class SignalBridge
{
    public static async Task<PlayFabResult> AwaitResult(Signal completion)
    {
        GodotObject owner = completion.Owner;
        if (owner == null)
        {
            return PlayFabResult.Failed("signal_owner_missing", "Async signal had no owner to await.");
        }

        Variant[] payload = await owner.ToSignal(owner, completion.Name);
        GodotObject result = payload.Length > 0 ? payload[0].AsGodotObject() : null;
        return PlayFabResult.From(result);
    }
}
