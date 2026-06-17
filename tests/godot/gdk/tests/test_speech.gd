extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_speech_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var speech = gdk.get_speech()
	assert_not_null(speech, "GDK.speech returns service object")
	if speech == null:
		return

	for method_name in [
		"get_installed_voices",
		"set_default_voice",
		"set_custom_voice",
		"synthesize_text",
		"synthesize_ssml",
		"synthesize_to_stream",
	]:
		assert_has_method_named(speech, method_name)

	var pre_init_result = speech.synthesize_text("hello")
	assert_result_error(pre_init_result, "not_initialized", "synthesize_text() rejects calls before GDK.initialize()")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for speech behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Speech runtime behavior: %s" % init_result.message)
		return

	assert_result_error(speech.synthesize_text(""), "invalid_input", "synthesize_text('') rejects blank text")
	assert_result_error(speech.synthesize_ssml("   "), "invalid_input", "synthesize_ssml() rejects whitespace input")
	assert_result_error(speech.set_custom_voice(""), "invalid_voice_id", "set_custom_voice('') rejects blank voice id")

	var voices = speech.get_installed_voices()
	assert_typeof(voices, TYPE_ARRAY, "get_installed_voices() returns an Array")


func test_speech_synthesis_produces_audio_when_available() -> void:
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Speech runtime unavailable")
		return

	var gdk = get_gdk()
	var speech = gdk.get_speech()
	if speech == null:
		return

	speech.set_default_voice()
	var result = speech.synthesize_text("GDK speech synthesis test.")
	assert_not_null(result, "synthesize_text() returns GDKResult")
	if result == null:
		return
	if not result.ok:
		pending("Speech synthesis unavailable on this host: %s" % result.message)
		return

	assert_dict_has_key(result.data, "audio_wav", "synthesize_text() result exposes audio_wav")
	assert_dict_has_key(result.data, "byte_count", "synthesize_text() result exposes byte_count")
	var audio: PackedByteArray = result.data["audio_wav"]
	assert_gt(audio.size(), 0, "synthesize_text() produces non-empty audio bytes")
	assert_eq(int(result.data["byte_count"]), audio.size(), "byte_count matches audio_wav length")

	var stream = speech.synthesize_to_stream("GDK speech stream test.")
	assert_not_null(stream, "synthesize_to_stream() returns an AudioStreamWAV when synthesis succeeds")
