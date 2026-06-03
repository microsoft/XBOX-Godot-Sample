extends "res://roguelike/bosses/boss_base.gd"
## Boss 1 — The Wall.
##
## Towering AI paddle (3× height) that's slow but punishing. You have to
## thread shots into the corners.

func _init() -> void:
	id = "the_wall"
	display_name = "THE WALL"
	win_rallies = 5
	paddle_speed = 90.0
	paddle_reaction = 24.0
	paddle_height_multiplier = 3.0
	ball_bounce_multiplier = 1.08
	ball_passive_acceleration = 0.6
	twin_balls = false
	target_multiplier = 2.0
	tagline = "AIM FOR THE GAPS"
