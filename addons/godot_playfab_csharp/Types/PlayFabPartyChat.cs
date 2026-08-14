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

    // Chat indicators and voice-chat state are polled, not signalled: PlayFab
    // Party raises no state change when an indicator flips.
    public int GetLocalChatIndicator(PlayFabUser user = null) =>
        Call("get_local_chat_indicator", user?.Raw).AsInt32();

    public int GetChatIndicator(Godot.Collections.Dictionary entityKey, PlayFabUser user = null) =>
        Call("get_chat_indicator", entityKey, user?.Raw).AsInt32();

    public Godot.Collections.Array GetChatIndicators(PlayFabUser user = null) =>
        Call("get_chat_indicators", user?.Raw).AsGodotArray();

    public bool IsAudioInputMuted(PlayFabUser user = null) =>
        Call("is_audio_input_muted", user?.Raw).AsBool();

    public Task<PlayFabResult> SetAudioInputMutedAsync(bool muted, PlayFabUser user = null) =>
        CallResultAsync("set_audio_input_muted_async", muted, user?.Raw);

    public float GetAudioRenderVolume(Godot.Collections.Dictionary entityKey, PlayFabUser user = null) =>
        Call("get_audio_render_volume", entityKey, user?.Raw).AsSingle();

    public Task<PlayFabResult> SetAudioRenderVolumeAsync(
        Godot.Collections.Dictionary entityKey, float volume, PlayFabUser user = null) =>
        CallResultAsync("set_audio_render_volume_async", entityKey, volume, user?.Raw);

    public int GetChatPermissions(Godot.Collections.Dictionary entityKey, PlayFabUser user = null) =>
        Call("get_chat_permissions", entityKey, user?.Raw).AsInt32();

    public bool IsAudioMuted(Godot.Collections.Dictionary entityKey, PlayFabUser user = null) =>
        Call("is_audio_muted", entityKey, user?.Raw).AsBool();

    public bool IsTextMuted(Godot.Collections.Dictionary entityKey, PlayFabUser user = null) =>
        Call("is_text_muted", entityKey, user?.Raw).AsBool();

    public string GetLanguage(PlayFabUser user = null) =>
        Call("get_language", user?.Raw).AsString();

    public Task<PlayFabResult> SetLanguageAsync(string languageCode, PlayFabUser user = null) =>
        CallResultAsync("set_language_async", languageCode, user?.Raw);

    public int GetTranscriptionOptions(PlayFabUser user = null) =>
        Call("get_transcription_options", user?.Raw).AsInt32();

    public Task<PlayFabResult> SetTranscriptionOptionsAsync(int options, PlayFabUser user = null) =>
        CallResultAsync("set_transcription_options_async", options, user?.Raw);

    public int GetTextChatOptions(PlayFabUser user = null) =>
        Call("get_text_chat_options", user?.Raw).AsInt32();

    public Task<PlayFabResult> SetTextChatOptionsAsync(int options, PlayFabUser user = null) =>
        CallResultAsync("set_text_chat_options_async", options, user?.Raw);

    public int GetAudioEncoderBitrate(PlayFabUser user = null) =>
        Call("get_audio_encoder_bitrate", user?.Raw).AsInt32();

    public Task<PlayFabResult> SetAudioEncoderBitrateAsync(int bitrate, PlayFabUser user = null) =>
        CallResultAsync("set_audio_encoder_bitrate_async", bitrate, user?.Raw);

    public int GetVoiceAudioOptions(PlayFabUser user = null) =>
        Call("get_voice_audio_options", user?.Raw).AsInt32();

    public Task<PlayFabResult> SetVoiceAudioOptionsAsync(int options, PlayFabUser user = null) =>
        CallResultAsync("set_voice_audio_options_async", options, user?.Raw);

    public int GetAudioInputState(PlayFabUser user = null) =>
        Call("get_audio_input_state", user?.Raw).AsInt32();

    public int GetAudioOutputState(PlayFabUser user = null) =>
        Call("get_audio_output_state", user?.Raw).AsInt32();

    public Task<PlayFabResult> PopulateTextToSpeechProfilesAsync(PlayFabUser user = null) =>
        CallResultAsync("populate_text_to_speech_profiles_async", user?.Raw);

    public Godot.Collections.Array GetTextToSpeechProfiles(PlayFabUser user = null) =>
        Call("get_text_to_speech_profiles", user?.Raw).AsGodotArray();

    public PlayFabPartyTextToSpeechProfile GetTextToSpeechProfile(int type, PlayFabUser user = null) =>
        PlayFabPartyTextToSpeechProfile.From(Call("get_text_to_speech_profile", type, user?.Raw).AsGodotObject());

    public Task<PlayFabResult> SetTextToSpeechProfileAsync(int type, string profileId, PlayFabUser user = null) =>
        CallResultAsync("set_text_to_speech_profile_async", type, profileId, user?.Raw);

    public Task<PlayFabResult> SynthesizeTextToSpeechAsync(int type, string text, PlayFabUser user = null) =>
        CallResultAsync("synthesize_text_to_speech_async", type, text, user?.Raw);
}
