using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbyJoinConfig : PlayFabObject
{
    internal PlayFabLobbyJoinConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbyJoinConfig From(GodotObject o) => o == null ? null : new PlayFabLobbyJoinConfig(o);

    public Godot.Collections.Dictionary MemberProperties => GetDict("member_properties");

    // Arranged-lobby initialization. These are read only by
    // join_arranged_lobby_async; ordinary joins ignore them. Unset or cleared
    // fields fall back to 8 / Private / Automatic rather than being omitted,
    // because the native PFLobbyArrangedJoinConfiguration has no optional
    // fields. Assign through Raw.Set, per the read-only facade convention.
    public int MaxMemberCount => GetInt32("max_member_count");

    public int AccessPolicy => GetInt32("access_policy");

    public int OwnerMigrationPolicy => GetInt32("owner_migration_policy");

    public bool HasMaxMemberCount() => Call("has_max_member_count").AsBool();

    public void ClearMaxMemberCount() => Call("clear_max_member_count");

    public bool HasAccessPolicy() => Call("has_access_policy").AsBool();

    public void ClearAccessPolicy() => Call("clear_access_policy");

    public bool HasOwnerMigrationPolicy() => Call("has_owner_migration_policy").AsBool();

    public void ClearOwnerMigrationPolicy() => Call("clear_owner_migration_policy");
}
