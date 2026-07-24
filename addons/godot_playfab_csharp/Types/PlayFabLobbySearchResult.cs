using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbySearchResult : PlayFabObject
{
    internal PlayFabLobbySearchResult(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbySearchResult From(GodotObject o) => o == null ? null : new PlayFabLobbySearchResult(o);

    public Godot.Collections.Array Lobbies => GetArray("lobbies");

    public string ContinuationToken => GetString("continuation_token");
}
