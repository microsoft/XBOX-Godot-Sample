extends RefCounted
## Pong Royale modifier registry — Balatro-flavored "jokers" for the run.
##
## Each modifier is a small RefCounted with optional per-event hooks. Static
## buffs (chip_bonus, mult_bonus) get summed by the controller before each
## hit. Behavioural hooks (on_pick / on_ball_spawn / on_paddle_spawn /
## on_hit / on_wave_clear / on_rally_lost) are no-ops by default and only
## overridden where a modifier needs custom behaviour.
##
## Usage:
##   const Modifiers = preload("res://roguelike/modifiers.gd")
##   var ids := Modifiers.draw_three([])  # returns 3 distinct modifier ids
##   var mod := Modifiers.create(ids[0])  # returns a Modifier instance
##   mod.on_pick(roguelike_state)         # if defined

const Palette = preload("res://theme/palette.gd")


# ---------------------------------------------------------------------------
# Base + concrete modifier classes (inner classes; instantiate via create())
# ---------------------------------------------------------------------------

class Modifier:
	var id: String = ""
	var display_name: String = ""
	var description: String = ""
	var rarity: String = "common"  # common / uncommon / rare
	var color: Color = Color.WHITE
	var glyph: String = "?"
	var chip_bonus: int = 0
	var mult_bonus: float = 0.0
	var combo_step_bonus: int = 0  # Combo King-style growth (extra steps per hit)

	# Optional hooks. Default no-ops; override in subclass where needed.
	func on_pick(_state) -> void: pass
	func on_ball_spawn(_ball) -> void: pass
	func on_paddle_spawn(_paddle) -> void: pass
	func on_hit(_ctx: Dictionary) -> void: pass
	func on_wave_clear(_state) -> float: return 0.0  # multiplier added to base 1.0
	func on_rally_lost(_state) -> void: pass


class _HeavyHitter extends Modifier: pass
class _QuickFingers extends Modifier: pass


class _ComboKing extends Modifier:
	# combo_step_bonus 1 → combo grows by 2 per hit instead of 1.
	pass


class _SteadyHand extends Modifier:
	func on_pick(state) -> void:
		state.lives += 1


class _IronWall extends Modifier:
	func on_paddle_spawn(paddle) -> void:
		paddle.scale.y *= 1.4


class _Bouncer extends Modifier:
	func on_ball_spawn(ball) -> void:
		ball.bounce_speed_multiplier *= 1.15


class _LuckyPunch extends Modifier:
	# 25% chance the hit doubles its score after base * mult.
	func on_hit(ctx: Dictionary) -> void:
		if randf() < 0.25:
			ctx["bonus_score"] = int(ctx.get("bonus_score", 0)) + int(ctx["score"])
			ctx["lucky"] = true


class _MiniBall extends Modifier:
	func on_ball_spawn(ball) -> void:
		ball.scale = Vector2(0.6, 0.6)


class _TwinThreat extends Modifier:
	func on_pick(state) -> void:
		state.extra_balls += 1


class _GlassCannon extends Modifier:
	# +1.5 mult is data-driven; the extra life loss on miss is the hook.
	func on_rally_lost(state) -> void:
		# Already ate one life from the miss; subtract one more for being glass.
		state.lives -= 1


class _TimeWarp extends Modifier:
	func on_ball_spawn(ball) -> void:
		ball._speed *= 0.75
		ball.bounce_speed_multiplier *= 0.92


class _GalaxyBrain extends Modifier:
	func on_wave_clear(_state) -> float:
		return 2.0  # base 1.0 + this 2.0 = 3x wave clear bonus


class _SecondWind extends Modifier:
	# Hit a min combo this rally to refund a life on rally win.
	# Implemented via on_hit setting a flag that the controller honours when
	# the rally ends. Refund happens at most once per rally.
	func on_hit(ctx: Dictionary) -> void:
		var c: int = int(ctx.get("combo", 0))
		if c >= 5:
			ctx["second_wind_refund"] = true


# ---------------------------------------------------------------------------
# Registry — ordered, single source of truth for static data + factory.
# ---------------------------------------------------------------------------

const _GOLD := Color(1.0, 0.78, 0.31)
const _SILVER := Color(0.78, 0.92, 0.78)
const _PURPLE := Color(0.78, 0.55, 0.95)

const _IDS_COMMON := [
	"heavy_hitter", "quick_fingers", "combo_king", "steady_hand", "iron_wall",
]
const _IDS_UNCOMMON := [
	"bouncer", "lucky_punch", "mini_ball", "twin_threat", "second_wind",
]
const _IDS_RARE := [
	"glass_cannon", "time_warp", "galaxy_brain",
]


static func all_ids() -> Array:
	var out: Array = []
	out.append_array(_IDS_COMMON)
	out.append_array(_IDS_UNCOMMON)
	out.append_array(_IDS_RARE)
	return out


