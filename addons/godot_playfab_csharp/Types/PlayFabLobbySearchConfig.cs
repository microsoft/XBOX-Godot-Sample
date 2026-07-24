using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbySearchConfig : PlayFabObject
{
    internal PlayFabLobbySearchConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbySearchConfig From(GodotObject o) => o == null ? null : new PlayFabLobbySearchConfig(o);

    public string Filter => GetString("filter");

    public string OrderBy => GetString("order_by");

    public int MaxResults => GetInt32("max_results");
}
