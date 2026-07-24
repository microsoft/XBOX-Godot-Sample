using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyConfig : PlayFabObject
{
    internal PlayFabPartyConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyConfig From(GodotObject o) => o == null ? null : new PlayFabPartyConfig(o);

    public int MaxPlayers => GetInt32("max_players");

    public int DirectPeerConnectivity => GetInt32("direct_peer_connectivity");

    public string InvitationId => GetString("invitation_id");

    public bool EnableVoiceChat => GetBool("enable_voice_chat");

    public bool EnableTextChat => GetBool("enable_text_chat");

    public bool EnableTranscription => GetBool("enable_transcription");

    public bool EnableTranslation => GetBool("enable_translation");

    public string AudioInput => GetString("audio_input");

    public string AudioOutput => GetString("audio_output");

    public Godot.Collections.Dictionary Metadata => GetDict("metadata");
}
