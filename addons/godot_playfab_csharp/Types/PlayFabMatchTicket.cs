using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabMatchTicket : PlayFabObject
{
    internal PlayFabMatchTicket(GodotObject o) : base(o)
    {
    }

    public static PlayFabMatchTicket From(GodotObject o) => o == null ? null : new PlayFabMatchTicket(o);

    private Action<PlayFabMatchTicketStateChange> _stateChanged;
    private Callable _stateChangedCallable;

    public event Action<PlayFabMatchTicketStateChange> StateChanged
    {
        add => AddSignal(ref _stateChanged, value, "state_changed", ref _stateChangedCallable,
            () => Callable.From((Variant a0) =>
                _stateChanged?.Invoke(PlayFabMatchTicketStateChange.From(a0.AsGodotObject()))));
        remove => RemoveSignal(ref _stateChanged, value, "state_changed", ref _stateChangedCallable);
    }

    public const int CREATED = 100;

    public const int STATUSCHANGED = 101;

    public const int COMPLETED = 102;

    public const int CANCELLED = 103;

    public const int FAILED = 104;

    public string TicketId => GetString("ticket_id");

    public string QueueName => GetString("queue_name");

    public int Status => GetInt32("status");

    public Godot.Collections.Array Members => GetArray("members");

    public string MatchId => GetString("match_id");

    public string ArrangedLobbyConnectionString => GetString("arranged_lobby_connection_string");

    public Godot.Collections.Dictionary Properties => GetDict("properties");

    public Task<PlayFabResult> RefreshAsync() =>
        CallResultAsync("refresh_async");

    public Task<PlayFabResult> CancelAsync() =>
        CallResultAsync("cancel_async");

    public bool IsComplete() =>
        Call("is_complete").AsBool();

    public bool IsCancelled() =>
        Call("is_cancelled").AsBool();
}
