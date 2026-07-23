using System;
using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Services;

/// <summary><c>GDK.activation</c> — game activation events (protocol/file/invite).</summary>
public sealed class GdkActivation : GdkServiceBase
{
    internal GdkActivation(GodotObject o) : base(o)
    {
        _o.Connect("protocol_activated", Callable.From((Variant a0) => ProtocolActivated?.Invoke(a0.AsString())));
        _o.Connect("file_activated", Callable.From((Variant a0) => FileActivated?.Invoke(a0.AsString())));
        _o.Connect("pending_invite_received", Callable.From((Variant a0) => PendingInviteReceived?.Invoke(a0.AsGodotDictionary())));
        _o.Connect("invite_accepted", Callable.From((Variant a0) => InviteAccepted?.Invoke(a0.AsGodotDictionary())));
        _o.Connect("activated", Callable.From((Variant a0) => Activated?.Invoke(a0.AsGodotDictionary())));
    }

    public event Action<string> ProtocolActivated;
    public event Action<string> FileActivated;
    public event Action<Godot.Collections.Dictionary> PendingInviteReceived;
    public event Action<Godot.Collections.Dictionary> InviteAccepted;
    public event Action<Godot.Collections.Dictionary> Activated;

    public GdkResult AcceptPendingInvite(string inviteUri) =>
        GdkResult.From(Call("accept_pending_invite", inviteUri).AsGodotObject());
}
