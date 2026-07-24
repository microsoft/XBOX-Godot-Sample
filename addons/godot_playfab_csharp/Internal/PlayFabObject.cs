using System;
using System.Reflection;
using System.Threading.Tasks;
using Godot;

namespace GodotPlayFab.Internal;

/// <summary>
/// Base class for every typed C# wrapper over a native <c>godot_playfab</c>
/// <see cref="GodotObject"/>. Wrappers never copy state — they hold the
/// underlying object and read through it on demand.
/// </summary>
public abstract class PlayFabObject
{
    protected readonly GodotObject _o;

    protected PlayFabObject(GodotObject o)
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

    protected Task<PlayFabResult> CallResultAsync(string method, params Variant[] args)
    {
        Signal completion = _o.Call(method, args).AsSignal();
        return SignalBridge.AwaitResult(completion);
    }

    /// <summary>
    /// Subscribes <paramref name="handler"/> to a C# event, connecting the backing
    /// native <paramref name="signal"/> only on the first subscription. Wrappers are
    /// created fresh on every <c>From()</c> / property read, so connecting eagerly in
    /// the constructor would leak the wrapper (the native signal retains the Callable
    /// closure) and duplicate delivery when the same native object is wrapped twice.
    /// Connecting lazily keeps unsubscribed wrappers cheap and collectable.
    /// </summary>
    protected void AddSignal<TDelegate>(
        ref TDelegate backing, TDelegate handler, string signal, ref Callable callable, Func<Callable> makeCallable)
        where TDelegate : Delegate
    {
        bool wasEmpty = backing is null;
        backing = (TDelegate)Delegate.Combine(backing, handler);
        if (wasEmpty && backing is not null && IsLive)
        {
            callable = makeCallable();
            _o.Connect(signal, callable);
        }
    }

    /// <summary>
    /// Unsubscribes <paramref name="handler"/>, disconnecting the backing native
    /// <paramref name="signal"/> once the last handler is removed. Pairs with
    /// <see cref="AddSignal{TDelegate}"/>.
    /// </summary>
    protected void RemoveSignal<TDelegate>(
        ref TDelegate backing, TDelegate handler, string signal, ref Callable callable)
        where TDelegate : Delegate
    {
        if (backing is null)
        {
            return;
        }

        backing = (TDelegate)Delegate.Remove(backing, handler);
        if (backing is null && IsLive && _o.IsConnected(signal, callable))
        {
            _o.Disconnect(signal, callable);
            callable = default;
        }
    }

    /// <summary>
    /// Reflection helper used by result payload typing: constructs a wrapper of
    /// type <typeparamref name="T"/> around <paramref name="o"/> via its
    /// non-public <c>(GodotObject)</c> constructor.
    /// </summary>
    internal static T Wrap<T>(GodotObject o) where T : PlayFabObject
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
