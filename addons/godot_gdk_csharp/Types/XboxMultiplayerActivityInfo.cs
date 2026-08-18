using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>A cached multiplayer activity for a user.</summary>
public sealed class XboxMultiplayerActivityInfo : XboxObject
{
    internal XboxMultiplayerActivityInfo(GodotObject o) : base(o) { }
    public static XboxMultiplayerActivityInfo From(GodotObject o) => o == null ? null : new XboxMultiplayerActivityInfo(o);

    public string Xuid => GetString("xuid");
    public string ConnectionString => GetString("connection_string");
    public string JoinRestriction => GetString("join_restriction");
    public int MaxPlayers => GetInt32("max_players");
    public int CurrentPlayers => GetInt32("current_players");
    public string GroupId => GetString("group_id");
    public string Platform => GetString("platform");
}
