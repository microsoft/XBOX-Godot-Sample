using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>
/// A capture metadata session. Add string/double/int events and states while a
/// diagnostic clip is being recorded, then close it.
/// </summary>
public sealed class XboxCaptureMetaData : XboxObject
{
    internal XboxCaptureMetaData(GodotObject o) : base(o) { }
    public static XboxCaptureMetaData From(GodotObject o) => o == null ? null : new XboxCaptureMetaData(o);

    public bool IsValid => Call("is_valid").AsBool();
    public void Close() => Call("close");
    public XboxResult StopAllStates() => XboxResult.From(Call("stop_all_states").AsGodotObject());
    public long GetRemainingStorageBytes() => Call("get_remaining_storage_bytes").AsInt64();

    public XboxResult AddStringEvent(string name, string value, int priority) =>
        XboxResult.From(Call("add_string_event", name, value, priority).AsGodotObject());

    public XboxResult AddDoubleEvent(string name, double value, int priority) =>
        XboxResult.From(Call("add_double_event", name, value, priority).AsGodotObject());

    public XboxResult AddInt32Event(string name, int value, int priority) =>
        XboxResult.From(Call("add_int32_event", name, value, priority).AsGodotObject());

    public XboxResult StartStringState(string name, string value, int priority) =>
        XboxResult.From(Call("start_string_state", name, value, priority).AsGodotObject());

    public XboxResult StartDoubleState(string name, double value, int priority) =>
        XboxResult.From(Call("start_double_state", name, value, priority).AsGodotObject());

    public XboxResult StartInt32State(string name, int value, int priority) =>
        XboxResult.From(Call("start_int32_state", name, value, priority).AsGodotObject());
}
