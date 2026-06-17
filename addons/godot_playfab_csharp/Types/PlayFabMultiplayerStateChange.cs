using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabMultiplayerStateChange : PlayFabObject
{
    internal PlayFabMultiplayerStateChange(GodotObject o) : base(o)
    {
    }

    public static PlayFabMultiplayerStateChange From(GodotObject o) => o == null ? null : new PlayFabMultiplayerStateChange(o);

    public int Kind => GetInt32("kind");

    public PlayFabLobby Lobby => PlayFabLobby.From(GetObject("lobby"));

    public PlayFabMatchTicket Ticket => PlayFabMatchTicket.From(GetObject("ticket"));

    public PlayFabResult Result => PlayFabResult.From(GetObject("result"));

    public Godot.Collections.Dictionary Properties => GetDict("properties");
}