static func create(id: String) -> Modifier:
	var m: Modifier
	match id:
		"heavy_hitter":
			m = _HeavyHitter.new()
			m.display_name = "HEAVY HITTER"
			m.description = "+4 chips per hit."
			m.rarity = "common"
			m.color = _SILVER
			m.glyph = "H"
			m.chip_bonus = 4
		"quick_fingers":
			m = _QuickFingers.new()
			m.display_name = "QUICK FINGERS"
			m.description = "+0.2 mult on every hit."
			m.rarity = "common"
			m.color = _SILVER
			m.glyph = "Q"
			m.mult_bonus = 0.2
		"combo_king":
			m = _ComboKing.new()
			m.display_name = "COMBO KING"
			m.description = "Combo grows twice as fast."
			m.rarity = "common"
			m.color = _SILVER
			m.glyph = "C"
			m.combo_step_bonus = 1
		"steady_hand":
			m = _SteadyHand.new()
			m.display_name = "STEADY HAND"
			m.description = "+1 life when picked."
			m.rarity = "common"
			m.color = _SILVER
			m.glyph = "+"
		"iron_wall":
			m = _IronWall.new()
			m.display_name = "IRON WALL"
			m.description = "Player paddle 40% taller."
			m.rarity = "common"
			m.color = _SILVER
			m.glyph = "I"
		"bouncer":
			m = _Bouncer.new()
			m.display_name = "BOUNCER"
			m.description = "Ball gains +15% speed per bounce."
			m.rarity = "uncommon"
			m.color = _GOLD
			m.glyph = "B"
		"lucky_punch":
			m = _LuckyPunch.new()
			m.display_name = "LUCKY PUNCH"
			m.description = "25% chance: double a hit's score."
			m.rarity = "uncommon"
			m.color = _GOLD
			m.glyph = "L"
		"mini_ball":
			m = _MiniBall.new()
			m.display_name = "MINI BALL"
			m.description = "Ball is tiny. +0.5 mult flat."
			m.rarity = "uncommon"
			m.color = _GOLD
			m.glyph = "m"
			m.mult_bonus = 0.5
		"twin_threat":
			m = _TwinThreat.new()
			m.display_name = "TWIN THREAT"
			m.description = "Spawn an extra ball each wave."
			m.rarity = "uncommon"
			m.color = _GOLD
			m.glyph = "T"
		"second_wind":
			m = _SecondWind.new()
			m.display_name = "SECOND WIND"
			m.description = "5+ combo on a rally win refunds a life."
			m.rarity = "uncommon"
			m.color = _GOLD
			m.glyph = "W"
		"glass_cannon":
			m = _GlassCannon.new()
			m.display_name = "GLASS CANNON"
			m.description = "+1.5 mult flat. Lose 2 lives on miss."
			m.rarity = "rare"
			m.color = _PURPLE
			m.glyph = "G"
			m.mult_bonus = 1.5
		"time_warp":
			m = _TimeWarp.new()
			m.display_name = "TIME WARP"
			m.description = "Ball starts slower. +0.8 mult flat."
			m.rarity = "rare"
			m.color = _PURPLE
			m.glyph = "Z"
			m.mult_bonus = 0.8
		"galaxy_brain":
			m = _GalaxyBrain.new()
			m.display_name = "GALAXY BRAIN"
			m.description = "3× wave clear bonus."
			m.rarity = "rare"
			m.color = _PURPLE
			m.glyph = "*"
		_:
			# Unknown id — return a stub so callers don't crash.
			m = Modifier.new()
			m.display_name = "UNKNOWN"
			m.description = "Unknown modifier id: %s" % id
	m.id = id
	return m


# ---------------------------------------------------------------------------
# Deck draw — pick three distinct modifiers, weighted by rarity.
# ---------------------------------------------------------------------------

const _RARITY_WEIGHTS := {
	"common": 6,
	"uncommon": 3,
	"rare": 1,
}


static func draw_three(owned_ids: Array) -> Array:
	var pool: Array = []
	for id in all_ids():
		if owned_ids.has(id):
			continue
		var rarity: String = _rarity_for(id)
		var w: int = int(_RARITY_WEIGHTS.get(rarity, 1))
		for i in range(w):
			pool.append(id)

	var picked: Array = []
	while picked.size() < 3 and not pool.is_empty():
		var idx: int = randi() % pool.size()
		var chosen: String = pool[idx]
		picked.append(chosen)
		# Remove all entries of chosen id so duplicates don't show up.
		pool = pool.filter(func(x): return x != chosen)

	# If we ran out of modifiers (player owns most), fill from owned.
	while picked.size() < 3:
		picked.append(owned_ids[randi() % owned_ids.size()])
	return picked


static func _rarity_for(id: String) -> String:
	if _IDS_COMMON.has(id):
		return "common"
	if _IDS_UNCOMMON.has(id):
		return "uncommon"
	if _IDS_RARE.has(id):
		return "rare"
	return "common"
