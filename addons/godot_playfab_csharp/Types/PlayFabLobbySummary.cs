using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbySummary : PlayFabObject
{
    internal PlayFabLobbySummary(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbySummary From(GodotObject o) => o == null ? null : new PlayFabLobbySummary(o);

    public string LobbyId => GetString("lobby_id");

    public string ConnectionString => GetString("connection_string");

    public Godot.Collections.Dictionary OwnerEntityKey => GetDict("owner_entity_key");

    public int MaxMemberCount => GetInt32("max_member_count");

    public int MemberCount => GetInt32("member_count");

    public Godot.Collections.Dictionary SearchProperties => GetDict("search_properties");

    public Godot.Collections.Dictionary LobbyProperties => GetDict("lobby_properties");
}
