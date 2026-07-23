using Godot;
using GodotGameInput.Internal;

namespace GodotGameInput;

/// <summary>
/// Wrapper over the native <c>GameInputBinding</c> <see cref="Resource"/>: one
/// row mapping a GameInput source onto a Godot <c>InputMap</c> action. Create
/// new bindings with <see cref="Create"/> or wrap an existing resource with
/// <see cref="From"/>.
/// </summary>
public sealed class GameInputBinding : GameInputObject
{
    internal GameInputBinding(GodotObject o) : base(o)
    {
    }

    public static GameInputBinding From(GodotObject o) => o == null ? null : new GameInputBinding(o);

    /// <summary>Instantiates a fresh native <c>GameInputBinding</c> resource.</summary>
    public static GameInputBinding Create() =>
        From(ClassDB.Instantiate("GameInputBinding").AsGodotObject());

    public StringName Action
    {
        get => Call("get_action").AsStringName();
        set => Call("set_action", value);
    }

    public GameInputDevice.Source Source
    {
        get => (GameInputDevice.Source)Call("get_source").AsInt32();
        set => Call("set_source", (int)value);
    }

    public bool IsAxis
    {
        get => Call("get_is_axis").AsBool();
        set => Call("set_is_axis", value);
    }

    public float AxisThreshold
    {
        get => (float)Call("get_axis_threshold").AsDouble();
        set => Call("set_axis_threshold", value);
    }

    public bool AxisInvert
    {
        get => Call("get_axis_invert").AsBool();
        set => Call("set_axis_invert", value);
    }

    public float Deadzone
    {
        get => (float)Call("get_deadzone").AsDouble();
        set => Call("set_deadzone", value);
    }
}
