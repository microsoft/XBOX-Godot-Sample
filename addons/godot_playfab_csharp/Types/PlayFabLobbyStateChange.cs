using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabLobbyStateChange : PlayFabObject
{
    internal PlayFabLobbyStateChange(GodotObject o) : base(o)
    {
    }

    public static PlayFabLobbyStateChange From(GodotObject o) => o == null ? null : new PlayFabLobbyStateChange(o);

    public int Kind => GetInt32("kind");

    public PlayFabLobby Lobby => PlayFabLobby.From(GetObject("lobby"));

    public PlayFabResult Result => PlayFabResult.From(GetObject("result"));

    public PlayFabLobbyMember Member => PlayFabLobbyMember.From(GetObject("member"));

    public PlayFabLobbyInvite Invite => PlayFabLobbyInvite.From(GetObject("invite"));

    public PlayFabUser User => PlayFabUser.From(GetObject("user"));

    public Godot.Collections.Dictionary Properties => GetDict("properties");
}
