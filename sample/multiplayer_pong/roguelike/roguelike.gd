extends Node2D
## Pong Royale — roguelike run controller (v2: Highscore Hustle).
##
## Score-driven loop with Balatro-style modifier picks:
##   * every paddle hit scores `chips × mult` toward a wave score target
##   * combo grows on each player hit, resets when the rally ends
##   * ball off enemy side = rally bonus = `combo × 25` (combo resets)
##   * ball off our side = -1 life, combo resets, game-over at 0 lives
##   * wave clears when wave_score >= target_score → modifier pick (3 cards)
##   * boss waves apply a target multiplier and unique mechanic
##
## Reuses the existing `paddle.tscn`, `ball.tscn`, `paddle.gd`, `ball.gd`,
## and `ai_paddle.gd` from the original sample so we share rendering, trail
## effects, rumble, and goal detection. Paddle hit hooks call back into
## `on_paddle_hit(paddle, ball)` here — guarded with `has_method` so the
## original multiplayer pong scenes are unaffected.

const Palette = preload("res://theme/palette.gd")
const SaveData = preload("res://services/save_data.gd")
const CRT_SHADER = preload("res://theme/crt_shader.gdshader")
const PADDLE_SCENE = preload("res://paddle.tscn")
const BALL_SCENE = preload("res://ball.tscn")
const AI_PADDLE_SCRIPT = preload("res://logic/ai_paddle.gd")
const HUD_SCRIPT = preload("res://roguelike/hud.gd")
const WaveDirector = preload("res://roguelike/wave_director.gd")
const Modifiers = preload("res://roguelike/modifiers.gd")
const ModifierPick = preload("res://roguelike/modifier_pick.gd")
const ScorePopup = preload("res://roguelike/score_popup.gd")
const BallSkins = preload("res://roguelike/ball_skins.gd")
const Consumables = preload("res://roguelike/consumables.gd")
const ParticleFx = preload("res://roguelike/particle_fx.gd")

const VIEWPORT_SIZE := Vector2(640, 400)
const STARTING_LIVES := 3
const COUNTDOWN_FROM := 3
const COUNTDOWN_STEP := 0.4  # snappy 1.2s total intro
const COUNTDOWN_GO := 0.25

# Scoring constants
const BASE_CHIPS := 5
const CHIPS_PER_SPEED := 1.0 / 15.0
const BASE_MULT := 1.0
const COMBO_MULT_STEP := 0.1
const RALLY_BONUS_PER_COMBO := 25
const WAVE_CLEAR_BONUS_BASE := 100
const WAVE_CLEAR_BONUS_PER_WAVE := 50

const HIT_RUMBLE_LOW := 0.35
const HIT_RUMBLE_HIGH := 0.15
const HIT_RUMBLE_DURATION := 0.08
const SCORE_RUMBLE_LOW := 0.7
const SCORE_RUMBLE_HIGH := 0.4
const SCORE_RUMBLE_DURATION := 0.25
const LIFE_LOST_RUMBLE_LOW := 0.95
const LIFE_LOST_RUMBLE_HIGH := 0.65
const LIFE_LOST_RUMBLE_DURATION := 0.45

const SHAKE_DURATION := 0.2
const SHAKE_STRENGTH := 8.0
const SHAKE_BOSS := 14.0

# Consumable system
const CONSUMABLE_SLOTS := 3
const CONSUMABLE_DROP_CHANCE := 0.6
const CONSUMABLE_BOSS_DROPS := 2
const SLOW_MO_TIMESCALE := 0.45

enum State { INIT, COUNTDOWN, PLAYING, WAVE_CLEAR, MODIFIER_PICK, GAME_OVER }

# Run state
var lives: int = STARTING_LIVES
var score: int = 0
var current_wave: int = 0
var combo: int = 0
var max_combo: int = 0
var wave_score: int = 0
var target_score: int = 0
var current_config: Dictionary = {}
var state: int = State.INIT

# Modifier-driven state
var active_modifiers: Array = []
var active_modifier_ids: Array = []
var extra_balls: int = 0
var _rally_refund_pending: bool = false

