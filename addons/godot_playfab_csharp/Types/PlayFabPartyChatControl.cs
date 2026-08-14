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

    // Indicators and device state are polled, not signalled.
    public int GetLocalChatIndicator() =>
        Call("get_local_chat_indicator").AsInt32();

    public int GetChatIndicator() =>
        Call("get_chat_indicator").AsInt32();

    public int GetAudioInputState() =>
        Call("get_audio_input_state").AsInt32();

    public int GetAudioOutputState() =>
        Call("get_audio_output_state").AsInt32();

    public int GetAudioInputSelectionType() =>
        Call("get_audio_input_selection_type").AsInt32();

    public int GetAudioOutputSelectionType() =>
        Call("get_audio_output_selection_type").AsInt32();

    public string GetAudioInputDeviceId() =>
        Call("get_audio_input_device_id").AsString();

    public string GetAudioOutputDeviceId() =>
        Call("get_audio_output_device_id").AsString();

    public bool IsAudioInputMuted() =>
        Call("is_audio_input_muted").AsBool();

    public Task<PlayFabResult> SetAudioInputMutedAsync(bool muted) =>
        CallResultAsync("set_audio_input_muted_async", muted);

    public float GetAudioRenderVolume(PlayFabPartyChatControl target) =>
        Call("get_audio_render_volume", target?.Raw).AsSingle();

    public Task<PlayFabResult> SetAudioRenderVolumeAsync(PlayFabPartyChatControl target, float volume) =>
        CallResultAsync("set_audio_render_volume_async", target?.Raw, volume);

    public int GetPermissions(PlayFabPartyChatControl target) =>
        Call("get_permissions", target?.Raw).AsInt32();

    public bool IsAudioMuted(PlayFabPartyChatControl target) =>
        Call("is_audio_muted", target?.Raw).AsBool();

    public bool IsTextMuted(PlayFabPartyChatControl target) =>
        Call("is_text_muted", target?.Raw).AsBool();

    public string GetLanguage() =>
        Call("get_language").AsString();

    public Task<PlayFabResult> SetLanguageAsync(string languageCode) =>
        CallResultAsync("set_language_async", languageCode);

    public int GetTranscriptionOptions() =>
        Call("get_transcription_options").AsInt32();

    public Task<PlayFabResult> SetTranscriptionOptionsAsync(int options) =>
        CallResultAsync("set_transcription_options_async", options);

    public int GetTextChatOptions() =>
        Call("get_text_chat_options").AsInt32();

    public Task<PlayFabResult> SetTextChatOptionsAsync(int options) =>
        CallResultAsync("set_text_chat_options_async", options);

    public int GetAudioEncoderBitrate() =>
        Call("get_audio_encoder_bitrate").AsInt32();

    public Task<PlayFabResult> SetAudioEncoderBitrateAsync(int bitrate) =>
        CallResultAsync("set_audio_encoder_bitrate_async", bitrate);

    public int GetVoiceAudioOptions() =>
        Call("get_voice_audio_options").AsInt32();

    public Task<PlayFabResult> SetVoiceAudioOptionsAsync(int options) =>
        CallResultAsync("set_voice_audio_options_async", options);

    public Task<PlayFabResult> PopulateTextToSpeechProfilesAsync() =>
        CallResultAsync("populate_text_to_speech_profiles_async");

    public Godot.Collections.Array GetTextToSpeechProfiles() =>
        Call("get_text_to_speech_profiles").AsGodotArray();

    public PlayFabPartyTextToSpeechProfile GetTextToSpeechProfile(int type) =>
        PlayFabPartyTextToSpeechProfile.From(Call("get_text_to_speech_profile", type).AsGodotObject());

    public Task<PlayFabResult> SetTextToSpeechProfileAsync(int type, string profileId) =>
        CallResultAsync("set_text_to_speech_profile_async", type, profileId);

    public Task<PlayFabResult> SynthesizeTextToSpeechAsync(int type, string text) =>
        CallResultAsync("synthesize_text_to_speech_async", type, text);
}
