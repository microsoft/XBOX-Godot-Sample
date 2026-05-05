extends Area2D

const DEFAULT_SPEED = 100.0
const TRAIL_LENGTH = 8

@export var bounce_speed_multiplier: float = 1.1
@export var passive_acceleration: float = 1.0  # speed gained per second while in flight

var direction := Vector2.LEFT
var stopped: bool = false
var _speed := DEFAULT_SPEED
var _trail_points: Array[Vector2] = []

@onready var _screen_size := get_viewport_rect().size


func _is_local_game() -> bool:
	return multiplayer.multiplayer_peer == null or multiplayer.multiplayer_peer is OfflineMultiplayerPeer


func _process(delta: float) -> void:
	_speed += passive_acceleration * delta
	if not stopped:
		translate(_speed * delta * direction)
		_trail_points.push_front(position)
		if _trail_points.size() > TRAIL_LENGTH:
			_trail_points.resize(TRAIL_LENGTH)
	queue_redraw()

	var ball_pos := position
	if (ball_pos.y < 0 and direction.y < 0) or (ball_pos.y > _screen_size.y and direction.y > 0):
		direction.y = -direction.y

	if _is_local_game():
		if ball_pos.x < 0:
			get_parent().update_score(false)
			_reset_ball(false)
		elif ball_pos.x > _screen_size.x:
			get_parent().update_score(true)
			_reset_ball(true)
	elif is_multiplayer_authority():
		if ball_pos.x < 0:
			get_parent().update_score.rpc(false)
			_reset_ball.rpc(false)
	else:
		if ball_pos.x > _screen_size.x:
			get_parent().update_score.rpc(true)
			_reset_ball.rpc(true)


func _draw() -> void:
	for i in range(_trail_points.size()):
		var alpha := 1.0 - float(i) / float(TRAIL_LENGTH)
		var radius := 5.0 * (1.0 - float(i) / float(TRAIL_LENGTH)) + 1.0
		var trail_color := Color(1.0, 0.9, 0.3, alpha * 0.6)
		draw_circle(_trail_points[i] - position, radius, trail_color)


@rpc("any_peer", "call_local")
func bounce(left: bool, random: float) -> void:
	if left:
		direction.x = abs(direction.x)
	else:
		direction.x = -abs(direction.x)

	_speed *= bounce_speed_multiplier
	direction.y = random * 2.0 - 1
	direction = direction.normalized()

	# Flash effect on bounce
	modulate = Color(3.0, 3.0, 3.0, 1.0)
	var tween := create_tween()
	tween.tween_property(self, "modulate", Color.WHITE, 0.15)


@rpc("any_peer", "call_local")
func stop() -> void:
	stopped = true


@rpc("any_peer", "call_local")
func _reset_ball(for_left: bool) -> void:
	position = _screen_size / 2
	if for_left:
		direction = Vector2.LEFT
	else:
		direction = Vector2.RIGHT
	_speed = DEFAULT_SPEED
	_trail_points.clear()