# Consumable state — three HUD slots fillable via wave-clear drops.
var consumable_slots: Array = ["", "", ""]
var _aim_shot_pending: bool = false
var _mega_hit_pending: bool = false
var _freeze_until_msec: int = 0
var _slow_mo_until_msec: int = 0
var _saved_time_scale: float = 1.0

# Scene refs
var _balls: Array = []
var _player_paddle: Area2D
var _ai_paddle: Area2D
var _hud: Node
var _camera: Camera2D
var _shake_timer: float = 0.0
var _modifier_pick_node: CanvasLayer = null


func _ready() -> void:
	randomize()
	_build_arena()
	_build_hud()
	_build_crt_overlay()
	_start_wave(1)


func _process(delta: float) -> void:
	if _shake_timer > 0.0:
		_shake_timer -= delta
		var amt := SHAKE_STRENGTH * (_shake_timer / SHAKE_DURATION)
		_camera.offset = Vector2(
			VIEWPORT_SIZE.x * 0.5 + randf_range(-amt, amt),
			VIEWPORT_SIZE.y * 0.5 + randf_range(-amt, amt))
	else:
		_camera.offset = VIEWPORT_SIZE * 0.5
	_tick_consumable_timers()


func _input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_cancel"):
		get_tree().change_scene_to_file("res://title.tscn")
	elif event.is_action_pressed("use_consumable_1"):
		_use_consumable(0)
	elif event.is_action_pressed("use_consumable_2"):
		_use_consumable(1)
	elif event.is_action_pressed("use_consumable_3"):
		_use_consumable(2)


# ---------------------------------------------------------------------------
# Arena construction
# ---------------------------------------------------------------------------

func _build_arena() -> void:
	var bg := ColorRect.new()
	bg.color = Palette.BACKGROUND
	bg.size = VIEWPORT_SIZE
	bg.z_index = -100
	add_child(bg)

	var divider := Node2D.new()
	divider.z_index = -50
	_draw_divider_dashes(divider)
	add_child(divider)

	_camera = Camera2D.new()
	_camera.offset = VIEWPORT_SIZE * 0.5
	add_child(_camera)


func _draw_divider_dashes(parent: Node2D) -> void:
	var color := Palette.XBOX_GREEN_DEEP
	color.a = 0.45
	var dashes := 22
	var dx := VIEWPORT_SIZE.x * 0.5
	var dash_height := VIEWPORT_SIZE.y / dashes - 8.0
	for i in range(dashes):
		var dash := ColorRect.new()
		dash.color = color
		dash.size = Vector2(2.0, dash_height)
		dash.position = Vector2(dx - 1.0, i * VIEWPORT_SIZE.y / dashes + 4.0)
		dash.mouse_filter = Control.MOUSE_FILTER_IGNORE
		parent.add_child(dash)


func _build_hud() -> void:
	_hud = HUD_SCRIPT.new()
	add_child(_hud)


func _build_crt_overlay() -> void:
	var crt_layer := CanvasLayer.new()
	crt_layer.layer = 100
	add_child(crt_layer)
	var rect := ColorRect.new()
	rect.size = VIEWPORT_SIZE
	rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var mat := ShaderMaterial.new()
	mat.shader = CRT_SHADER
	rect.material = mat
	crt_layer.add_child(rect)


# ---------------------------------------------------------------------------
# Wave lifecycle
# ---------------------------------------------------------------------------

func _start_wave(wave_index: int) -> void:
	current_wave = wave_index
	wave_score = 0
	combo = 0
	_rally_refund_pending = false
	current_config = WaveDirector.config_for_wave(wave_index)
	target_score = int(current_config.get("target_score", 100))

	_spawn_paddles()
	_spawn_balls()
	_refresh_hud()

	if current_config.get("is_boss", false):
		_hud.show_boss(current_config.boss.display_name, current_config.boss.tagline)
	else:
		_hud.hide_boss()

	_start_countdown()


