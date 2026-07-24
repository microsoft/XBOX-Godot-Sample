using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyMember : PlayFabObject
{
    internal PlayFabPartyMember(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyMember From(GodotObject o) => o == null ? null : new PlayFabPartyMember(o);

    public int PeerId => GetInt32("peer_id");

    public Godot.Collections.Dictionary EntityKey => GetDict("entity_key");

    public PlayFabUser User => PlayFabUser.From(GetObject("user"));

    public bool IsLocal => GetBool("is_local");

    public int GetPeerId() =>
        Call("get_peer_id").AsInt32();

    public Godot.Collections.Dictionary GetEntityKey() =>
        Call("get_entity_key").AsGodotDictionary();

    public PlayFabUser GetUser() =>
        PlayFabUser.From(Call("get_user").AsGodotObject());

    public bool IsLocalMember() =>
        Call("is_local_member").AsBool();
}
