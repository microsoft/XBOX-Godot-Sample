using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary>
/// <c>GDK.get_events()</c> — XSAPI title telemetry events (PlayStream / Game
/// Analytics). Events are tagged with an optional play-session id so a single
/// gameplay session can be correlated across multiple writes.
/// </summary>
public sealed class XboxEvents : XboxServiceBase
{
    internal XboxEvents(GodotObject o) : base(o) { }

    /// <summary>Optional id correlating events from the same play session.</summary>
    public string PlaySessionId
    {
        get => Call("get_play_session_id").AsString();
        set => Call("set_play_session_id", value);
    }

    public XboxResult WriteEvent(XboxUser user, string eventName,
        Godot.Collections.Dictionary dimensions, Godot.Collections.Dictionary measurements) =>
        XboxResult.From(Call("write_event", user?.Raw, eventName, dimensions, measurements).AsGodotObject());

    public void SetPlaySessionId(string playSessionId) => Call("set_play_session_id", playSessionId);

    public string GetPlaySessionId() => Call("get_play_session_id").AsString();
}
