using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>A cached presence record for a single Xbox user.</summary>
public sealed class GdkPresenceRecord : GdkObject
{
    internal GdkPresenceRecord(GodotObject o) : base(o) { }
    public static GdkPresenceRecord From(GodotObject o) => o == null ? null : new GdkPresenceRecord(o);

    public string Xuid => GetString("xuid");
    public int UserState => GetInt32("user_state");
    public string UserStateName => Call("get_user_state_name").AsString();
    public bool IsOnline => Call("is_online").AsBool();
    public Godot.Collections.Array TitleRecords => GetArray("title_records");
}
