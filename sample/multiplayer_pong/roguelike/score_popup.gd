extends Node2D
## Floating score popup for Pong Royale.
##
## Spawned by the run controller / HUD at the ball position; tweens up and
## fades out, then frees itself. Pure script — no .tscn needed.
##
## Usage:
##   const ScorePopup = preload("res://roguelike/score_popup.gd")
##   ScorePopup.spawn(arena_node, ball.position, "+250", color, big_pop=false)

const RISE_DISTANCE := 36.0
const LIFETIME := 0.85
const FONT_SIZE_DEFAULT := 18
const FONT_SIZE_BIG := 28

var _text: String = ""
var _color: Color = Color.WHITE
var _big: bool = false


static func spawn(
		parent: Node,
		at: Vector2,
		text: String,
		color: Color,
		big: bool = false) -> Node2D:
	var s := load("res://roguelike/score_popup.gd").new() as Node2D
	s.position = at
	s._text = text
	s._color = color
	s._big = big
	parent.add_child(s)
	return s


func _ready() -> void:
	var label := Label.new()
	label.text = _text
	var font_size: int = FONT_SIZE_BIG if _big else FONT_SIZE_DEFAULT
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", _color)
	label.add_theme_constant_override("outline_size", 4)
	label.add_theme_color_override("font_outline_color", Color(0.05, 0.08, 0.05, 1.0))
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.size = Vector2(140, 32)
	label.position = Vector2(-70, -16)
	label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(label)

	z_index = 50
	var tween := create_tween().set_parallel(true)
	tween.tween_property(self, "position:y", position.y - RISE_DISTANCE, LIFETIME) \
			.set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_OUT)
	tween.tween_property(label, "modulate:a", 0.0, LIFETIME) \
			.set_trans(Tween.TRANS_LINEAR)
	if _big:
		var bump := create_tween()
		bump.tween_property(self, "scale", Vector2(1.25, 1.25), 0.12) \
				.set_trans(Tween.TRANS_BACK).set_ease(Tween.EASE_OUT)
		bump.tween_property(self, "scale", Vector2(1.0, 1.0), 0.18) \
				.set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_IN)
	get_tree().create_timer(LIFETIME + 0.05).timeout.connect(queue_free)
