using System;
using System.Reflection;
using Godot;

namespace GodotGdk.Internal;

/// <summary>
/// Base class for every typed C# wrapper over a native <c>godot_gdk</c>
/// <see cref="GodotObject"/>. Wrappers never copy state — they hold the
/// underlying object and read through it on demand.
/// </summary>
public abstract class GdkObject
{
    protected readonly GodotObject _o;

    protected GdkObject(GodotObject o)
    {
        _o = o;
    }

    /// <summary>The underlying native object, for advanced/interop use.</summary>
    public GodotObject Raw => _o;

    /// <summary>True when the wrapper holds a live native object.</summary>
    public bool IsLive => _o != null && GodotObject.IsInstanceValid(_o);

    protected Variant Get(string name) => _o.Get(name);
    protected string GetString(string name) => _o.Get(name).AsString();
    protected long GetInt(string name) => _o.Get(name).AsInt64();
    protected int GetInt32(string name) => _o.Get(name).AsInt32();
    protected double GetDouble(string name) => _o.Get(name).AsDouble();
    protected bool GetBool(string name) => _o.Get(name).AsBool();
    protected GodotObject GetObject(string name) => _o.Get(name).AsGodotObject();
    protected Godot.Collections.Array GetArray(string name) => _o.Get(name).AsGodotArray();
    protected Godot.Collections.Dictionary GetDict(string name) => _o.Get(name).AsGodotDictionary();
    protected Color GetColor(string name) => _o.Get(name).AsColor();

    protected Variant Call(string method, params Variant[] args) => _o.Call(method, args);

    /// <summary>
    /// Reflection helper used by result payload typing: constructs a wrapper of
    /// type <typeparamref name="T"/> around <paramref name="o"/> via its
    /// non-public <c>(GodotObject)</c> constructor.
    /// </summary>
    internal static T Wrap<T>(GodotObject o) where T : GdkObject
    {
        if (o == null)
        {
            return null;
        }

        return (T)Activator.CreateInstance(
            typeof(T),
            BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public,
            binder: null,
            args: new object[] { o },
            culture: null);
    }
}
