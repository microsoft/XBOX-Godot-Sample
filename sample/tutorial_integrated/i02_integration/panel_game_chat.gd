extends VBoxContainer

const AddonApi = preload("res://shared/addon_api.gd")

## Integration tech demo — GDK Game Chat panel (GDK.game_chat).
##
## The GDK-native voice + text chat option, shown next to the PlayFab Party
## panel so the two approaches can be compared directly:
##   - PlayFab Party (panel_party.gd) owns its own transport end-to-end.
##   - GDK Game Chat (this panel) encodes opaque data frames but builds NO
##     transport; the title ferries frames over whatever network it already
##     has. Here we feed each outgoing frame straight back in (single-process
##     loopback) so the data-frame plumbing is demonstrable without a peer.
##
## Independent voice/text control mirrors Party: the mic-mute toggle stops
## microphone audio (voice) while text chat keeps working, because Game Chat
## tracks send/receive bits for microphone, text-to-speech, and text
## separately via CommunicationRelationship flags.
##
## Source: docs/gdk/api-reference.md (Game Chat service: GDK.game_chat)

# A synthetic loopback peer. In a real title this XUID + endpoint would belong
# to a remote player reached over the title's transport.
const LOOPBACK_XUID := "2814639011419087"
const LOOPBACK_ENDPOINT := 1

@onready var _status: Label = $Status
@onready var _chat_log: RichTextLabel = $ChatLog
@onready var _chat_input: LineEdit = $ChatInput
@onready var _send: Button = $Send
@onready var _mute_mic: CheckButton = $MuteMic

var _auth: Node = null
var _chat = null
var _local_xuid: String = ""
var _initialized: bool = false

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	if _auth == null:
		_status.text = "[ERR] Auth autoload missing"
		return
	if not Engine.has_singleton("GDK"):
		_status.text = "[ERR] godot_gdk extension not loaded"
		return
	_auth.state_changed.connect(_on_auth_state_changed)
	if _auth.is_signed_in():
		_initialize_after_sign_in()
		return
	await _auth.sign_in()
	if is_inside_tree() and _auth.is_signed_in():
		_initialize_after_sign_in()

func _exit_tree() -> void:
	if _auth != null and _auth.state_changed.is_connected(_on_auth_state_changed):
		_auth.state_changed.disconnect(_on_auth_state_changed)
	if not _initialized or _chat == null:
		return
	if _chat.outgoing_data_frame.is_connected(_on_outgoing_data_frame):
		_chat.outgoing_data_frame.disconnect(_on_outgoing_data_frame)
	if _chat.text_chat_received.is_connected(_on_text_chat_received):
		_chat.text_chat_received.disconnect(_on_text_chat_received)
	if _chat.transcribed_chat_received.is_connected(_on_transcribed_chat_received):
		_chat.transcribed_chat_received.disconnect(_on_transcribed_chat_received)
	_chat.cleanup()

func _on_auth_state_changed(_state) -> void:
	if _initialized or _auth == null:
		return
	if _auth.is_signed_in():
		_initialize_after_sign_in()

func _initialize_after_sign_in() -> void:
	if _initialized:
		return
	_initialized = true
	_send.pressed.connect(_on_send_pressed)
	_mute_mic.toggled.connect(_on_mute_mic_toggled)

	_chat = AddonApi.singleton("GDK").game_chat

	var init = _chat.initialize()
	if not init.ok:
		_status.text = "game_chat.initialize() failed: %s (%s)" % [init.message, init.code]
		return

	# Loopback wiring: every frame Game Chat wants to send is fed straight back
	# in as if it had arrived from the peer. A real title would transmit it.
	_chat.outgoing_data_frame.connect(_on_outgoing_data_frame)
	_chat.text_chat_received.connect(_on_text_chat_received)
	_chat.transcribed_chat_received.connect(_on_transcribed_chat_received)

	var xbox = _auth.get("xbox_user")
	var add_local = _chat.add_local_user(xbox)
	if not add_local.ok:
		_status.text = "add_local_user() failed: %s (%s)" % [add_local.message, add_local.code]
		return
	_local_xuid = add_local.data.get("xuid", "")

	var add_remote = _chat.add_remote_user(LOOPBACK_XUID, LOOPBACK_ENDPOINT)
	if not add_remote.ok:
		_status.text = "add_remote_user() failed: %s" % add_remote.message
		return

	_status.text = "Game Chat ready — local %s + loopback peer on endpoint %d" % [
			_local_xuid.left(8), LOOPBACK_ENDPOINT]
	_chat_log.append_text("[i]No transport built: outgoing frames are looped back to the local manager.[/i]\n")

func _on_send_pressed() -> void:
	var text: String = _chat_input.text.strip_edges()
	if text.is_empty() or _chat == null or _local_xuid.is_empty():
		return
	var result = _chat.send_text(_local_xuid, text)
	if result.ok:
		_chat_log.append_text("[me] %s\n" % text)
		_chat_input.text = ""
	else:
		_chat_log.append_text("[i]send_text failed: %s (%s)[/i]\n" % [result.message, result.code])

func _on_mute_mic_toggled(button_pressed: bool) -> void:
	# Voice and text are independent: muting the mic leaves text chat working.
	if _chat == null or _local_xuid.is_empty():
		return
	var result = _chat.set_microphone_muted(_local_xuid, button_pressed)
	if result.ok:
		_chat_log.append_text("[i]microphone %s (text chat still works)[/i]\n" % [
				"muted" if button_pressed else "unmuted"])
	else:
		_chat_log.append_text("[i]set_microphone_muted failed: %s[/i]\n" % result.message)

func _on_outgoing_data_frame(target_endpoint_ids: PackedInt64Array, bytes: PackedByteArray, _transport_requirement: int) -> void:
	# A real title would deliver these bytes to each endpoint over its transport.
	# Loopback: hand them right back to Game Chat as inbound frames.
	for endpoint in target_endpoint_ids:
		_chat.process_incoming_data_frame(endpoint, bytes)
	_chat_log.append_text("[frame] looped %d byte(s) back to %d endpoint(s)\n" % [
			bytes.size(), target_endpoint_ids.size()])

func _on_text_chat_received(sender_xuid: String, message: String) -> void:
	_chat_log.append_text("[%s] %s\n" % [sender_xuid.left(8), message])

func _on_transcribed_chat_received(speaker_xuid: String, message: String) -> void:
	_chat_log.append_text("[%s ~voice] %s\n" % [speaker_xuid.left(8), message])
