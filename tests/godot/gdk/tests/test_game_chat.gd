extends "res://addons/godot_gdk_tests/gdk_test_base.gd"

# Game Chat 2 (GDK.game_chat) coverage. Game Chat encodes its own opaque data
# frames and does not pick a transport; the wrapper exposes that surface via the
# outgoing_data_frame signal + process_incoming_data_frame. Real voice/text
# delivery needs signed-in users and multi-machine audio, so headless runs cover
# the service surface, enum constants, lifecycle, and argument validation; the
# signed-in loopback path degrades to pending on hosts without an Xbox user.

const REMOTE_XUID := "2814639011419087"
const REMOTE_ENDPOINT := 1001


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	var gdk = get_gdk()
	if gdk != null:
		var game_chat = gdk.get_game_chat()
		if game_chat != null and game_chat.is_initialized():
			game_chat.cleanup()
	reset_runtime()


func test_game_chat_surface_and_constants() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var game_chat = gdk.get_game_chat()
	assert_not_null(game_chat, "GDK.game_chat returns service object")
	if game_chat == null:
		return

	for method_name in [
		"initialize",
		"is_initialized",
		"cleanup",
		"add_local_user",
		"add_remote_user",
		"remove_user",
		"set_communication_relationship",
		"set_microphone_muted",
		"set_remote_user_muted",
		"set_audio_render_volume",
		"send_text",
		"synthesize_text_to_speech",
		"process_incoming_data_frame",
		"get_chat_users",
	]:
		assert_has_method_named(game_chat, method_name)

	for signal_name in [
		"outgoing_data_frame",
		"text_chat_received",
		"transcribed_chat_received",
	]:
		assert_has_signal_named(game_chat, signal_name)

	# Communication-relationship flags compose as documented in GDKGameChat.xml.
	assert_eq(get_class_constant("GDKGameChat", "RELATIONSHIP_NONE"), 0, "RELATIONSHIP_NONE == 0")
	assert_eq(get_class_constant("GDKGameChat", "RELATIONSHIP_SEND_ALL"), 7, "RELATIONSHIP_SEND_ALL == 7")
	assert_eq(get_class_constant("GDKGameChat", "RELATIONSHIP_RECEIVE_ALL"), 24, "RELATIONSHIP_RECEIVE_ALL == 24")
	assert_eq(get_class_constant("GDKGameChat", "RELATIONSHIP_SEND_AND_RECEIVE_ALL"), 31, "RELATIONSHIP_SEND_AND_RECEIVE_ALL == 31")
	assert_eq(get_class_constant("GDKGameChat", "TRANSPORT_GUARANTEED"), 0, "TRANSPORT_GUARANTEED == 0")
	assert_eq(get_class_constant("GDKGameChat", "TRANSPORT_BEST_EFFORT"), 1, "TRANSPORT_BEST_EFFORT == 1")
	assert_eq(get_class_constant("GDKGameChat", "CHAT_INDICATOR_SILENT"), 0, "CHAT_INDICATOR_SILENT == 0")
	assert_eq(get_class_constant("GDKGameChat", "CHAT_INDICATOR_NO_MICROPHONE"), 7, "CHAT_INDICATOR_NO_MICROPHONE == 7")


func test_game_chat_rejects_calls_before_initialize() -> void:
	if pending_unless_runtime_available():
		return

	var game_chat = get_gdk().get_game_chat()
	if game_chat == null:
		return

	assert_false(game_chat.is_initialized(), "is_initialized() is false before initialize()")
	assert_eq(game_chat.get_chat_users().size(), 0, "get_chat_users() is empty before initialize()")

	assert_result_error(game_chat.initialize(), "not_initialized", "initialize() requires GDK.initialize() first")
	assert_result_error(game_chat.add_remote_user(REMOTE_XUID, REMOTE_ENDPOINT), "not_initialized", "add_remote_user() requires initialize()")
	assert_result_error(game_chat.remove_user(REMOTE_XUID), "not_initialized", "remove_user() requires initialize()")
	assert_result_error(game_chat.send_text(REMOTE_XUID, "hello"), "not_initialized", "send_text() requires initialize()")
	assert_result_error(game_chat.process_incoming_data_frame(REMOTE_ENDPOINT, PackedByteArray([1, 2, 3])), "not_initialized", "process_incoming_data_frame() requires initialize()")


