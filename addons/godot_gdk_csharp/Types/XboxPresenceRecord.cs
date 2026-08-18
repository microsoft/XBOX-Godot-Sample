using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>A cached presence record for a single Xbox user.</summary>
public sealed class XboxPresenceRecord : XboxObject
{
    internal XboxPresenceRecord(GodotObject o) : base(o) { }
    public static XboxPresenceRecord From(GodotObject o) => o == null ? null : new XboxPresenceRecord(o);

    public string Xuid => GetString("xuid");
    public int UserState => GetInt32("user_state");
    public string UserStateName => Call("get_user_state_name").AsString();
    public bool IsOnline => Call("is_online").AsBool();
    public Godot.Collections.Array TitleRecords => GetArray("title_records");
}
