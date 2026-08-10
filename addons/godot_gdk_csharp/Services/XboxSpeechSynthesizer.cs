using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Services;

/// <summary>
/// <c>GDK.get_speech()</c> — Windows text-to-speech synthesis, used by Game Chat
/// and for standalone narration. Synthesized audio can be played back through the
/// chat pipeline or returned as a Godot audio stream.
/// </summary>
public sealed class XboxSpeechSynthesizer : XboxServiceBase
{
    internal XboxSpeechSynthesizer(GodotObject o) : base(o) { }

    public Godot.Collections.Array GetInstalledVoices() =>
        Call("get_installed_voices").AsGodotArray();

    public XboxResult SetDefaultVoice() =>
        XboxResult.From(Call("set_default_voice").AsGodotObject());

    public XboxResult SetCustomVoice(string voiceId) =>
        XboxResult.From(Call("set_custom_voice", voiceId).AsGodotObject());

    public XboxResult SynthesizeText(string text) =>
        XboxResult.From(Call("synthesize_text", text).AsGodotObject());

    public XboxResult SynthesizeSsml(string ssml) =>
        XboxResult.From(Call("synthesize_ssml", ssml).AsGodotObject());

    public AudioStreamWav SynthesizeToStream(string text) =>
        Call("synthesize_to_stream", text).As<AudioStreamWav>();
}
