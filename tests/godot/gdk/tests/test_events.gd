extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_events_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var events = gdk.get_events()
	assert_not_null(events, "GDK.events returns service object")
	if events == null:
		return

	for method_name in [
		"write_event",
		"set_play_session_id",
		"get_play_session_id",
	]:
		assert_has_method_named(events, method_name)

	var blank_user = instantiate_class("GDKUser")

	var pre_init_result = events.write_event(blank_user, "LevelComplete")
	assert_result_error(pre_init_result, "not_initialized", "write_event() rejects calls before GDK.initialize()")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for events behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Events runtime behavior: %s" % init_result.message)
		return

	assert_result_error(events.write_event(blank_user, "  "), "invalid_event_name", "write_event() rejects blank event names")
	assert_result_error(events.write_event(null, "LevelComplete"), "invalid_user", "write_event() requires a signed-in user")


func test_events_play_session_id_round_trips() -> void:
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Events runtime unavailable")
		return

	var gdk = get_gdk()
	var events = gdk.get_events()
	if events == null:
		return

	var generated = events.get_play_session_id()
	assert_typeof(generated, TYPE_STRING, "get_play_session_id() returns a String")
	assert_gt(generated.length(), 0, "play_session_id is auto-generated and non-empty")

	events.set_play_session_id("custom-session-1234")
	assert_eq(events.get_play_session_id(), "custom-session-1234", "set_play_session_id() stores the provided id")

	events.set_play_session_id("")
	var regenerated = events.get_play_session_id()
	assert_gt(regenerated.length(), 0, "empty set_play_session_id() regenerates a fresh GUID")
	assert_ne(regenerated, "custom-session-1234", "regenerated play_session_id differs from the custom id")