func _spawn_paddles() -> void:
	# Player paddle
	if _player_paddle != null and is_instance_valid(_player_paddle):
		_player_paddle.queue_free()
	_player_paddle = PADDLE_SCENE.instantiate()
	_player_paddle.position = Vector2(32.0, VIEWPORT_SIZE.y * 0.5)
	_player_paddle.set("left", true)
	_player_paddle.modulate = Palette.PADDLE_PLAYER
	add_child(_player_paddle)
	for mod in active_modifiers:
		mod.on_paddle_spawn(_player_paddle)

	# AI paddle — set_script BEFORE add_child so ai_paddle._ready runs.
	if _ai_paddle != null and is_instance_valid(_ai_paddle):
		_ai_paddle.queue_free()
	_ai_paddle = PADDLE_SCENE.instantiate()
	_ai_paddle.set_script(AI_PADDLE_SCRIPT)
	_ai_paddle.position = Vector2(VIEWPORT_SIZE.x - 32.0, VIEWPORT_SIZE.y * 0.5)
	_ai_paddle.modulate = Palette.PADDLE_ENEMY
	if _ai_paddle.has_node("You"):
		_ai_paddle.get_node("You").hide()
	_ai_paddle.set("motion_speed", current_config.get("paddle_speed", 250.0))
	_ai_paddle.set("reaction_distance", current_config.get("paddle_reaction", 10.0))
	_ai_paddle.set("height_multiplier", current_config.get("paddle_height_multiplier", 1.0))
	_ai_paddle.set("tracking_jitter", current_config.get("paddle_tracking_jitter", 0.0))
	add_child(_ai_paddle)


func _spawn_balls() -> void:
	for ball in _balls:
		if is_instance_valid(ball):
			ball.queue_free()
	_balls.clear()

	var primary := _make_ball("Ball")
	primary.position = VIEWPORT_SIZE * 0.5
	primary.modulate = Palette.XBOX_GREEN_GLOW
	primary.direction = _random_kickoff()
	primary.stopped = true
	add_child(primary)
	_balls.append(primary)

	if current_config.get("twin_balls", false):
		var twin := _make_ball("Ball2")
		twin.position = VIEWPORT_SIZE * 0.5 + Vector2(0.0, -40.0)
		twin.modulate = Palette.PADDLE_ENEMY
		twin.direction = _random_kickoff()
		twin.stopped = true
		add_child(twin)
		_balls.append(twin)

	# Modifier-driven extra balls (Twin Threat). Spawn alternating offsets.
	for i in range(extra_balls):
		var extra := _make_ball("Ball_extra_%d" % i)
		var sign_y := -1.0 if i % 2 == 0 else 1.0
		extra.position = VIEWPORT_SIZE * 0.5 + Vector2(0.0, 60.0 * sign_y)
		extra.modulate = Palette.XBOX_GREEN
		extra.direction = _random_kickoff()
		extra.stopped = true
		add_child(extra)
		_balls.append(extra)


func _random_kickoff() -> Vector2:
	return Vector2(1.0 if randf() > 0.5 else -1.0, randf_range(-0.5, 0.5)).normalized()


func _make_ball(node_name: String) -> Area2D:
	var ball := BALL_SCENE.instantiate()
	ball.name = node_name
	ball.set("bounce_speed_multiplier", current_config.get("ball_bounce_multiplier", 1.1))
	ball.set("passive_acceleration", current_config.get("ball_passive_acceleration", 1.0))
	# Apply start speed override (BALL_SCENE's default is 100; we want snappier).
	ball.set("_speed", float(current_config.get("ball_start_speed", 220.0)))
	# Apply the player's chosen cosmetic skin (only the primary ball is
	# skinned to avoid losing the red/green tint differentiation on twin
	# / extra balls).
	if node_name == "Ball":
		BallSkins.apply(ball, _selected_skin_id())
	for mod in active_modifiers:
		mod.on_ball_spawn(ball)
	return ball


func _selected_skin_id() -> String:
	var pf := get_node_or_null("/root/PlayFabService")
	if pf and pf.has_method("get_selected_skin"):
		return String(pf.get_selected_skin())
	return BallSkins.DEFAULT_SKIN


# ---------------------------------------------------------------------------
# Countdown / pause flow
# ---------------------------------------------------------------------------

