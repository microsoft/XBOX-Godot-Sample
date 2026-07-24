using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyChatControl : PlayFabObject
{
    internal PlayFabPartyChatControl(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyChatControl From(GodotObject o) => o == null ? null : new PlayFabPartyChatControl(o);

    private Action<PlayFabPartyChatStateChange> _stateChanged;
    private Callable _stateChangedCallable;
    private Action<PlayFabPartyChatMessage> _messageReceived;
    private Callable _messageReceivedCallable;
    private Action<PlayFabPartyChatMessage> _transcriptionReceived;
    private Callable _transcriptionReceivedCallable;

    public event Action<PlayFabPartyChatStateChange> StateChanged
    {
        add => AddSignal(ref _stateChanged, value, "state_changed", ref _stateChangedCallable,
            () => Callable.From((Variant a0) =>
                _stateChanged?.Invoke(PlayFabPartyChatStateChange.From(a0.AsGodotObject()))));
        remove => RemoveSignal(ref _stateChanged, value, "state_changed", ref _stateChangedCallable);
    }

    public event Action<PlayFabPartyChatMessage> MessageReceived
    {
        add => AddSignal(ref _messageReceived, value, "message_received", ref _messageReceivedCallable,
            () => Callable.From((Variant a0) =>
                _messageReceived?.Invoke(PlayFabPartyChatMessage.From(a0.AsGodotObject()))));
        remove => RemoveSignal(ref _messageReceived, value, "message_received", ref _messageReceivedCallable);
    }

    public event Action<PlayFabPartyChatMessage> TranscriptionReceived
    {
        add => AddSignal(ref _transcriptionReceived, value, "transcription_received", ref _transcriptionReceivedCallable,
            () => Callable.From((Variant a0) =>
                _transcriptionReceived?.Invoke(PlayFabPartyChatMessage.From(a0.AsGodotObject()))));
        remove => RemoveSignal(ref _transcriptionReceived, value, "transcription_received", ref _transcriptionReceivedCallable);
    }

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

    public Task<PlayFabResult> SetAudioMutedAsync(PlayFabPartyChatControl target, bool muted) =>
        CallResultAsync("set_audio_muted_async", target?.Raw, muted);

    public Task<PlayFabResult> SetTextMutedAsync(PlayFabPartyChatControl target, bool muted) =>
        CallResultAsync("set_text_muted_async", target?.Raw, muted);

    public Task<PlayFabResult> DestroyAsync() =>
        CallResultAsync("destroy_async");
}
