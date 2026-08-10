using System;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary>
/// <c>GDK.get_game_chat()</c> — Game Chat 2 voice and text chat. The title owns
/// transport: outgoing encoded frames arrive via <see cref="OutgoingDataFrame"/>
/// and inbound frames are fed back through <see cref="ProcessIncomingDataFrame"/>.
/// </summary>
public sealed class XboxGameChat : XboxServiceBase
{
    internal XboxGameChat(GodotObject o) : base(o)
    {
        _o.Connect("outgoing_data_frame", Callable.From((Variant a0, Variant a1, Variant a2) =>
            OutgoingDataFrame?.Invoke(a0.AsInt64Array(), a1.AsByteArray(), a2.AsInt32())));
        _o.Connect("text_chat_received", Callable.From((Variant a0, Variant a1) =>
            TextChatReceived?.Invoke(a0.AsString(), a1.AsString())));
        _o.Connect("transcribed_chat_received", Callable.From((Variant a0, Variant a1) =>
            TranscribedChatReceived?.Invoke(a0.AsString(), a1.AsString())));
    }

    /// <summary>Encoded chat data the title must deliver to the given endpoints.</summary>
    public event Action<long[], byte[], int> OutgoingDataFrame;

    /// <summary>A text chat message was received from a remote speaker.</summary>
    public event Action<string, string> TextChatReceived;

    /// <summary>Speech-to-text transcription was received for a remote speaker.</summary>
    public event Action<string, string> TranscribedChatReceived;

    public XboxResult Initialize(int maxUsers, int defaultRelationship) =>
        XboxResult.From(Call("initialize", maxUsers, defaultRelationship).AsGodotObject());

    public bool IsInitialized() => Call("is_initialized").AsBool();

    public void Cleanup() => Call("cleanup");

    public XboxResult AddLocalUser(XboxUser user) =>
        XboxResult.From(Call("add_local_user", user?.Raw).AsGodotObject());

    public XboxResult AddRemoteUser(string xuid, int endpointId) =>
        XboxResult.From(Call("add_remote_user", xuid, endpointId).AsGodotObject());

    public XboxResult RemoveUser(string xuid) =>
        XboxResult.From(Call("remove_user", xuid).AsGodotObject());

    public XboxResult SetCommunicationRelationship(string localXuid, string targetXuid, int relationship) =>
        XboxResult.From(Call("set_communication_relationship", localXuid, targetXuid, relationship).AsGodotObject());

    public XboxResult SetMicrophoneMuted(string localXuid, bool muted) =>
        XboxResult.From(Call("set_microphone_muted", localXuid, muted).AsGodotObject());

    public XboxResult SetRemoteUserMuted(string localXuid, string targetXuid, bool muted) =>
        XboxResult.From(Call("set_remote_user_muted", localXuid, targetXuid, muted).AsGodotObject());

    public XboxResult SetAudioRenderVolume(string localXuid, string targetXuid, float volume) =>
        XboxResult.From(Call("set_audio_render_volume", localXuid, targetXuid, volume).AsGodotObject());

    public XboxResult SendText(string localXuid, string text) =>
        XboxResult.From(Call("send_text", localXuid, text).AsGodotObject());

    public XboxResult SynthesizeTextToSpeech(string localXuid, string text) =>
        XboxResult.From(Call("synthesize_text_to_speech", localXuid, text).AsGodotObject());

    public XboxResult ProcessIncomingDataFrame(int sourceEndpointId, byte[] bytes) =>
        XboxResult.From(Call("process_incoming_data_frame", sourceEndpointId, bytes).AsGodotObject());

    public Godot.Collections.Array GetChatUsers() =>
        Call("get_chat_users").AsGodotArray();

    // Communication relationship flags (a GameChat2 bitmask).
    public const int RELATIONSHIPNONE = 0;
    public const int RELATIONSHIPSENDMICROPHONEAUDIO = 1;
    public const int RELATIONSHIPSENDTEXTTOSPEECHAUDIO = 2;
    public const int RELATIONSHIPSENDTEXT = 4;
    public const int RELATIONSHIPRECEIVEAUDIO = 8;
    public const int RELATIONSHIPRECEIVETEXT = 16;
    public const int RELATIONSHIPSENDALL = 7;
    public const int RELATIONSHIPRECEIVEALL = 24;
    public const int RELATIONSHIPSENDANDRECEIVEALL = 31;

    // Transport reliability requirement for an outgoing data frame.
    public const int TRANSPORTGUARANTEED = 0;
    public const int TRANSPORTBESTEFFORT = 1;

    // Per-user chat indicator state.
    public const int CHATINDICATORSILENT = 0;
    public const int CHATINDICATORTALKING = 1;
    public const int CHATINDICATORLOCALMICROPHONEMUTED = 2;
    public const int CHATINDICATORINCOMINGCOMMUNICATIONSMUTED = 3;
    public const int CHATINDICATORREPUTATIONRESTRICTED = 4;
    public const int CHATINDICATORPLATFORMRESTRICTED = 5;
    public const int CHATINDICATORNOCHATFOCUS = 6;
    public const int CHATINDICATORNOMICROPHONE = 7;
}