func _start_countdown() -> void:
	state = State.COUNTDOWN
	_set_balls_stopped(true)
	var label := str(current_config.get("label", "WAVE"))
	_hud.set_status("◆  %s  ◆" % label)
	for i in range(COUNTDOWN_FROM, 0, -1):
		_hud.show_countdown(str(i))
		await get_tree().create_timer(COUNTDOWN_STEP).timeout
		if not is_inside_tree() or state != State.COUNTDOWN:
			return
	_hud.show_countdown("GO!")
	await get_tree().create_timer(COUNTDOWN_GO).timeout
	if not is_inside_tree() or state != State.COUNTDOWN:
		return
	_hud.show_countdown("")
	_hud.set_status("")
	state = State.PLAYING
	_set_balls_stopped(false)


func _set_balls_stopped(stop: bool) -> void:
	for ball in _balls:
		if is_instance_valid(ball):
			ball.stopped = stop


# ---------------------------------------------------------------------------
# Paddle hit hook — called by paddle.gd / ai_paddle.gd via has_method gate
# ---------------------------------------------------------------------------

func on_paddle_hit(paddle: Area2D, ball: Area2D) -> void:
	if state != State.PLAYING:
		return
	# Only player hits score & build combo. AI hits keep the rally alive.
	if paddle != _player_paddle:
		# Particle pop on AI returns so even AI hits feel meaty.
		ParticleFx.burst_hit(self, ball.position, Palette.PADDLE_ENEMY, 1.0)
		return

	var ball_speed := float(ball.get("_speed"))
	var base_chips: int = BASE_CHIPS + int(ceil(ball_speed * CHIPS_PER_SPEED))
	var base_mult: float = BASE_MULT + float(combo) * COMBO_MULT_STEP

	# Mega Hit consumable consumes here, before modifier hooks see the chips.
	var mega_used := false
	if _mega_hit_pending:
		base_chips *= 3
		base_mult += 1.0
		_mega_hit_pending = false
		mega_used = true

	var ctx: Dictionary = {
		"chips": base_chips,
		"mult": base_mult,
		"score": 0,
		"bonus_score": 0,
		"combo": combo,
		"ball_speed": ball_speed,
		"combo_step": 1,
	}
	for mod in active_modifiers:
		ctx["chips"] = int(ctx["chips"]) + int(mod.chip_bonus)
		ctx["mult"] = float(ctx["mult"]) + float(mod.mult_bonus)
		ctx["combo_step"] = int(ctx["combo_step"]) + int(mod.combo_step_bonus)
	# Compute base score before behavioral hooks (so on_hit can multiply it).
	ctx["score"] = int(round(float(ctx["chips"]) * float(ctx["mult"])))
	for mod in active_modifiers:
		mod.on_hit(ctx)

	# Aim Shot — override the ball's direction to fire toward the AI's farthest
	# screen edge (its current "blind side"), AFTER the random bounce in
	# paddle.gd. Locks in a steep angle that's hard to recover from.
	if _aim_shot_pending:
		var ai_y: float = _ai_paddle.position.y if _ai_paddle != null else VIEWPORT_SIZE.y * 0.5
		var aim_y: float = -0.85 if ai_y > VIEWPORT_SIZE.y * 0.5 else 0.85
		ball.direction = Vector2(absf(ball.direction.x), aim_y).normalized()
		_aim_shot_pending = false
		ParticleFx.burst_aim(self, ball.position)

	var hit_total: int = int(ctx["score"]) + int(ctx.get("bonus_score", 0))
	wave_score += hit_total
	score += hit_total
	combo += int(ctx["combo_step"])
	max_combo = maxi(max_combo, combo)
	# Cosmetic unlocks driven by big combos.
	_award_combo_unlock(combo)

	if bool(ctx.get("second_wind_refund", false)):
		_rally_refund_pending = true

	# Floating popup at the ball — extra pop for lucky / big-combo hits.
	var popup_color: Color = Palette.XBOX_GREEN_GLOW
	var big: bool = false
	if mega_used:
		popup_color = Color(1.0, 0.55, 0.95)
		big = true
	elif bool(ctx.get("lucky", false)):
		popup_color = Color(1.0, 0.78, 0.31)
		big = true
	elif combo >= 8:
		popup_color = Palette.PADDLE_PLAYER
		big = true
	ScorePopup.spawn(self, ball.position, "+%d" % hit_total, popup_color, big)
	# Particle burst — sparks fly back toward the player on player hits.
	ParticleFx.burst_hit(self, ball.position, popup_color, -1.0)
	if big:
		# Extra fountain-style burst for big hits / mega / lucky.
		ParticleFx.burst_score(self, ball.position, popup_color, true)
	_hud.pulse_combo(combo)
	_refresh_hud()
	_check_wave_progress()


