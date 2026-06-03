extends Node2D

signal game_finished()

const SCORE_TO_WIN = 10
const AI_PADDLE_SCRIPT = preload("res://logic/ai_paddle.gd")
const SHAKE_DURATION = 0.2
const SHAKE_STRENGTH = 6.0

const HIT_RUMBLE_LOW = 0.35
const HIT_RUMBLE_HIGH = 0.15
const HIT_RUMBLE_DURATION = 0.08
const SCORE_RUMBLE_LOW = 0.7
const SCORE_RUMBLE_HIGH = 0.4
const SCORE_RUMBLE_DURATION = 0.25

var score_left := 0
var score_right := 0
var is_single_player := false
var _shake_timer := 0.0
var _speed_label_alpha := 0.0

@onready var player2: Area2D = $Player2
@onready var score_left_node: Label = $ScoreLeft
@onready var score_right_node: Label = $ScoreRight
@onready var winner_left: Label = $WinnerLeft
@onready var winner_right: Label = $WinnerRight
@onready var camera: Camera2D = $Camera2D
@onready var speed_label: Label = $SpeedLabel

func _ready() -> void:
	# Style the score labels
	score_left_node.add_theme_font_size_override("font_size", 48)
	score_left_node.add_theme_color_override("font_color", Color(0.0, 1.0, 1.0, 0.8))
	score_right_node.add_theme_font_size_override("font_size", 48)
	score_right_node.add_theme_color_override("font_color", Color(1.0, 0.0, 1.0, 0.8))

	winner_left.add_theme_font_size_override("font_size", 24)
	winner_left.add_theme_color_override("font_color", Color(0.0, 1.0, 1.0))
	winner_right.add_theme_font_size_override("font_size", 24)
	winner_right.add_theme_color_override("font_color", Color(1.0, 0.0, 1.0))

	# Create speed indicator
	speed_label.add_theme_font_size_override("font_size", 12)
	speed_label.add_theme_color_override("font_color", Color(1.0, 1.0, 1.0, 0.3))

	if is_single_player:
		player2.set_script(AI_PADDLE_SCRIPT)
		if player2.has_node("You"):
			player2.get_node("You").hide()
	else:
		if multiplayer.is_server():
			player2.set_multiplayer_authority(multiplayer.get_peers()[0])
		else:
			player2.set_multiplayer_authority(multiplayer.get_unique_id())

	print("Unique id: ", multiplayer.get_unique_id())


func _process(delta: float) -> void:
	# Screen shake decay
	if _shake_timer > 0:
		_shake_timer -= delta
		var shake_amount := SHAKE_STRENGTH * (_shake_timer / SHAKE_DURATION)
		camera.offset = Vector2(320 + randf_range(-shake_amount, shake_amount),
								200 + randf_range(-shake_amount, shake_amount))
	else:
		camera.offset = Vector2(320, 200)

	# Speed indicator
	var ball = $Ball
	if ball and not ball.stopped:
		speed_label.text = "SPEED: %.0f" % ball._speed
		var intensity := clampf((ball._speed - 100.0) / 200.0, 0.0, 1.0)
		speed_label.add_theme_color_override("font_color",
			Color(1.0, 1.0 - intensity * 0.7, 1.0 - intensity, 0.4 + intensity * 0.3))


func _shake() -> void:
	_shake_timer = SHAKE_DURATION


@rpc("any_peer", "call_local")
func update_score(add_to_left: bool) -> void:
	_shake()
	pulse_rumble(SCORE_RUMBLE_LOW, SCORE_RUMBLE_HIGH, SCORE_RUMBLE_DURATION)

	if add_to_left:
		score_left += 1
		score_left_node.set_text(str(score_left))
		_pop_label(score_left_node)
	else:
		score_right += 1
		score_right_node.set_text(str(score_right))
		_pop_label(score_right_node)

	var game_ended: bool = false
	if score_left == SCORE_TO_WIN:
		winner_left.show()
		game_ended = true
	elif score_right == SCORE_TO_WIN:
		winner_right.show()
		game_ended = true

	if game_ended:
		$ExitGame.show()
		if is_single_player:
			$Ball.stop()
		else:
			$Ball.stop.rpc()


func pulse_rumble(low: float, high: float, duration: float) -> void:
	# Primary-device rumble pulse via godot_gameinput. Soft-fails when GameInput
	# is unavailable / not initialized, when no device is connected, or when the
	# device doesn't support vibration. Pong stays fully playable in either case.
	if not Engine.has_singleton("GameInput"):
		return
	var gi = Engine.get_singleton("GameInput")
	if not gi.is_initialized():
		return
	gi.poll()
	var device = gi.get_primary_device()
	if device == null or not device.supports_vibration():
		return
	gi.set_vibration(device, low, high)
	await get_tree().create_timer(duration).timeout
	# Re-resolve in case the device hot-unplugged during the pulse.
	var still = gi.get_primary_device()
	if still != null:
		gi.stop_haptics(still)


func _pop_label(label: Label) -> void:
	var tween := create_tween()
	label.scale = Vector2(2.0, 2.0)
	tween.tween_property(label, "scale", Vector2.ONE, 0.3).set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_ELASTIC)


func _on_exit_game_pressed() -> void:
	game_finished.emit()
