using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabMatchTicketStateChange : PlayFabObject
{
    internal PlayFabMatchTicketStateChange(GodotObject o) : base(o)
    {
    }

    public static PlayFabMatchTicketStateChange From(GodotObject o) => o == null ? null : new PlayFabMatchTicketStateChange(o);

    public int Kind => GetInt32("kind");

    public PlayFabMatchTicket Ticket => PlayFabMatchTicket.From(GetObject("ticket"));

    public PlayFabResult Result => PlayFabResult.From(GetObject("result"));

    public int Status => GetInt32("status");

    public string MatchId => GetString("match_id");

    public string ArrangedLobbyConnectionString => GetString("arranged_lobby_connection_string");
}