# ---------------------------------------------------------------------------
# Scoring (called by Ball when it leaves the play area)
# ---------------------------------------------------------------------------

func update_score(add_to_left: bool) -> void:
	# Ball went off the LEFT edge → AI scored → player loses a life.
	# Convention from ball.gd: add_to_left=true means right-side player scored
	# (= we scored, AI conceded).
	if state != State.PLAYING:
		return
	if add_to_left:
		_on_rally_won()
	else:
		_on_rally_lost()


func _on_rally_won() -> void:
	var rally_bonus: int = combo * RALLY_BONUS_PER_COMBO
	if rally_bonus > 0:
		wave_score += rally_bonus
		score += rally_bonus
		ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5, "RALLY +%d" % rally_bonus,
				Palette.XBOX_GREEN_GLOW, true)
	if _rally_refund_pending and lives < STARTING_LIVES + _life_pickups_owned():
		# Second Wind refund — capped so it can't infinitely stack.
		lives += 1
		ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5 + Vector2(0, 26),
				"+1 LIFE", Color(0.6, 1.0, 0.6), false)
	_rally_refund_pending = false
	combo = 0
	# A celebration burst at center plus an edge pop where the ball escaped.
	ParticleFx.burst_score(self, VIEWPORT_SIZE * 0.5, Palette.XBOX_GREEN_GLOW)
	_shake()
	_pulse_rumble(SCORE_RUMBLE_LOW, SCORE_RUMBLE_HIGH, SCORE_RUMBLE_DURATION)
	_refresh_hud()
	_check_wave_progress()


func _on_rally_lost() -> void:
	lives -= 1
	for mod in active_modifiers:
		mod.on_rally_lost(self)
	combo = 0
	_rally_refund_pending = false
	_shake()
	_pulse_rumble(LIFE_LOST_RUMBLE_LOW, LIFE_LOST_RUMBLE_HIGH, LIFE_LOST_RUMBLE_DURATION)
	ParticleFx.burst_miss(self, VIEWPORT_SIZE * 0.5)
	ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5, "MISS!", Palette.TEXT_DANGER, true)
	_refresh_hud()
	if lives <= 0:
		_enter_game_over()


func _life_pickups_owned() -> int:
	# Used to cap Second Wind refunds at a reasonable ceiling.
	var n := 0
	for id in active_modifier_ids:
		if id == "steady_hand":
			n += 1
	return n + 2  # base headroom


func _check_wave_progress() -> void:
	if state != State.PLAYING:
		return
	if wave_score >= target_score:
		_on_wave_clear()


# ---------------------------------------------------------------------------
# Wave-clear / modifier pick / game-over flow
# ---------------------------------------------------------------------------

