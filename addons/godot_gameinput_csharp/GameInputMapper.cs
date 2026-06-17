using System;
using Godot;
using GodotGameInput.Internal;

namespace GodotGameInput;

/// <summary>
/// Wrapper over the native <c>GameInputMapper</c> <see cref="Node"/>, which
/// bridges a <see cref="GameInputActionMap"/> onto Godot's <c>InputMap</c> each
/// frame. Instantiate with <see cref="Create"/> and add <see cref="Raw"/> to the
/// scene tree, or wrap an existing node with <see cref="From"/>.
/// </summary>
public sealed class GameInputMapper : GameInputObject
{
    internal GameInputMapper(GodotObject o) : base(o)
    {
    }

    public static GameInputMapper From(GodotObject o) => o == null ? null : new GameInputMapper(o);

    /// <summary>Instantiates a fresh native <c>GameInputMapper</c> node.</summary>
    public static GameInputMapper Create() =>
        From(ClassDB.Instantiate("GameInputMapper").AsGodotObject());

    /// <summary>The underlying node, ready to add to the scene tree.</summary>
    public Node Node => _o as Node;

    /// <summary>Target-kind bit flags, matching native <c>GameInputMapper.KindFlags</c>.</summary>
    [Flags]
    public enum KindFlags
    {
        Gamepad = 1,
        Keyboard = 2,
        Mouse = 4,
    }

    public GameInputActionMap ActionMap
    {
        get => GameInputActionMap.From(Call("get_action_map").AsGodotObject());
        set => Call("set_action_map", value?.Raw);
    }

    public KindFlags TargetKindMask
    {
        get => (KindFlags)Call("get_target_kind_mask").AsInt32();
        set => Call("set_target_kind_mask", (int)value);
    }

    public long TargetDeviceId
    {
        get => Call("get_target_device_id").AsInt64();
        set => Call("set_target_device_id", value);
    }

    public int ActiveBindingCount => Call("get_active_binding_count").AsInt32();
}
