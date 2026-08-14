using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

/// <summary>
/// A synthetic voice available to PlayFab Party text-to-speech. Populate the
/// list with <see cref="PlayFabPartyChat.PopulateTextToSpeechProfilesAsync"/>.
/// </summary>
public sealed class PlayFabPartyTextToSpeechProfile : PlayFabObject
{
    internal PlayFabPartyTextToSpeechProfile(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyTextToSpeechProfile From(GodotObject o) => o == null ? null : new PlayFabPartyTextToSpeechProfile(o);

    public string Identifier => GetString("identifier");

    public string Name => GetString("name");

    public string LanguageCode => GetString("language_code");

    public int Gender => GetInt32("gender");

    public string GetIdentifier() =>
        Call("get_identifier").AsString();

    public string GetName() =>
        Call("get_name").AsString();

    public string GetLanguageCode() =>
        Call("get_language_code").AsString();

    public int GetGender() =>
        Call("get_gender").AsInt32();
}
