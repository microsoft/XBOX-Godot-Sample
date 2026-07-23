using Godot;
using GodotGameInput.Internal;

namespace GodotGameInput;

/// <summary>
/// Weak handle to a single GameInput device, mirroring the native
/// <c>GameInputDevice</c>. Stores only the session-local device id; all methods
/// resolve through the <see cref="GameInput"/> singleton and soft-fail with safe
/// defaults if the device has disconnected.
/// </summary>
public sealed class GameInputDevice : GameInputObject
{
    internal GameInputDevice(GodotObject o) : base(o)
    {
    }

    public static GameInputDevice From(GodotObject o) => o == null ? null : new GameInputDevice(o);

    public long DeviceId => Call("get_device_id").AsInt64();
    public string DisplayName => Call("get_display_name").AsString();
    public int KindMask => Call("get_kind_mask").AsInt32();
    public bool IsConnected => Call("is_connected").AsBool();
    public bool SupportsVibration => Call("supports_vibration").AsBool();
    public bool SupportsHaptics => Call("supports_haptics").AsBool();
    public Godot.Collections.Dictionary GetDeviceInfo() => Call("get_device_info").AsGodotDictionary();

    /// <summary>Translates a <see cref="Button"/> flag into a <see cref="Source"/> value.</summary>
    public int ButtonToSource(Button button) => Call("button_to_source", (int)button).AsInt32();

    /// <summary>Translates an <see cref="Axis"/> index into a <see cref="Source"/> value.</summary>
    public int AxisToSource(Axis axis) => Call("axis_to_source", (int)axis).AsInt32();

    /// <summary>Gamepad button flags (bitfield), matching native <c>GameInputDevice.Button</c>.</summary>
    public enum Button
    {
        None = 0,
        Menu = 1,
        View = 2,
        A = 4,
        B = 8,
        X = 16,
        Y = 32,
        DpadUp = 64,
        DpadDown = 128,
        DpadLeft = 256,
        DpadRight = 512,
        LeftShoulder = 1024,
        RightShoulder = 2048,
        LeftThumb = 4096,
        RightThumb = 8192,
    }

    /// <summary>Gamepad axis indices, matching native <c>GameInputDevice.Axis</c>.</summary>
    public enum Axis
    {
        LeftX = 0,
        LeftY = 1,
        RightX = 2,
        RightY = 3,
        LeftTrigger = 4,
        RightTrigger = 5,
    }

    /// <summary>
    /// Combined button/axis source namespace used by <see cref="GameInputBinding"/>,
    /// matching native <c>GameInputDevice.Source</c> (buttons 0-13, axes 100-105).
    /// </summary>
    public enum Source
    {
        BtnMenu = 0,
        BtnView = 1,
        BtnA = 2,
        BtnB = 3,
        BtnX = 4,
        BtnY = 5,
        BtnDpadUp = 6,
        BtnDpadDown = 7,
        BtnDpadLeft = 8,
        BtnDpadRight = 9,
        BtnLeftShoulder = 10,
        BtnRightShoulder = 11,
        BtnLeftThumb = 12,
        BtnRightThumb = 13,
        AxisLeftX = 100,
        AxisLeftY = 101,
        AxisRightX = 102,
        AxisRightY = 103,
        AxisLeftTrigger = 104,
        AxisRightTrigger = 105,
    }
}
