extends "res://roguelike/bosses/boss_base.gd"
## Boss 2 — The Twins.
##
## Two balls in play simultaneously. The AI paddle is normal-sized but a
## little quicker; managing both balls is the actual challenge.

func _init() -> void:
	id = "the_twins"
	display_name = "THE TWINS"
	win_rallies = 5
	paddle_speed = 240.0
	paddle_reaction = 10.0
	paddle_tracking_jitter = 14.0
	paddle_height_multiplier = 1.0
	ball_bounce_multiplier = 1.1
	ball_passive_acceleration = 0.8
	twin_balls = true
	target_multiplier = 2.5
	tagline = "DOUBLE TROUBLE"
