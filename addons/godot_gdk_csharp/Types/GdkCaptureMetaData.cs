using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>
/// A capture metadata session. Add string/double/int events and states while a
/// diagnostic clip is being recorded, then close it.
/// </summary>
public sealed class GdkCaptureMetaData : GdkObject
{
    internal GdkCaptureMetaData(GodotObject o) : base(o) { }
    public static GdkCaptureMetaData From(GodotObject o) => o == null ? null : new GdkCaptureMetaData(o);

    public bool IsValid => Call("is_valid").AsBool();
    public void Close() => Call("close");
    public GdkResult StopAllStates() => GdkResult.From(Call("stop_all_states").AsGodotObject());
    public long GetRemainingStorageBytes() => Call("get_remaining_storage_bytes").AsInt64();

    public GdkResult AddStringEvent(string name, string value, int priority) =>
        GdkResult.From(Call("add_string_event", name, value, priority).AsGodotObject());

    public GdkResult AddDoubleEvent(string name, double value, int priority) =>
        GdkResult.From(Call("add_double_event", name, value, priority).AsGodotObject());

    public GdkResult AddInt32Event(string name, int value, int priority) =>
        GdkResult.From(Call("add_int32_event", name, value, priority).AsGodotObject());

    public GdkResult StartStringState(string name, string value, int priority) =>
        GdkResult.From(Call("start_string_state", name, value, priority).AsGodotObject());

    public GdkResult StartDoubleState(string name, double value, int priority) =>
        GdkResult.From(Call("start_double_state", name, value, priority).AsGodotObject());

    public GdkResult StartInt32State(string name, int value, int priority) =>
        GdkResult.From(Call("start_int32_state", name, value, priority).AsGodotObject());
}
