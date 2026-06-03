extends "res://roguelike/bosses/boss_base.gd"
## Boss 3 — Speed Demon.
##
## Single ball, but every bounce ratchets the speed up aggressively. Long
## rallies become unsurvivable; you have to score quickly.

func _init() -> void:
	id = "speed_demon"
	display_name = "SPEED DEMON"
	win_rallies = 5
	paddle_speed = 290.0
	paddle_reaction = 12.0
	paddle_tracking_jitter = 8.0
	paddle_height_multiplier = 1.0
	ball_bounce_multiplier = 1.28
	ball_passive_acceleration = 1.5
	twin_balls = false
	target_multiplier = 2.8
	tagline = "DON'T BLINK"
