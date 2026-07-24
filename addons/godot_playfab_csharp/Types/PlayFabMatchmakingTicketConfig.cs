using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabMatchmakingTicketConfig : PlayFabObject
{
    internal PlayFabMatchmakingTicketConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabMatchmakingTicketConfig From(GodotObject o) => o == null ? null : new PlayFabMatchmakingTicketConfig(o);

    public string QueueName => GetString("queue_name");

    public int TimeoutSeconds => GetInt32("timeout_seconds");

    public Godot.Collections.Array Members => GetArray("members");
}
