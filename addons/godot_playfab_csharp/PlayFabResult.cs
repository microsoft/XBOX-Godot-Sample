using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab;

/// <summary>
/// Normalized result of a <c>godot_playfab</c> async operation. Mirrors the native
/// <c>PlayFabResult</c>: a success bit, a stable string <see cref="Code"/>, a
/// human-readable <see cref="Message"/>, and a typed <see cref="Data"/> payload.
/// </summary>
public sealed class PlayFabResult : PlayFabObject
{
    internal PlayFabResult(GodotObject o) : base(o)
    {
    }

    public static PlayFabResult From(GodotObject o) => new PlayFabResult(o);

    internal static PlayFabResult Failed(string code, string message)
    {
        return new PlayFabResult(null) { _syntheticCode = code, _syntheticMessage = message };
    }

    private string _syntheticCode;
    private string _syntheticMessage;

    /// <summary>True when the operation succeeded.</summary>
    public bool Ok => _o != null && GetBool("ok");

    /// <summary>The underlying <c>HRESULT</c> (0 when synthetic/unset).</summary>
    public long HResult => _o == null ? 0 : GetInt("hresult");

    /// <summary>Stable, branchable string error id (empty on success).</summary>
    public string Code => _o == null ? (_syntheticCode ?? string.Empty) : GetString("code");

    /// <summary>Short, human-readable description.</summary>
    public string Message => _o == null ? (_syntheticMessage ?? string.Empty) : GetString("message");

    /// <summary>The raw typed payload as a <see cref="Variant"/>.</summary>
    public Variant Data => _o == null ? default : Get("data");

    /// <summary>The payload as a <see cref="GodotObject"/> (null if not an object).</summary>
    public GodotObject DataObject => _o == null ? null : GetObject("data");

    /// <summary>The payload wrapped as a typed PlayFab wrapper of type <typeparamref name="T"/>.</summary>
    public T DataAs<T>() where T : PlayFabObject => Wrap<T>(DataObject);
}
