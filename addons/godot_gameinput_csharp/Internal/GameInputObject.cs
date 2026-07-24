using Godot;

namespace GodotGameInput.Internal;

/// <summary>
/// Base class for typed C# wrappers over native <c>godot_gameinput</c>
/// objects. Holds the underlying <see cref="GodotObject"/> and reads through it
/// on demand; wrappers never copy native state.
/// </summary>
public abstract class GameInputObject
{
    protected readonly GodotObject _o;

    protected GameInputObject(GodotObject o)
    {
        _o = o;
    }

    /// <summary>The underlying native object, for advanced/interop use.</summary>
    public GodotObject Raw => _o;

    /// <summary>True when the wrapper holds a live native object.</summary>
    public bool IsLive => _o != null && GodotObject.IsInstanceValid(_o);

    protected Variant Get(string name) => _o.Get(name);
    protected Variant Call(string method, params Variant[] args) => _o.Call(method, args);
    protected void Set(string name, Variant value) => _o.Set(name, value);
}
