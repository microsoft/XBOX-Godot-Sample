extends RefCounted
## Wave configuration / difficulty curve for Pong Royale's roguelike run.
##
## v2 — score-target driven (Balatro "blind" semantics). Each wave returns a
## target score the player must hit by chaining paddle hits and rally wins.
## Boss waves apply a target multiplier and inherit a unique mechanic.
##
## Wave layout:
##   * Every 3rd wave (3, 6, 9, ...) is a boss.
##   * Boss rotation cycles through The Wall → The Twins → Speed Demon.
##   * Non-boss waves scale AI speed/reaction smoothly.
##   * Target score grows superlinearly so modifier picks compound.

const Bosses = preload("res://roguelike/bosses/boss_base.gd")
const TheWall = preload("res://roguelike/bosses/the_wall.gd")
const TheTwins = preload("res://roguelike/bosses/the_twins.gd")
const SpeedDemon = preload("res://roguelike/bosses/speed_demon.gd")

const WAVES_PER_BOSS := 3
const BASE_TARGET := 80
const BASE_BALL_SPEED := 220.0
const BALL_SPEED_PER_WAVE := 8.0
const MAX_BALL_SPEED := 360.0


static func is_boss_wave(wave_index: int) -> bool:
	return wave_index > 0 and wave_index % WAVES_PER_BOSS == 0


static func boss_for_wave(wave_index: int) -> Bosses:
	# 1st boss = The Wall (wave 3), 2nd = The Twins (6), 3rd = Speed Demon (9),
	# then loop. Returns a fresh boss config instance per call.
	var boss_index: int = (wave_index / WAVES_PER_BOSS - 1) % 3
	match boss_index:
		0: return TheWall.new()
		1: return TheTwins.new()
		_: return SpeedDemon.new()


static func _target_for_wave(wave_index: int) -> int:
	# Balatro-ish curve: BASE * (wave * 0.85)^1.35
	# wave 1 -> ~80, wave 2 -> ~165, wave 3 -> ~265, wave 5 -> ~520, wave 9 -> ~1100
	var w: float = maxf(1.0, float(wave_index))
	var raw: float = float(BASE_TARGET) * pow(w * 0.85, 1.35)
	return int(round(raw / 5.0) * 5)


static func _start_speed_for_wave(wave_index: int) -> float:
	return minf(BASE_BALL_SPEED + float(wave_index - 1) * BALL_SPEED_PER_WAVE, MAX_BALL_SPEED)


static func config_for_wave(wave_index: int) -> Dictionary:
	if is_boss_wave(wave_index):
		var boss := boss_for_wave(wave_index)
		var cfg := boss.to_wave_config(wave_index)
		var base_target: int = _target_for_wave(wave_index)
		cfg["target_score"] = int(round(base_target * float(cfg.get("target_multiplier", 2.5))))
		cfg["ball_start_speed"] = _start_speed_for_wave(wave_index)
		return cfg
	# Normal wave — scale paddle speed + reaction. Wave 1 starts intentionally
	# soft so newcomers can rack up combos and reach the first boss; we ramp up
	# pretty hard by the time bosses get harder, so the late-game still bites.
	var diff_t := clampf(float(wave_index) / 14.0, 0.0, 1.0)
	var paddle_speed: float = lerpf(135.0, 285.0, diff_t)
	var reaction: float = lerpf(22.0, 6.0, diff_t)
	var jitter: float = lerpf(80.0, 6.0, diff_t)
	return {
		"is_boss": false,
		"wave_index": wave_index,
		"boss": null,
		"target_score": _target_for_wave(wave_index),
		"label": "WAVE %02d" % wave_index,
		"paddle_speed": paddle_speed,
		"paddle_reaction": reaction,
		"paddle_height_multiplier": 1.0,
		"paddle_tracking_jitter": jitter,
		"ball_bounce_multiplier": 1.1,
		"ball_passive_acceleration": 1.2,
		"ball_start_speed": _start_speed_for_wave(wave_index),
		"twin_balls": false,
		"target_multiplier": 1.0,
		"tagline": "",
	}
