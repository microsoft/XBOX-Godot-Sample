extends Control

const AddonApi = preload("res://shared/addon_api.gd")

## GDK Tutorial 5 reference scene — Text-to-Speech (GDK.speech).
##
## GDK-only, no PlayFab and no network: XSpeechSynthesizer runs locally and
## produces WAV/PCM bytes. Buttons drive each step:
##   - List the installed system voices (GDK.speech.get_installed_voices).
##   - Synthesize the text box to an AudioStreamWAV and play it back
##     (GDK.speech.synthesize_to_stream → AudioStreamPlayer).
##   - Synthesize SSML markup so prosody/voice control is demonstrated
##     (GDK.speech.synthesize_ssml → GDKResult.data.audio_wav).
##
## Text-to-speech only needs the GDK runtime initialized; it does NOT require a
## signed-in user, so the scene works even if Xbox sign-in is unavailable.
##
## NOTE: scene scripts use `get_node("/root/GdkAuth")` instead of the bare
## `GdkAuth.` reference so the headless parse gate stays clean.
##
## Source: docs/gdk/api-reference.md (Speech service: GDK.speech)

@onready var _log: RichTextLabel = $Root/LogPanel/Log
@onready var _text_input: LineEdit = $Root/TextInput
@onready var _voices_btn: Button = $Root/Buttons/VoicesBtn
@onready var _speak_btn: Button = $Root/Buttons/SpeakBtn
@onready var _ssml_btn: Button = $Root/Buttons/SsmlBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn
@onready var _player: AudioStreamPlayer = $AudioPlayer

var _auth: Node = null

func _ready() -> void:
	_back_btn.pressed.connect(_on_back_pressed)
	_voices_btn.pressed.connect(_on_voices_pressed)
	_speak_btn.pressed.connect(_on_speak_pressed)
	_ssml_btn.pressed.connect(_on_ssml_pressed)

	_auth = get_node_or_null("/root/GdkAuth")
	if not Engine.has_singleton("GDK"):
		_append("[color=red]GDK extension is not loaded.[/color]")
		_set_buttons_enabled(false)
		return

	_set_buttons_enabled(false)
	_append("Initializing GDK runtime…")

	# TTS only needs GDK.initialize() (done by GdkAuth.sign_in()); a signed-in
	# user is not required, so a failed sign-in still leaves speech usable.
	if _auth != null:
		await _auth.call("sign_in")

	if AddonApi.singleton("GDK").is_initialized():
		_append("GDK runtime ready — text-to-speech is available (no user required).")
		_set_buttons_enabled(true)
	else:
		_append("[color=red]GDK runtime is not initialized — check MicrosoftGame.config.[/color]")

func _on_voices_pressed() -> void:
	var voices: Array = AddonApi.singleton("GDK").speech.get_installed_voices()
	if voices.is_empty():
		_append("[color=orange][Voices] No installed voices reported.[/color]")
		return
	_append("[Voices] %d installed voice(s):" % voices.size())
	for voice in voices:
		_append("  • %s (%s, %s)" % [
				voice.get("display_name", "?"),
				voice.get("language", "?"),
				voice.get("gender", "?")])

func _on_speak_pressed() -> void:
	var text := _text_input.text.strip_edges()
	if text.is_empty():
		text = "Hello from the Xbox GDK text to speech engine."

	# Make sure we are using the system default voice for this pass.
	var pick = AddonApi.singleton("GDK").speech.set_default_voice()
	if not pick.ok:
		_append("[color=orange][Speak] set_default_voice failed: %s[/color]" % pick.message)

	# synthesize_to_stream wraps synthesize_text and packs the WAV bytes into a
	# ready-to-play AudioStreamWAV.
	var stream: AudioStreamWAV = AddonApi.singleton("GDK").speech.synthesize_to_stream(text)
	if stream == null:
		_append("[color=orange][Speak] Synthesis returned no audio.[/color]")
		return
	_player.stream = stream
	_player.play()
	_append("[color=green][Speak] Playing %d sample(s): \"%s\"[/color]" % [
			stream.data.size(), text])

func _on_ssml_pressed() -> void:
	# SSML lets the title control prosody, emphasis, and voice selection inline.
	var ssml := "<speak version='1.0' xml:lang='en-US'>" + \
			"GDK speech also supports <emphasis level='strong'>SSML</emphasis>, " + \
			"<break time='300ms'/> for full prosody control.</speak>"
	var result = AddonApi.singleton("GDK").speech.synthesize_ssml(ssml)
	if not result.ok:
		_append("[color=orange][SSML] synthesize_ssml failed: %s (%s)[/color]" % [result.message, result.code])
		return
	var wav: PackedByteArray = result.data.get("audio_wav", PackedByteArray())
	_append("[color=green][SSML] Synthesized %d WAV byte(s).[/color]" % wav.size())

func _set_buttons_enabled(enabled: bool) -> void:
	_voices_btn.disabled = not enabled
	_speak_btn.disabled = not enabled
	_ssml_btn.disabled = not enabled

func _append(line: String) -> void:
	_log.append_text(line + "\n")
	print(line)

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
