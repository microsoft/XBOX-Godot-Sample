extends Control

const DEFAULT_PORT = 8910

@onready var address: LineEdit = $Address
@onready var host_button: Button = $HostButton
@onready var join_button: Button = $JoinButton
@onready var single_player_button: Button = $SinglePlayerButton
@onready var sign_in_button: Button = $SignInButton
@onready var xbox_status_label: Label = $XboxStatusLabel
@onready var status_ok: Label = $StatusOk
@onready var status_fail: Label = $StatusFail
@onready var port_forward_label: Label = $PortForward
@onready var find_public_ip_button: LinkButton = $FindPublicIP

var peer: ENetMultiplayerPeer
var _title_hue := 0.0
var _signed_in := false
var _sign_in_op = null

func _ready() -> void:
	multiplayer.peer_connected.connect(_player_connected)
	multiplayer.peer_disconnected.connect(_player_disconnected)
	multiplayer.connected_to_server.connect(_connected_ok)
	multiplayer.connection_failed.connect(_connected_fail)
	multiplayer.server_disconnected.connect(_server_disconnected)

	# Dark neon background (deferred to avoid add_child during parent _ready)
	call_deferred("_setup_visuals")

	# Defer Xbox setup so GDKBootstrap has time to initialize
	call_deferred("_setup_xbox")

	_setup_gameinput()


func _setup_visuals() -> void:
	var bg = get_parent().get_node_or_null("Background")
	if bg == null:
		bg = ColorRect.new()
		bg.name = "Background"
		bg.set_anchors_preset(PRESET_FULL_RECT)
		bg.color = Color(0.05, 0.05, 0.12, 1.0)
		bg.z_index = -1
		# Decorative only — must not eat clicks intended for the panel buttons.
		bg.mouse_filter = Control.MOUSE_FILTER_IGNORE
		get_parent().add_child(bg)
		# Also move behind existing siblings so it doesn't render on top of them.
		get_parent().move_child(bg, 0)

	var title_label = get_parent().get_node_or_null("Title")
	if title_label:
		title_label.add_theme_font_size_override("font_size", 40)


func _setup_xbox() -> void:
	var gdk = _get_gdk()
	if gdk != null:
		gdk.users.user_added.connect(_on_user_added)
		gdk.users.user_removed.connect(_on_user_removed)
		gdk.users.primary_user_changed.connect(_on_primary_user_changed)
		gdk.initialized.connect(_refresh_xbox_state)

	_refresh_xbox_state()


func _get_gdk():
	# Try the bootstrap's helper first (handles extension loading)
	var bootstrap = get_node_or_null("/root/GDKBootstrap")
	if bootstrap != null and bootstrap.has_method("get_gdk"):
		return bootstrap.get_gdk()
	# Fallback to direct singleton check
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")
	return null


func _process(delta: float) -> void:
	_title_hue += delta * 0.1
	if _title_hue > 1.0:
		_title_hue -= 1.0
	var title_label = get_parent().get_node_or_null("Title")
	if title_label:
		title_label.add_theme_color_override("font_color",
			Color.from_hsv(_title_hue, 0.7, 1.0))


#region Xbox Identity
func _refresh_xbox_state() -> void:
	var gdk = _get_gdk()
	if gdk == null:
		xbox_status_label.text = "Xbox: Extension not loaded"
		xbox_status_label.add_theme_color_override("font_color", Color(0.5, 0.5, 0.5))
		sign_in_button.visible = false
		host_button.disabled = true
		join_button.disabled = true
		return

	if not gdk.is_initialized():
		xbox_status_label.text = "Xbox: Initializing..."
		xbox_status_label.add_theme_color_override("font_color", Color(0.7, 0.7, 0.3))
		sign_in_button.visible = false
		# Multiplayer disabled without Xbox, single player always works
		host_button.disabled = true
		join_button.disabled = true
		return

	var user = gdk.users.get_primary_user()
	if user != null and user.signed_in:
		_signed_in = true
		xbox_status_label.text = "Xbox: %s" % user.gamertag
		xbox_status_label.add_theme_color_override("font_color", Color(0.3, 1.0, 0.3))
		sign_in_button.visible = false
		host_button.disabled = false
		join_button.disabled = false
	else:
		_signed_in = false
		xbox_status_label.text = "Xbox: Not signed in"
		xbox_status_label.add_theme_color_override("font_color", Color(1.0, 0.7, 0.3))
		sign_in_button.visible = true
		sign_in_button.disabled = false
		host_button.disabled = true
		join_button.disabled = true


func _on_sign_in_pressed() -> void:
	var gdk = _get_gdk()
	if gdk == null or not gdk.is_initialized():
		return

	sign_in_button.disabled = true
	sign_in_button.text = "Signing in..."

	_sign_in_op = gdk.users.add_user_with_ui_async()
	if _sign_in_op == null:
		sign_in_button.text = "Sign In to Xbox"
		sign_in_button.disabled = false
		return

	if _sign_in_op.is_done():
		_on_sign_in_completed(_sign_in_op.get_result())
	else:
		_sign_in_op.completed.connect(_on_sign_in_completed)


