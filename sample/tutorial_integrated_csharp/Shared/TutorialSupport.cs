using Godot;
using System.Threading.Tasks;
using GodotPlayFab;
using GodotPlayFab.Types;

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

    public static string DisplayName(Godot.Collections.Dictionary row)
    {
        string display = DictString(row, "display_name");
        if (!string.IsNullOrEmpty(display)) return display;
        return DictString(DictDict(row, "entity"), "id", "?");
    }

    public static int PrimaryScore(Godot.Collections.Dictionary row)
    {
        if (row == null || !row.ContainsKey("scores")) return 0;
        string[] scores = row["scores"].AsStringArray();
        return scores.Length > 0 && int.TryParse(scores[0], out int value) ? value : 0;
    }

    public static PlayFabLobbyConfig LobbyConfig(int maxPlayers, int accessPolicy, int ownerMigrationPolicy,
        Godot.Collections.Dictionary search, Godot.Collections.Dictionary lobbyProps, Godot.Collections.Dictionary memberProps)
    {
        GodotObject obj = ClassDB.Instantiate("PlayFabLobbyConfig").AsGodotObject();
        obj.Set("max_players", maxPlayers);
        obj.Set("access_policy", accessPolicy);
        obj.Set("owner_migration_policy", ownerMigrationPolicy);
        obj.Set("search_properties", search ?? new Godot.Collections.Dictionary());
        obj.Set("lobby_properties", lobbyProps ?? new Godot.Collections.Dictionary());
        obj.Set("member_properties", memberProps ?? new Godot.Collections.Dictionary());
        return PlayFabLobbyConfig.From(obj);
    }

    public static PlayFabLobbyJoinConfig LobbyJoinConfig(Godot.Collections.Dictionary memberProps)
    {
        GodotObject obj = ClassDB.Instantiate("PlayFabLobbyJoinConfig").AsGodotObject();
        obj.Set("member_properties", memberProps ?? new Godot.Collections.Dictionary());
        return PlayFabLobbyJoinConfig.From(obj);
    }

    public static PlayFabPartyConfig PartyConfig(int maxPlayers, int directPeerConnectivity, bool voice, bool text, string invitationId = "")
    {
        GodotObject obj = ClassDB.Instantiate("PlayFabPartyConfig").AsGodotObject();
        obj.Set("max_players", maxPlayers);
        obj.Set("direct_peer_connectivity", directPeerConnectivity);
        obj.Set("enable_voice_chat", voice);
        obj.Set("enable_text_chat", text);
        obj.Set("enable_transcription", false);
        obj.Set("invitation_id", invitationId ?? string.Empty);
        return PlayFabPartyConfig.From(obj);
    }
}


