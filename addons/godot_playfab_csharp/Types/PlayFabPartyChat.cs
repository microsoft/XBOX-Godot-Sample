using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyChat : PlayFabObject
{
    internal PlayFabPartyChat(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyChat From(GodotObject o) => o == null ? null : new PlayFabPartyChat(o);

    private Action<PlayFabPartyChatStateChange> _stateChanged;
    private Callable _stateChangedCallable;
    private Action<Godot.Collections.Dictionary, PlayFabPartyChatControl> _chatControlAdded;
    private Callable _chatControlAddedCallable;
    private Action<Godot.Collections.Dictionary> _chatControlRemoved;
    private Callable _chatControlRemovedCallable;
    private Action<Godot.Collections.Dictionary, PlayFabPartyChatMessage> _textMessageReceived;
    private Callable _textMessageReceivedCallable;
    private Action<Godot.Collections.Dictionary, PlayFabPartyChatMessage> _transcriptionReceived;
    private Callable _transcriptionReceivedCallable;
    private Action<Godot.Collections.Dictionary, int> _chatPermissionsChanged;
    private Callable _chatPermissionsChangedCallable;
    private Action<Godot.Collections.Dictionary, bool> _audioMutedChanged;
    private Callable _audioMutedChangedCallable;
    private Action<Godot.Collections.Dictionary, bool> _textMutedChanged;
    private Callable _textMutedChangedCallable;

    public event Action<PlayFabPartyChatStateChange> StateChanged
    {
        add => AddSignal(ref _stateChanged, value, "state_changed", ref _stateChangedCallable,
            () => Callable.From((Variant a0) =>
                _stateChanged?.Invoke(PlayFabPartyChatStateChange.From(a0.AsGodotObject()))));
        remove => RemoveSignal(ref _stateChanged, value, "state_changed", ref _stateChangedCallable);
    }

    public event Action<Godot.Collections.Dictionary, PlayFabPartyChatControl> ChatControlAdded
    {
        add => AddSignal(ref _chatControlAdded, value, "chat_control_added", ref _chatControlAddedCallable,
            () => Callable.From((Variant a0, Variant a1) =>
                _chatControlAdded?.Invoke(a0.AsGodotDictionary(), PlayFabPartyChatControl.From(a1.AsGodotObject()))));
        remove => RemoveSignal(ref _chatControlAdded, value, "chat_control_added", ref _chatControlAddedCallable);
    }

    public event Action<Godot.Collections.Dictionary> ChatControlRemoved
    {
        add => AddSignal(ref _chatControlRemoved, value, "chat_control_removed", ref _chatControlRemovedCallable,
            () => Callable.From((Variant a0) =>
                _chatControlRemoved?.Invoke(a0.AsGodotDictionary())));
        remove => RemoveSignal(ref _chatControlRemoved, value, "chat_control_removed", ref _chatControlRemovedCallable);
    }

    public event Action<Godot.Collections.Dictionary, PlayFabPartyChatMessage> TextMessageReceived
    {
        add => AddSignal(ref _textMessageReceived, value, "text_message_received", ref _textMessageReceivedCallable,
            () => Callable.From((Variant a0, Variant a1) =>
                _textMessageReceived?.Invoke(a0.AsGodotDictionary(), PlayFabPartyChatMessage.From(a1.AsGodotObject()))));
        remove => RemoveSignal(ref _textMessageReceived, value, "text_message_received", ref _textMessageReceivedCallable);
    }

    public event Action<Godot.Collections.Dictionary, PlayFabPartyChatMessage> TranscriptionReceived
    {
        add => AddSignal(ref _transcriptionReceived, value, "transcription_received", ref _transcriptionReceivedCallable,
            () => Callable.From((Variant a0, Variant a1) =>
                _transcriptionReceived?.Invoke(a0.AsGodotDictionary(), PlayFabPartyChatMessage.From(a1.AsGodotObject()))));
        remove => RemoveSignal(ref _transcriptionReceived, value, "transcription_received", ref _transcriptionReceivedCallable);
    }

    public event Action<Godot.Collections.Dictionary, int> ChatPermissionsChanged
    {
        add => AddSignal(ref _chatPermissionsChanged, value, "chat_permissions_changed", ref _chatPermissionsChangedCallable,
            () => Callable.From((Variant a0, Variant a1) =>
                _chatPermissionsChanged?.Invoke(a0.AsGodotDictionary(), a1.AsInt32())));
        remove => RemoveSignal(ref _chatPermissionsChanged, value, "chat_permissions_changed", ref _chatPermissionsChangedCallable);
    }

    public event Action<Godot.Collections.Dictionary, bool> AudioMutedChanged
    {
        add => AddSignal(ref _audioMutedChanged, value, "audio_muted_changed", ref _audioMutedChangedCallable,
            () => Callable.From((Variant a0, Variant a1) =>
                _audioMutedChanged?.Invoke(a0.AsGodotDictionary(), a1.AsBool())));
        remove => RemoveSignal(ref _audioMutedChanged, value, "audio_muted_changed", ref _audioMutedChangedCallable);
    }

    public event Action<Godot.Collections.Dictionary, bool> TextMutedChanged
    {
        add => AddSignal(ref _textMutedChanged, value, "text_muted_changed", ref _textMutedChangedCallable,
            () => Callable.From((Variant a0, Variant a1) =>
                _textMutedChanged?.Invoke(a0.AsGodotDictionary(), a1.AsBool())));
        remove => RemoveSignal(ref _textMutedChanged, value, "text_muted_changed", ref _textMutedChangedCallable);
    }

    public Task<PlayFabResult> CreateLocalChatControlAsync(PlayFabUser user, PlayFabPartyConfig config = null) =>
        CallResultAsync("create_local_chat_control_async", user?.Raw, config?.Raw);

    public Task<PlayFabResult> DestroyLocalChatControlAsync(PlayFabUser user) =>
        CallResultAsync("destroy_local_chat_control_async", user?.Raw);

    public PlayFabPartyChatControl GetLocalChatControl(PlayFabUser user) =>
        PlayFabPartyChatControl.From(Call("get_local_chat_control", user?.Raw).AsGodotObject());

    public Godot.Collections.Array GetChatControls() =>
        Call("get_chat_controls").AsGodotArray();

    public Godot.Collections.Array GetRemoteEntityKeys() =>
        Call("get_remote_entity_keys").AsGodotArray();

    public PlayFabPartyChatControl GetChatControl(Godot.Collections.Dictionary entityKey) =>
        PlayFabPartyChatControl.From(Call("get_chat_control", entityKey).AsGodotObject());

    public Task<PlayFabResult> SendTextAsync(
        string message, Godot.Collections.Array targetEntityKeys, PlayFabPartyTextMessageConfig config = null) =>
        CallResultAsync("send_text_async", message, targetEntityKeys ?? new Godot.Collections.Array(), config?.Raw);

    public Task<PlayFabResult> SetAudioMutedAsync(Godot.Collections.Dictionary entityKey, bool muted) =>
        CallResultAsync("set_audio_muted_async", entityKey, muted);

    public Task<PlayFabResult> SetChatPermissionsAsync(Godot.Collections.Dictionary entityKey, int permissions) =>
        CallResultAsync("set_chat_permissions_async", entityKey, permissions);

    public Task<PlayFabResult> SetTextMutedAsync(Godot.Collections.Dictionary entityKey, bool muted) =>
        CallResultAsync("set_text_muted_async", entityKey, muted);
}
