using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyChatMessage : PlayFabObject
{
    internal PlayFabPartyChatMessage(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyChatMessage From(GodotObject o) => o == null ? null : new PlayFabPartyChatMessage(o);

    public PlayFabPartyChatControl Sender => PlayFabPartyChatControl.From(GetObject("sender"));

    public Godot.Collections.Dictionary SenderEntityKey => GetDict("sender_entity_key");

    public Godot.Collections.Array Targets => GetArray("targets");

    public string Text => GetString("text");

    public string LanguageCode => GetString("language_code");

    public string TranslatedText => GetString("translated_text");

    public bool IsTranscription => GetBool("is_transcription");

    public int Timestamp => GetInt32("timestamp");

    public Godot.Collections.Dictionary Metadata => GetDict("metadata");

    public PlayFabPartyChatControl GetSender() =>
        PlayFabPartyChatControl.From(Call("get_sender").AsGodotObject());

    public Godot.Collections.Dictionary GetSenderEntityKey() =>
        Call("get_sender_entity_key").AsGodotDictionary();

    public Godot.Collections.Array GetTargets() =>
        Call("get_targets").AsGodotArray();

    public string GetText() =>
        Call("get_text").AsString();

    public string GetLanguageCode() =>
        Call("get_language_code").AsString();

    public string GetTranslatedText() =>
        Call("get_translated_text").AsString();


    public int GetTimestamp() =>
        Call("get_timestamp").AsInt32();

    public Godot.Collections.Dictionary GetMetadata() =>
        Call("get_metadata").AsGodotDictionary();
}
