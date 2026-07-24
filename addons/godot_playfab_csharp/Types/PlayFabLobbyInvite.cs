using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbyInvite : PlayFabObject
{
    internal PlayFabLobbyInvite(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbyInvite From(GodotObject o) => o == null ? null : new PlayFabLobbyInvite(o);

    public string LobbyId => GetString("lobby_id");

    public string ConnectionString => GetString("connection_string");

    public string SenderUserId => GetString("sender_user_id");

    public Godot.Collections.Dictionary SenderEntityKey => GetDict("sender_entity_key");

    public string InviteUri => GetString("invite_uri");

    public Godot.Collections.Dictionary Properties => GetDict("properties");
}
