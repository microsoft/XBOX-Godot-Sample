extends Control

## GameInput action bridge — standalone tutorial sample.
##
## Builds a GameInputActionMap programmatically (matching the Step 2
## table in the tutorial), attaches a GameInputMapper that polls every
## frame, and renders live action state + device hot-plug events.
##
## Independent of GDK / PlayFab — no sign-in flow.
##
## Source: docs/tutorials/gameinput-action-bridge.md

@onready var _runtime_status: Label = $Root/RuntimeStatus
@onready var _device_count: Label = $Root/DeviceCount
@onready var _devices: Label = $Root/Devices
@onready var _action_state: Label = $Root/ActionState
@onready var _hotplug_log: RichTextLabel = $Root/HotplugLog
@onready var _player: ColorRect = $Player

const PLAYER_SPEED := 240.0
const PLAYER_JUMP_VELOCITY := -480.0
const PLAYER_GRAVITY := 1200.0
const PLAYER_FLOOR_Y := 320.0

var _player_velocity_y: float = 0.0
var _mapper: GameInputMapper = null

func _ready() -> void:
	if not Engine.has_singleton("GameInput"):
		_runtime_status.text = "GameInput singleton missing. Build the addon (cmake --build build --preset debug)."
		_device_count.text = ""
		_devices.text = ""
		_action_state.text = ""
		return

	if not GameInput.is_initialized():
		push_warning("[Pad] GameInput runtime not available — gamepad input disabled.")
		_runtime_status.text = "GameInput runtime NOT initialized (set game_input/runtime/initialize_on_startup=true)."
	else:
		_runtime_status.text = "GameInput runtime initialized."

	_mapper = GameInputMapper.new()
	_mapper.name = "GamepadMapper"
	_mapper.action_map = _build_default_map()
	add_child(_mapper)

	GameInput.device_connected.connect(_on_device_connected)
	GameInput.device_disconnected.connect(_on_device_disconnected)

	# Seed the UI with whatever was connected before _ready.
	_refresh_devices()
	_append_hotplug("Seeded with %d device(s) at startup" % GameInput.get_connected_device_count())

	# Position the player on the floor.
	_player.position = Vector2(get_viewport_rect().size.x * 0.5, PLAYER_FLOOR_Y)

func _build_default_map() -> GameInputActionMap:
	var map := GameInputActionMap.new()

	var accept := GameInputBinding.new()
	accept.action = &"ui_accept"
	accept.source = GameInputDevice.SRC_BTN_A
	map.add_binding(accept)

	var jump := GameInputBinding.new()
	jump.action = &"jump"
	jump.source = GameInputDevice.SRC_BTN_A
	map.add_binding(jump)

	var left := GameInputBinding.new()
	left.action = &"move_left"
	left.source = GameInputDevice.SRC_AXIS_LEFT_X
	left.is_axis = true
	left.axis_invert = true
	map.add_binding(left)

	var right := GameInputBinding.new()
	right.action = &"move_right"
	right.source = GameInputDevice.SRC_AXIS_LEFT_X
	right.is_axis = true
	map.add_binding(right)

	return map

func _physics_process(delta: float) -> void:
	if not Engine.has_singleton("GameInput"):
		return
	var direction: float = Input.get_action_strength("move_right") - Input.get_action_strength("move_left")
	_player.position.x += direction * PLAYER_SPEED * delta

	if Input.is_action_just_pressed("jump") and _player.position.y >= PLAYER_FLOOR_Y:
		_player_velocity_y = PLAYER_JUMP_VELOCITY

	_player_velocity_y += PLAYER_GRAVITY * delta
	_player.position.y += _player_velocity_y * delta
	if _player.position.y >= PLAYER_FLOOR_Y:
		_player.position.y = PLAYER_FLOOR_Y
		_player_velocity_y = 0.0

	var viewport_w: float = get_viewport_rect().size.x
	_player.position.x = clamp(_player.position.x, 0.0, viewport_w - _player.size.x)

func _process(_delta: float) -> void:
	if not Engine.has_singleton("GameInput"):
		return
	_action_state.text = (
			"move_left=%.2f  move_right=%.2f  jump=%s  ui_accept=%s"
			% [
				Input.get_action_strength("move_left"),
				Input.get_action_strength("move_right"),
				str(Input.is_action_pressed("jump")),
				str(Input.is_action_pressed("ui_accept")),
			]
	)

func _on_device_connected(device: GameInputDevice) -> void:
	_append_hotplug("connected: id=%d (%s)" % [device.get_device_id(), device.get_display_name()])
	_refresh_devices()

func _on_device_disconnected(device_id: int) -> void:
	_append_hotplug("disconnected: id=%d" % device_id)
	_refresh_devices()

func _refresh_devices() -> void:
	var count: int = GameInput.get_connected_device_count()
	_device_count.text = "Connected gamepads: %d" % count

	var lines := PackedStringArray()
	for device in GameInput.get_devices(GameInput.DEVICE_GAMEPAD):
		lines.append("- id=%d %s" % [device.get_device_id(), device.get_display_name()])
	if lines.is_empty():
		lines.append("- (none — plug in a gamepad)")
	_devices.text = "\n".join(lines)

func _append_hotplug(line: String) -> void:
	_hotplug_log.append_text(line + "\n")
