using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Services;

/// <summary>
/// <c>GDK.get_speech()</c> — Windows text-to-speech synthesis, used by Game Chat
/// and for standalone narration. Synthesized audio can be played back through the
/// chat pipeline or returned as a Godot audio stream.
/// </summary>
public sealed class GdkSpeechSynthesizer : GdkServiceBase
{
    internal GdkSpeechSynthesizer(GodotObject o) : base(o) { }

    public Godot.Collections.Array GetInstalledVoices() =>
        Call("get_installed_voices").AsGodotArray();

    public GdkResult SetDefaultVoice() =>
        GdkResult.From(Call("set_default_voice").AsGodotObject());

    public GdkResult SetCustomVoice(string voiceId) =>
        GdkResult.From(Call("set_custom_voice", voiceId).AsGodotObject());

    public GdkResult SynthesizeText(string text) =>
        GdkResult.From(Call("synthesize_text", text).AsGodotObject());

    public GdkResult SynthesizeSsml(string ssml) =>
        GdkResult.From(Call("synthesize_ssml", ssml).AsGodotObject());

    public AudioStreamWav SynthesizeToStream(string text) =>
        Call("synthesize_to_stream", text).As<AudioStreamWav>();
}
