extends Area2D
## Simple AI paddle that tracks the ball position with a reaction delay.
##
## Tunables are exported so the roguelike's wave director and bosses can
## reconfigure a freshly-instantiated paddle without subclassing.

@export var motion_speed: float = 250.0
@export var reaction_distance: float = 10.0
@export var height_multiplier: float = 1.0
@export var ball_node_path: NodePath = NodePath("../Ball")
## How many pixels the AI may misread the ball's vertical position by. Re-rolls
## every `tracking_jitter_period` seconds so the AI feels lifelike instead of
## frame-perfect. 0 = perfect tracking (boss default).
@export var tracking_jitter: float = 0.0
@export var tracking_jitter_period: float = 0.4

var _motion := 0.0
var _screen_size_y := 0.0
var _aim_offset := 0.0
var _aim_timer := 0.0


func _ready() -> void:
	_screen_size_y = get_viewport_rect().size.y
	# Reconnect the collision signal since set_script() may drop it.
	if not area_entered.is_connected(_on_paddle_area_enter):
		area_entered.connect(_on_paddle_area_enter)
	if not is_equal_approx(height_multiplier, 1.0):
		scale.y = height_multiplier
	# Seed the first aim offset so the AI doesn't start with a perfectly accurate
	# read and then "jitter into" being worse a second later.
	_roll_aim_offset()


func _process(delta: float) -> void:
	if _screen_size_y == 0.0:
		_screen_size_y = get_viewport_rect().size.y
	var ball := _resolve_ball()
	if ball == null:
		return
	if tracking_jitter > 0.0:
		_aim_timer -= delta
		if _aim_timer <= 0.0:
			_roll_aim_offset()
	var target_y: float = ball.position.y + _aim_offset
	var diff: float = target_y - position.y
	if absf(diff) > reaction_distance:
		_motion = signf(diff) * motion_speed
	else:
		_motion = 0.0
	translate(Vector2(0.0, _motion * delta))
	var half_height := 16.0 * height_multiplier
	position.y = clampf(position.y, half_height, _screen_size_y - half_height)


func _roll_aim_offset() -> void:
	if tracking_jitter <= 0.0:
		_aim_offset = 0.0
	else:
		_aim_offset = randf_range(-tracking_jitter, tracking_jitter)
	_aim_timer = tracking_jitter_period


func _resolve_ball() -> Node2D:
	var node := get_node_or_null(ball_node_path)
	if node is Node2D:
		return node as Node2D
	# Fallback: pick the first sibling with a ball.gd script (so The Twins'
	# extra ball still gets tracked even if the path is misconfigured).
	var parent := get_parent()
	if parent == null:
		return null
	for child in parent.get_children():
		var script: Script = (child as Node).get_script() if child is Node else null
		if child is Node2D and script != null and script.resource_path.ends_with("ball.gd"):
			return child as Node2D
	return null


func _on_paddle_area_enter(area: Area2D) -> void:
	area.bounce(false, randf())
	var pong = get_parent()
	if pong != null and pong.has_method("on_paddle_hit"):
		pong.on_paddle_hit(self, area)
