extends Area2D
## Simple AI paddle that tracks the ball position with a reaction delay.

const MOTION_SPEED = 150
const REACTION_DISTANCE = 40.0

var _motion := 0.0
var _screen_size_y := 0.0

func _ready() -> void:
	_screen_size_y = get_viewport_rect().size.y
	# Reconnect the collision signal since set_script() may drop it.
	if not area_entered.is_connected(_on_paddle_area_enter):
		area_entered.connect(_on_paddle_area_enter)


func _process(delta: float) -> void:
	if _screen_size_y == 0.0:
		_screen_size_y = get_viewport_rect().size.y

	var ball = get_node_or_null(^"../Ball")
	if ball == null:
		return

	var diff: float = ball.position.y - position.y
	if absf(diff) > REACTION_DISTANCE:
		_motion = signf(diff) * MOTION_SPEED
	else:
		_motion = 0.0

	translate(Vector2(0.0, _motion * delta))
	position.y = clampf(position.y, 16, _screen_size_y - 16)


func _on_paddle_area_enter(area: Area2D) -> void:
	area.bounce(false, randf())