func _on_wave_clear() -> void:
	state = State.WAVE_CLEAR
	_set_balls_stopped(true)
	combo = 0
	_rally_refund_pending = false

	var base_bonus := WAVE_CLEAR_BONUS_BASE + current_wave * WAVE_CLEAR_BONUS_PER_WAVE
	var clear_mult: float = 1.0
	for mod in active_modifiers:
		clear_mult += mod.on_wave_clear(self)
	var bonus: int = int(round(float(base_bonus) * clear_mult))
	score += bonus
	if current_config.get("is_boss", false):
		_hud.set_status("◆◆  BOSS DOWN  ◆◆  +%d" % bonus)
		_shake_timer = SHAKE_DURATION
		ParticleFx.burst_celebration(self, VIEWPORT_SIZE * 0.5, Color(1.0, 0.78, 0.31))
		var boss = current_config.get("boss")
		if boss != null:
			var boss_id := ""
			if boss is Dictionary:
				boss_id = String(boss.get("id", ""))
			elif boss.get("id") != null:
				boss_id = String(boss.get("id"))
			_award_boss_unlock(boss_id)
		# Bosses guarantee a couple of consumable drops as a reward.
		for _i in range(CONSUMABLE_BOSS_DROPS):
			_roll_consumable_drop(true)
	else:
		_hud.set_status("◆  WAVE CLEAR  ◆  +%d" % bonus)
		ParticleFx.burst_celebration(self, VIEWPORT_SIZE * 0.5, Palette.XBOX_GREEN_GLOW)
		_roll_consumable_drop(false)
	ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5,
			"+%d" % bonus, Palette.XBOX_GREEN_GLOW, true)
	_refresh_hud()

	await get_tree().create_timer(1.2).timeout
	if not is_inside_tree():
		return
	_show_modifier_pick()


func _show_modifier_pick() -> void:
	state = State.MODIFIER_PICK
	_hud.set_status("")
	_modifier_pick_node = ModifierPick.new()
	add_child(_modifier_pick_node)
	_modifier_pick_node.modifier_chosen.connect(_on_modifier_chosen)
	_modifier_pick_node.show_pick(Modifiers.draw_three(active_modifier_ids))


func _on_modifier_chosen(id: String) -> void:
	var mod = Modifiers.create(id)
	active_modifier_ids.append(id)
	active_modifiers.append(mod)
	mod.on_pick(self)
	if _modifier_pick_node != null:
		_modifier_pick_node.queue_free()
		_modifier_pick_node = null
	_refresh_hud()
	_start_wave(current_wave + 1)


func _enter_game_over() -> void:
	state = State.GAME_OVER
	_set_balls_stopped(true)
	# Make sure we don't carry slow-mo into the score-entry scene.
	_reset_run_consumables()
	_hud.show_countdown("")
	_hud.set_status("◆  GAME OVER  ◆")
	# Cosmetic unlocks driven by final score (e.g. Champion).
	_award_score_unlock(score)
	var pf := get_node_or_null("/root/PlayFabService")
	if pf != null and pf.has_method("set_pending_run"):
		pf.set_pending_run(score, current_wave, max_combo)
	await get_tree().create_timer(1.5).timeout
	if not is_inside_tree():
		return
	get_tree().change_scene_to_file("res://leaderboard/score_entry.tscn")


# ---------------------------------------------------------------------------
# Cosmetic unlocks
# ---------------------------------------------------------------------------

func _award_boss_unlock(boss_id: String) -> void:
	if boss_id == "":
		return
	for skin_id in BallSkins.skins_unlocked_by_boss(boss_id):
		_award_skin_unlock(skin_id)


func _award_combo_unlock(current_combo: int) -> void:
	for skin_id in BallSkins.skins_unlocked_by_combo(current_combo):
		_award_skin_unlock(skin_id)


func _award_score_unlock(final_score: int) -> void:
	for skin_id in BallSkins.skins_unlocked_by_score(final_score):
		_award_skin_unlock(skin_id)


func _award_skin_unlock(skin_id: String) -> void:
	var pf := get_node_or_null("/root/PlayFabService")
	if pf == null or not pf.has_method("unlock_skin"):
		return
	if not pf.unlock_skin(skin_id):
		return  # already owned
	var skin: Dictionary = BallSkins.get_skin(skin_id)
	var label: String = "NEW SKIN: %s" % String(skin.get("name", skin_id)).to_upper()
	ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5 + Vector2(0, 60), label,
			Color(1.0, 0.95, 0.45), true)
	ParticleFx.burst_celebration(self, VIEWPORT_SIZE * 0.5, Color(1.0, 0.95, 0.45))


# ---------------------------------------------------------------------------
# Consumables — three slots refilled by wave-clear drops, fired with X/Y/RB.
# ---------------------------------------------------------------------------

