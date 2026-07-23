using Godot;
using GodotGameInput.Internal;

namespace GodotGameInput;

/// <summary>
/// Immutable per-frame snapshot of a gamepad's state, mirroring the native
/// <c>GameInputReading</c>. Holds both current and previous poll state so edge
/// queries return correct transitions.
/// </summary>
public sealed class GameInputReading : GameInputObject
{
    internal GameInputReading(GodotObject o) : base(o)
    {
    }

    public static GameInputReading From(GodotObject o) => o == null ? null : new GameInputReading(o);

    public bool IsButtonDown(GameInputDevice.Button button) => Call("is_button_down", (int)button).AsBool();

    public bool IsButtonDown(int buttonMask) => Call("is_button_down", buttonMask).AsBool();

    public bool WasButtonPressed(GameInputDevice.Button button) =>
        Call("was_button_pressed", (int)button).AsBool();

    public bool WasButtonReleased(GameInputDevice.Button button) =>
        Call("was_button_released", (int)button).AsBool();

    public float GetAxis(GameInputDevice.Axis axis) => (float)Call("get_axis", (int)axis).AsDouble();

    public int GetButtonsMask() => Call("get_buttons_mask").AsInt32();

    public long GetTimestamp() => Call("get_timestamp").AsInt64();
}
