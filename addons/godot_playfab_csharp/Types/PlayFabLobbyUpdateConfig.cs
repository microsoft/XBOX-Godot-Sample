using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbyUpdateConfig : PlayFabObject
{
    internal PlayFabLobbyUpdateConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbyUpdateConfig From(GodotObject o) => o == null ? null : new PlayFabLobbyUpdateConfig(o);

    public const int ACCESSPOLICYPUBLIC = 0;

    public const int ACCESSPOLICYFRIENDS = 1;

    public const int ACCESSPOLICYPRIVATE = 2;

    public const int MEMBERSHIPLOCKUNLOCKED = 0;

    public const int MEMBERSHIPLOCKLOCKED = 1;

    public int MembershipLock => GetInt32("membership_lock");

    public int AccessPolicy => GetInt32("access_policy");

    public int MaxMemberCount => GetInt32("max_member_count");

    public bool RestrictInvitesToLobbyOwner => GetBool("restrict_invites_to_lobby_owner");

    public Godot.Collections.Dictionary NewOwnerEntityKey => GetDict("new_owner_entity_key");

    public Godot.Collections.Dictionary SearchProperties => GetDict("search_properties");

    public Godot.Collections.Dictionary LobbyProperties => GetDict("lobby_properties");

    public bool IsEmpty() => Call("is_empty").AsBool();

    public bool HasMembershipLock() => Call("has_membership_lock").AsBool();

    public void ClearMembershipLock() => Call("clear_membership_lock");

    public bool HasAccessPolicy() => Call("has_access_policy").AsBool();

    public void ClearAccessPolicy() => Call("clear_access_policy");

    public bool HasMaxMemberCount() => Call("has_max_member_count").AsBool();

    public void ClearMaxMemberCount() => Call("clear_max_member_count");

    public bool HasRestrictInvitesToLobbyOwner() => Call("has_restrict_invites_to_lobby_owner").AsBool();

    public void ClearRestrictInvitesToLobbyOwner() => Call("clear_restrict_invites_to_lobby_owner");

    public bool HasNewOwnerEntityKey() => Call("has_new_owner_entity_key").AsBool();

    public void ClearNewOwnerEntityKey() => Call("clear_new_owner_entity_key");

    public bool HasSearchProperties() => Call("has_search_properties").AsBool();

    public void ClearSearchProperties() => Call("clear_search_properties");

    public bool HasLobbyProperties() => Call("has_lobby_properties").AsBool();

    public void ClearLobbyProperties() => Call("clear_lobby_properties");
}
