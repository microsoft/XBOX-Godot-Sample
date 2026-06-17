using Godot;
using System.Threading.Tasks;
using GodotGdk;

/// <summary>
/// GDK Tutorial 5 reference scene — Text-to-Speech (GDK.speech).
///
/// GDK-only, no PlayFab and no network: XSpeechSynthesizer runs locally and
/// produces WAV/PCM bytes. Buttons list installed voices, synthesize the text
/// box to an AudioStreamWav and play it back, and synthesize SSML markup.
///
/// Text-to-speech only needs the GDK runtime initialized; it does NOT require a
/// signed-in user, so the scene works even if Xbox sign-in is unavailable.
/// </summary>
public partial class G05Speech : Control
{
    private RichTextLabel _log;
    private LineEdit _textInput;
    private Button _voicesBtn;
    private Button _speakBtn;
    private Button _ssmlBtn;
    private Button _backBtn;
    private AudioStreamPlayer _player;
    private GdkAuth _auth;

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _textInput = GetNode<LineEdit>("Root/TextInput");
        _voicesBtn = GetNode<Button>("Root/Buttons/VoicesBtn");
        _speakBtn = GetNode<Button>("Root/Buttons/SpeakBtn");
        _ssmlBtn = GetNode<Button>("Root/Buttons/SsmlBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _player = GetNode<AudioStreamPlayer>("AudioPlayer");
        _backBtn.Pressed += OnBackPressed;
        _voicesBtn.Pressed += OnVoicesPressed;
        _speakBtn.Pressed += OnSpeakPressed;
        _ssmlBtn.Pressed += OnSsmlPressed;

        _auth = GetNodeOrNull<GdkAuth>("/root/GdkAuth");
        if (!Gdk.IsAvailable)
        {
            Append("[color=red]GDK extension is not loaded.[/color]");
            SetButtonsEnabled(false);
            return;
        }

        SetButtonsEnabled(false);
        Append("Initializing GDK runtime…");

        // TTS only needs GDK.Initialize() (done by GdkAuth.SignInAsync()); a
        // signed-in user is not required, so a failed sign-in still leaves
        // speech usable.
        if (_auth != null) await _auth.SignInAsync();
        if (!IsInsideTree()) return;

        if (Gdk.IsInitialized)
        {
            Append("GDK runtime ready — text-to-speech is available (no user required).");
            SetButtonsEnabled(true);
        }
        else
        {
            Append("[color=red]GDK runtime is not initialized — check MicrosoftGame.config.[/color]");
        }
    }

    private void OnVoicesPressed()
    {
        Godot.Collections.Array voices = Gdk.Speech.GetInstalledVoices();
        if (voices.Count == 0) { Append("[color=orange][Voices] No installed voices reported.[/color]"); return; }
        Append($"[Voices] {voices.Count} installed voice(s):");
        foreach (Variant entry in voices)
        {
            Godot.Collections.Dictionary voice = entry.AsGodotDictionary();
            Append($"  • {TutorialSupport.DictString(voice, "display_name", "?")} ({TutorialSupport.DictString(voice, "language", "?")}, {TutorialSupport.DictString(voice, "gender", "?")})");
        }
    }

    private void OnSpeakPressed()
    {
        string text = _textInput.Text.Trim();
        if (string.IsNullOrEmpty(text)) text = "Hello from the Xbox GDK text to speech engine.";

        // Make sure we are using the system default voice for this pass.
        GdkResult pick = Gdk.Speech.SetDefaultVoice();
        if (!pick.Ok) Append($"[color=orange][Speak] set_default_voice failed: {pick.Message}[/color]");

        // SynthesizeToStream wraps synthesize_text and packs the WAV bytes into
        // a ready-to-play AudioStreamWav.
        AudioStreamWav stream = Gdk.Speech.SynthesizeToStream(text);
        if (stream == null) { Append("[color=orange][Speak] Synthesis returned no audio.[/color]"); return; }
        _player.Stream = stream;
        _player.Play();
        Append($"[color=green][Speak] Playing {stream.Data.Length} sample(s): \"{text}\"[/color]");
    }

    private void OnSsmlPressed()
    {
        // SSML lets the title control prosody, emphasis, and voice selection inline.
        const string ssml = "<speak version='1.0' xml:lang='en-US'>" +
            "GDK speech also supports <emphasis level='strong'>SSML</emphasis>, " +
            "<break time='300ms'/> for full prosody control.</speak>";
        GdkResult result = Gdk.Speech.SynthesizeSsml(ssml);
        if (!result.Ok) { Append($"[color=orange][SSML] synthesize_ssml failed: {result.Message} ({result.Code})[/color]"); return; }
        Godot.Collections.Dictionary data = result.Data.AsGodotDictionary();
        byte[] wav = data.ContainsKey("audio_wav") ? data["audio_wav"].AsByteArray() : System.Array.Empty<byte>();
        Append($"[color=green][SSML] Synthesized {wav.Length} WAV byte(s).[/color]");
    }

    private void SetButtonsEnabled(bool enabled)
    {
        _voicesBtn.Disabled = !enabled;
        _speakBtn.Disabled = !enabled;
        _ssmlBtn.Disabled = !enabled;
    }

    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}
