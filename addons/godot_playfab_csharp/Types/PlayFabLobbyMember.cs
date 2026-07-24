using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbyMember : PlayFabObject
{
    internal PlayFabLobbyMember(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbyMember From(GodotObject o) => o == null ? null : new PlayFabLobbyMember(o);

    public string UserId => GetString("user_id");

    public Godot.Collections.Dictionary EntityKey => GetDict("entity_key");

    public Godot.Collections.Dictionary Properties => GetDict("properties");

    public bool IsLocal => GetBool("is_local");
}
