using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbyConfig : PlayFabObject
{
    internal PlayFabLobbyConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbyConfig From(GodotObject o) => o == null ? null : new PlayFabLobbyConfig(o);

    public const int ACCESSPOLICYPUBLIC = 0;

    public const int ACCESSPOLICYFRIENDS = 1;

    public const int ACCESSPOLICYPRIVATE = 2;

    public const int OWNERMIGRATIONAUTOMATIC = 0;

    public const int OWNERMIGRATIONMANUAL = 1;

    public const int OWNERMIGRATIONNONE = 2;

    public int MaxPlayers => GetInt32("max_players");

    public int AccessPolicy => GetInt32("access_policy");

    public int OwnerMigrationPolicy => GetInt32("owner_migration_policy");

    public Godot.Collections.Dictionary SearchProperties => GetDict("search_properties");

    public Godot.Collections.Dictionary LobbyProperties => GetDict("lobby_properties");

    public Godot.Collections.Dictionary MemberProperties => GetDict("member_properties");

    public bool RestrictInvitesToLobbyOwner => GetBool("restrict_invites_to_lobby_owner");
}
