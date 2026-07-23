using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyChat : PlayFabObject
{
    internal PlayFabPartyChat(GodotObject o) : base(o)
    {
        _o.Connect("state_changed", Callable.From((Variant a0) =>
            StateChanged?.Invoke(PlayFabPartyChatStateChange.From(a0.AsGodotObject()))));
        _o.Connect("chat_control_added", Callable.From((Variant a0, Variant a1) =>
            ChatControlAdded?.Invoke(a0.AsGodotDictionary(), PlayFabPartyChatControl.From(a1.AsGodotObject()))));
        _o.Connect("chat_control_removed", Callable.From((Variant a0) =>
            ChatControlRemoved?.Invoke(a0.AsGodotDictionary())));
        _o.Connect("text_message_received", Callable.From((Variant a0, Variant a1) =>
            TextMessageReceived?.Invoke(a0.AsGodotDictionary(), PlayFabPartyChatMessage.From(a1.AsGodotObject()))));
        _o.Connect("transcription_received", Callable.From((Variant a0, Variant a1) =>
            TranscriptionReceived?.Invoke(a0.AsGodotDictionary(), PlayFabPartyChatMessage.From(a1.AsGodotObject()))));
        _o.Connect("chat_permissions_changed", Callable.From((Variant a0, Variant a1) =>
            ChatPermissionsChanged?.Invoke(a0.AsGodotDictionary(), a1.AsInt32())));
        _o.Connect("audio_muted_changed", Callable.From((Variant a0, Variant a1) =>
            AudioMutedChanged?.Invoke(a0.AsGodotDictionary(), a1.AsBool())));
        _o.Connect("text_muted_changed", Callable.From((Variant a0, Variant a1) =>
            TextMutedChanged?.Invoke(a0.AsGodotDictionary(), a1.AsBool())));
    }

    public static PlayFabPartyChat From(GodotObject o) => o == null ? null : new PlayFabPartyChat(o);

    public event Action<PlayFabPartyChatStateChange> StateChanged;
    public event Action<Godot.Collections.Dictionary, PlayFabPartyChatControl> ChatControlAdded;
    public event Action<Godot.Collections.Dictionary> ChatControlRemoved;
    public event Action<Godot.Collections.Dictionary, PlayFabPartyChatMessage> TextMessageReceived;
    public event Action<Godot.Collections.Dictionary, PlayFabPartyChatMessage> TranscriptionReceived;
    public event Action<Godot.Collections.Dictionary, int> ChatPermissionsChanged;
    public event Action<Godot.Collections.Dictionary, bool> AudioMutedChanged;
    public event Action<Godot.Collections.Dictionary, bool> TextMutedChanged;

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
