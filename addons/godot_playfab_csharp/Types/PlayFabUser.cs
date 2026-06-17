using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabUser : PlayFabObject
{
    internal PlayFabUser(GodotObject o) : base(o)
    {
    }

    public static PlayFabUser From(GodotObject o) => o == null ? null : new PlayFabUser(o);

    public int LocalId => GetInt32("local_id");

    public string CustomId => GetString("custom_id");

    public Godot.Collections.Dictionary EntityKey => GetDict("entity_key");

    public bool HasLocalUserHandle => GetBool("has_local_user_handle");

    public int GetLocalId() =>
        Call("get_local_id").AsInt32();

    public string GetCustomId() =>
        Call("get_custom_id").AsString();

    public Godot.Collections.Dictionary GetEntityKey() =>
        Call("get_entity_key").AsGodotDictionary();

}
