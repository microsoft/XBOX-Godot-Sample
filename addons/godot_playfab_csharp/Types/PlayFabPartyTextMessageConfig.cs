using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyTextMessageConfig : PlayFabObject
{
    internal PlayFabPartyTextMessageConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyTextMessageConfig From(GodotObject o) => o == null ? null : new PlayFabPartyTextMessageConfig(o);

    public string LanguageCode => GetString("language_code");

    public string[] TranslateToLanguages => Get("translate_to_languages").AsStringArray();

    public Godot.Collections.Dictionary Metadata => GetDict("metadata");
}