func _on_sign_in_completed(result) -> void:
	if result != null and result.ok:
		_set_status("Signed in!", true)
	else:
		var msg = result.message if result != null else "Sign-in failed"
		_set_status(msg, false)
	_refresh_xbox_state()
	sign_in_button.text = "Sign In to Xbox"


func _on_user_added(_user) -> void:
	_refresh_xbox_state()


func _on_user_removed(_local_id: int) -> void:
	_refresh_xbox_state()


func _on_primary_user_changed(_user) -> void:
	_refresh_xbox_state()
#endregion

#region Network callbacks
func _player_connected(_id: int) -> void:
	_start_game(false)


func _player_disconnected(_id: int) -> void:
	if multiplayer.is_server():
		_end_game("Client disconnected.")
	else:
		_end_game("Server disconnected.")


func _connected_ok() -> void:
	pass


func _connected_fail() -> void:
	_set_status("Couldn't connect.", false)
	multiplayer.set_multiplayer_peer(null)
	_set_buttons_disabled(false)


func _server_disconnected() -> void:
	_end_game("Server disconnected.")
#endregion

#region Game creation
func _start_game(single_player: bool) -> void:
	var pong: Node2D = load("res://pong.tscn").instantiate()
	pong.is_single_player = single_player
	pong.game_finished.connect(_end_game, CONNECT_DEFERRED)
	get_tree().get_root().add_child(pong)
	get_parent().hide()


func _end_game(with_error: String = "") -> void:
	if has_node(^"/root/Pong"):
		get_node(^"/root/Pong").free()
		get_parent().show()

	multiplayer.set_multiplayer_peer(null)
	_set_buttons_disabled(false)
	_set_status(with_error, false)
	port_forward_label.visible = false
	find_public_ip_button.visible = false
	get_window().title = ProjectSettings.get_setting("application/config/name")


func _set_buttons_disabled(disabled: bool) -> void:
	# Single player is always available
	single_player_button.set_disabled(disabled)
	# Multiplayer requires Xbox sign-in
	if _signed_in:
		host_button.set_disabled(disabled)
		join_button.set_disabled(disabled)
	else:
		host_button.set_disabled(true)
		join_button.set_disabled(true)


func _set_status(text: String, is_ok: bool) -> void:
	if is_ok:
		status_ok.set_text(text)
		status_fail.set_text("")
	else:
		status_ok.set_text("")
		status_fail.set_text(text)


func _on_single_player_pressed() -> void:
	_set_buttons_disabled(true)
	_start_game(true)
	get_window().title = ProjectSettings.get_setting("application/config/name") + ": Single Player"


func _on_host_pressed() -> void:
	if not _signed_in:
		_set_status("Sign in to Xbox first.", false)
		return

	peer = ENetMultiplayerPeer.new()
	var err = peer.create_server(DEFAULT_PORT, 1)
	if err != OK:
		_set_status("Can't host, address in use.", false)
		return
	peer.get_host().compress(ENetConnection.COMPRESS_RANGE_CODER)

	multiplayer.set_multiplayer_peer(peer)
	_set_buttons_disabled(true)
	_set_status("Waiting for player...", true)
	get_window().title = ProjectSettings.get_setting("application/config/name") + ": Server"

	port_forward_label.visible = true
	find_public_ip_button.visible = true


func _on_join_pressed() -> void:
	if not _signed_in:
		_set_status("Sign in to Xbox first.", false)
		return

	var ip = address.get_text()
	if not ip.is_valid_ip_address():
		_set_status("IP address is invalid.", false)
		return

	peer = ENetMultiplayerPeer.new()
	peer.create_client(ip, DEFAULT_PORT)
	peer.get_host().compress(ENetConnection.COMPRESS_RANGE_CODER)
	multiplayer.set_multiplayer_peer(peer)

	_set_status("Connecting...", true)
	get_window().title = ProjectSettings.get_setting("application/config/name") + ": Client"
#endregion

func _on_find_public_ip_pressed() -> void:
	OS.shell_open("https://icanhazip.com/")


#region GameInput hot-plug surface
func _setup_gameinput() -> void:
	if not Engine.has_singleton("GameInput"):
		return
	var gi = Engine.get_singleton("GameInput")
	if not gi.device_connected.is_connected(_on_gameinput_device_connected):
		gi.device_connected.connect(_on_gameinput_device_connected)
	if not gi.device_disconnected.is_connected(_on_gameinput_device_disconnected):
		gi.device_disconnected.connect(_on_gameinput_device_disconnected)


func _on_gameinput_device_connected(device) -> void:
	if device == null:
		return
	var name: String = ""
	if device.has_method("get_display_name"):
		name = str(device.get_display_name())
	if name == "":
		name = "Controller"
	_set_status("Controller connected: %s" % name, true)


func _on_gameinput_device_disconnected(_device_id: int) -> void:
	_set_status("Controller disconnected.", false)
#endregion