func _roll_consumable_drop(forced: bool) -> void:
	if not forced and randf() > CONSUMABLE_DROP_CHANCE:
		return
	var slot_index: int = consumable_slots.find("")
	if slot_index < 0:
		# No empty slot — drops are silently lost so the player has to spend
		# what they're hoarding. Visible feedback would feel punishing.
		return
	var id: String = Consumables.random_drop_id()
	consumable_slots[slot_index] = id
	var info: Dictionary = Consumables.pretty(id)
	ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5 + Vector2(0, -90),
			"+ %s %s" % [String(info.get("glyph", "?")), String(info.get("name", id))],
			info.get("color", Palette.XBOX_GREEN_GLOW), false)
	_refresh_hud()


func _use_consumable(slot_index: int) -> void:
	if slot_index < 0 or slot_index >= consumable_slots.size():
		return
	if state != State.PLAYING:
		return
	var id: String = String(consumable_slots[slot_index])
	if id == "":
		return
	consumable_slots[slot_index] = ""
	_refresh_hud()
	var consumable = Consumables.create(id)
	_apply_consumable_effect(consumable)


func _apply_consumable_effect(consumable) -> void:
	match consumable.effect:
		"freeze":
			_freeze_until_msec = Time.get_ticks_msec() + int(consumable.duration * 1000.0)
			_set_balls_stopped(true)
			ParticleFx.burst_freeze(self, VIEWPORT_SIZE * 0.5)
			ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5 + Vector2(0, -40),
					"FREEZE", consumable.color, true)
		"aim":
			_aim_shot_pending = true
			ParticleFx.burst_aim(self, _player_paddle.position if _player_paddle != null else VIEWPORT_SIZE * 0.5)
			ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5 + Vector2(0, -40),
					"AIM SHOT READY", consumable.color, true)
		"mega":
			_mega_hit_pending = true
			ParticleFx.burst_celebration(self, _player_paddle.position if _player_paddle != null else VIEWPORT_SIZE * 0.5,
					consumable.color)
			ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5 + Vector2(0, -40),
					"MEGA HIT READY", consumable.color, true)
		"slow_mo":
			_saved_time_scale = Engine.time_scale
			Engine.time_scale = SLOW_MO_TIMESCALE
			_slow_mo_until_msec = Time.get_ticks_msec() + int(consumable.duration * 1000.0)
			ScorePopup.spawn(self, VIEWPORT_SIZE * 0.5 + Vector2(0, -40),
					"SLOW MO", consumable.color, true)


func _tick_consumable_timers() -> void:
	var now := Time.get_ticks_msec()
	if _freeze_until_msec > 0 and now >= _freeze_until_msec:
		_freeze_until_msec = 0
		# Only un-freeze if we're still mid-rally; the regular wave/countdown
		# flow will handle stop-state otherwise.
		if state == State.PLAYING:
			_set_balls_stopped(false)
	if _slow_mo_until_msec > 0 and now >= _slow_mo_until_msec:
		_slow_mo_until_msec = 0
		Engine.time_scale = _saved_time_scale


func _reset_run_consumables() -> void:
	consumable_slots = ["", "", ""]
	_aim_shot_pending = false
	_mega_hit_pending = false
	_freeze_until_msec = 0
	if _slow_mo_until_msec != 0:
		Engine.time_scale = _saved_time_scale
		_slow_mo_until_msec = 0


# ---------------------------------------------------------------------------
# Helpers used by paddle.gd (rumble) and HUD refresh
# ---------------------------------------------------------------------------

func pulse_rumble(low: float, high: float, duration: float) -> void:
	_pulse_rumble(low, high, duration)


func _pulse_rumble(low: float, high: float, duration: float) -> void:
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
	var still = gi.get_primary_device()
	if still != null:
		gi.stop_haptics(still)


func _shake() -> void:
	_shake_timer = SHAKE_DURATION


func _refresh_hud() -> void:
	if _hud == null:
		return
	_hud.set_lives(lives)
	_hud.set_score(score)
	_hud.set_wave(current_config.get("label", "WAVE"))
	_hud.set_target_progress(wave_score, target_score)
	_hud.set_combo(combo)
	_hud.set_modifiers(active_modifiers)
	_hud.set_consumables(consumable_slots)
