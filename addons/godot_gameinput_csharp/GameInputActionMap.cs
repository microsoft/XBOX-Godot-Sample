using System.Collections.Generic;
using Godot;
using GodotGameInput.Internal;

namespace GodotGameInput;

/// <summary>
/// Wrapper over the native <c>GameInputActionMap</c> <see cref="Resource"/>: an
/// ordered list of <see cref="GameInputBinding"/> rows consumed by a
/// <see cref="GameInputMapper"/>. Create with <see cref="Create"/> or wrap an
/// existing resource (e.g. from <c>ResourceLoader</c>) with <see cref="From"/>.
/// </summary>
public sealed class GameInputActionMap : GameInputObject
{
    internal GameInputActionMap(GodotObject o) : base(o)
    {
    }

    public static GameInputActionMap From(GodotObject o) => o == null ? null : new GameInputActionMap(o);

    /// <summary>Instantiates a fresh native <c>GameInputActionMap</c> resource.</summary>
    public static GameInputActionMap Create() =>
        From(ClassDB.Instantiate("GameInputActionMap").AsGodotObject());

    public int BindingCount => Call("get_binding_count").AsInt32();

    public IReadOnlyList<GameInputBinding> GetBindings()
    {
        var result = new List<GameInputBinding>();
        Godot.Collections.Array bindings = Call("get_bindings").AsGodotArray();
        foreach (Variant binding in bindings)
        {
            GameInputBinding wrapped = GameInputBinding.From(binding.AsGodotObject());
            if (wrapped != null)
            {
                result.Add(wrapped);
            }
        }

        return result;
    }

    public GameInputBinding GetBinding(int index) =>
        GameInputBinding.From(Call("get_binding", index).AsGodotObject());

    public void AddBinding(GameInputBinding binding) => Call("add_binding", binding?.Raw);

    public void Clear() => Call("clear");

    public void SetBindings(IEnumerable<GameInputBinding> bindings)
    {
        var array = new Godot.Collections.Array();
        foreach (GameInputBinding binding in bindings)
        {
            if (binding?.Raw != null)
            {
                array.Add(binding.Raw);
            }
        }

        Call("set_bindings", array);
    }
}
