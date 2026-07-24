using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>A cached multiplayer activity for a user.</summary>
public sealed class GdkMultiplayerActivityInfo : GdkObject
{
    internal GdkMultiplayerActivityInfo(GodotObject o) : base(o) { }
    public static GdkMultiplayerActivityInfo From(GodotObject o) => o == null ? null : new GdkMultiplayerActivityInfo(o);

    public string Xuid => GetString("xuid");
    public string ConnectionString => GetString("connection_string");
    public string JoinRestriction => GetString("join_restriction");
    public int MaxPlayers => GetInt32("max_players");
    public int CurrentPlayers => GetInt32("current_players");
    public string GroupId => GetString("group_id");
    public string Platform => GetString("platform");
}