func test_game_chat_lifecycle_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for game chat returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Game chat runtime behavior: %s" % init_result.message)
		return

	var game_chat = get_gdk().get_game_chat()
	if game_chat == null:
		return

	# Lifecycle: initialize -> already_initialized -> cleanup -> invalid_max_users -> re-initialize.
	assert_false(game_chat.is_initialized(), "is_initialized() is false until initialize() succeeds")
	assert_result_ok(game_chat.initialize(), "initialize() succeeds once the runtime is ready")
	assert_true(game_chat.is_initialized(), "is_initialized() is true after initialize()")
	assert_result_error(game_chat.initialize(), "already_initialized", "initialize() is rejected while already initialized")

	game_chat.cleanup()
	assert_false(game_chat.is_initialized(), "is_initialized() is false after cleanup()")
	assert_result_error(game_chat.initialize(0), "invalid_max_users", "initialize() rejects non-positive max_users")
	assert_result_ok(game_chat.initialize(), "initialize() succeeds again after cleanup()")

	# Remote users can be added without sign-in (they carry no local audio).
	assert_result_error(game_chat.add_remote_user("", REMOTE_ENDPOINT), "invalid_xuid", "add_remote_user() rejects an empty xuid")
	var add_remote = game_chat.add_remote_user(REMOTE_XUID, REMOTE_ENDPOINT)
	assert_result_ok(add_remote, "add_remote_user() adds a remote chat user")
	if add_remote.ok:
		assert_dict_has_key(add_remote.data, "xuid", "add_remote_user() data exposes xuid")
		assert_dict_has_key(add_remote.data, "endpoint_id", "add_remote_user() data exposes endpoint_id")

	var users = game_chat.get_chat_users()
	var found_remote := false
	for entry in users:
		if entry.get("xuid", "") == REMOTE_XUID:
			found_remote = true
			assert_false(entry.get("is_local", true), "remote chat user reports is_local == false")
			assert_dict_has_key(entry, "chat_indicator", "chat user entry exposes chat_indicator")
	assert_true(found_remote, "get_chat_users() lists the added remote user")

	# Argument validation against an initialized manager.
	var send_all := get_class_constant("GDKGameChat", "RELATIONSHIP_SEND_ALL")
	assert_result_error(game_chat.set_communication_relationship("unknown-local", REMOTE_XUID, send_all), "user_not_found", "set_communication_relationship() rejects an unknown local user")
	assert_result_error(game_chat.set_microphone_muted("unknown-local", true), "user_not_found", "set_microphone_muted() rejects an unknown local user")
	assert_result_error(game_chat.set_microphone_muted(REMOTE_XUID, true), "not_local_user", "set_microphone_muted() rejects a remote user")
	assert_result_error(game_chat.send_text(REMOTE_XUID, ""), "invalid_text", "send_text() rejects empty text")
	assert_result_error(game_chat.send_text("unknown-local", "hello"), "user_not_found", "send_text() rejects an unknown local user")
	assert_result_error(game_chat.process_incoming_data_frame(REMOTE_ENDPOINT, PackedByteArray()), "invalid_data", "process_incoming_data_frame() rejects an empty payload")

	assert_result_error(game_chat.remove_user("unknown-user"), "user_not_found", "remove_user() rejects an unknown user")
	assert_result_ok(game_chat.remove_user(REMOTE_XUID), "remove_user() removes the remote user")
	assert_eq(game_chat.get_chat_users().size(), 0, "get_chat_users() is empty after removing the remote user")

	game_chat.cleanup()


func test_game_chat_text_loopback_when_user_available() -> void:
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Game chat runtime unavailable")
		return

	var game_chat = get_gdk().get_game_chat()
	if game_chat == null:
		return

	var sign_in = await ensure_primary_user()
	var user = sign_in["user"]
	if user == null:
		pending("Game chat loopback coverage: no signed-in Xbox user is available on this machine.")
		return

	assert_result_ok(game_chat.initialize(), "initialize() succeeds for loopback coverage")

	var add_local = game_chat.add_local_user(user)
	assert_result_ok(add_local, "add_local_user() adds the signed-in user")
	if not add_local.ok:
		game_chat.cleanup()
		return
	var local_xuid: String = add_local.data.get("xuid", "")

	assert_result_ok(game_chat.add_remote_user(REMOTE_XUID, REMOTE_ENDPOINT), "add_remote_user() adds the loopback peer")

	var captured_frames: Array = []
	var on_frame := func(target_endpoint_ids, bytes, transport_requirement):
		captured_frames.append({
			"targets": target_endpoint_ids,
			"bytes": bytes,
			"transport": transport_requirement,
		})
	game_chat.outgoing_data_frame.connect(on_frame)

	assert_result_ok(game_chat.send_text(local_xuid, "loopback hello"), "send_text() queues a chat text frame")

	# The per-frame pump runs via GDK.dispatch() on the embedded frame callback;
	# give it a few frames to surface the encoded outgoing data frame.
	for _i in range(30):
		await get_tree().process_frame
		if not captured_frames.is_empty():
			break

	game_chat.outgoing_data_frame.disconnect(on_frame)

	if captured_frames.is_empty():
		pending("Game chat did not emit an outgoing data frame for the local user on this host.")
		game_chat.cleanup()
		return

	# Loopback: feed the captured frame back in as if it arrived from the peer.
	var frame: Dictionary = captured_frames[0]
	assert_true(frame["bytes"].size() > 0, "outgoing_data_frame carries a non-empty payload")

	var received: Array = []
	var on_text := func(sender_xuid, message):
		received.append({ "sender": sender_xuid, "message": message })
	game_chat.text_chat_received.connect(on_text)

	var round_trip = game_chat.process_incoming_data_frame(REMOTE_ENDPOINT, frame["bytes"])
	assert_result_ok(round_trip, "process_incoming_data_frame() accepts a looped-back frame")

	# Pump a few frames so the state-change pump surfaces text_chat_received.
	for _i in range(30):
		await get_tree().process_frame
		if not received.is_empty():
			break

	game_chat.text_chat_received.disconnect(on_text)

	assert_false(received.is_empty(), "text_chat_received fires for the looped-back text frame")
	if not received.is_empty():
		assert_eq(received[0]["message"], "loopback hello", "text_chat_received delivers the original message text")
		assert_false(String(received[0]["sender"]).is_empty(), "text_chat_received reports a non-empty sender XUID")

	game_chat.cleanup()
