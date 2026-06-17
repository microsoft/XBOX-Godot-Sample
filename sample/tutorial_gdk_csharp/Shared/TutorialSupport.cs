using Godot;

/// <summary>
/// Small typed-Dictionary helpers shared across the GDK tutorial scenes.
///
/// The GDK-only track never touches PlayFab, so this helper carries only the
/// generic Godot.Collections.Dictionary accessors (no PlayFab config
/// factories like the integrated track's TutorialSupport).
/// </summary>
public static class TutorialSupport
{
    public static string DictString(Godot.Collections.Dictionary dict, string key, string fallback = "") =>
        dict != null && dict.ContainsKey(key) ? dict[key].AsString() : fallback;

    public static int DictInt(Godot.Collections.Dictionary dict, string key, int fallback = 0) =>
        dict != null && dict.ContainsKey(key) ? dict[key].AsInt32() : fallback;

    public static bool DictBool(Godot.Collections.Dictionary dict, string key, bool fallback = false) =>
        dict != null && dict.ContainsKey(key) ? dict[key].AsBool() : fallback;

    public static Godot.Collections.Array DictArray(Godot.Collections.Dictionary dict, string key) =>
        dict != null && dict.ContainsKey(key) ? dict[key].AsGodotArray() : new Godot.Collections.Array();

    public static Godot.Collections.Dictionary DictDict(Godot.Collections.Dictionary dict, string key) =>
        dict != null && dict.ContainsKey(key) ? dict[key].AsGodotDictionary() : new Godot.Collections.Dictionary();
}
