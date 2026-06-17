using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyChatControl : PlayFabObject
{
    internal PlayFabPartyChatControl(GodotObject o) : base(o)
    {
        _o.Connect("state_changed", Callable.From((Variant a0) =>
            StateChanged?.Invoke(PlayFabPartyChatStateChange.From(a0.AsGodotObject()))));
        _o.Connect("message_received", Callable.From((Variant a0) =>
            MessageReceived?.Invoke(PlayFabPartyChatMessage.From(a0.AsGodotObject()))));
        _o.Connect("transcription_received", Callable.From((Variant a0) =>
            TranscriptionReceived?.Invoke(PlayFabPartyChatMessage.From(a0.AsGodotObject()))));
    }

    public static PlayFabPartyChatControl From(GodotObject o) => o == null ? null : new PlayFabPartyChatControl(o);

    public event Action<PlayFabPartyChatStateChange> StateChanged;
    public event Action<PlayFabPartyChatMessage> MessageReceived;
    public event Action<PlayFabPartyChatMessage> TranscriptionReceived;

    public string Id => GetString("id");

    public PlayFabUser User => PlayFabUser.From(GetObject("user"));

    public bool IsVoiceEnabled => GetBool("is_voice_enabled");

    public bool IsTextEnabled => GetBool("is_text_enabled");

    public bool IsTranscriptionEnabled => GetBool("is_transcription_enabled");

    public bool IsLocal => GetBool("is_local");

    public string GetId() =>
        Call("get_id").AsString();

    public PlayFabUser GetUser() =>
        PlayFabUser.From(Call("get_user").AsGodotObject());





    public Task<PlayFabResult> SendTextAsync(Godot.Collections.Array targets, string message, PlayFabPartyTextMessageConfig config = null) =>
        CallResultAsync("send_text_async", targets ?? new Godot.Collections.Array(), message, config?.Raw);

    public Task<PlayFabResult> SetPermissionsAsync(PlayFabPartyChatControl target, int permissions) =>
        CallResultAsync("set_permissions_async", target?.Raw, permissions);

    public Task<PlayFabResult> SetMutedAsync(PlayFabPartyChatControl target, bool muted) =>
        CallResultAsync("set_muted_async", target?.Raw, muted);

    public Task<PlayFabResult> DestroyAsync() =>
        CallResultAsync("destroy_async");
}
