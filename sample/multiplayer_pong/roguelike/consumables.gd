extends RefCounted
## Slay-the-Spire-style consumables for Pong Royale's roguelike run.
##
## A consumable is a one-shot ability the player can trigger mid-rally with
## the X / Y / RB controller buttons (or 1 / 2 / 3 on the keyboard). They drop
## from wave clears and live in three HUD slots; using one consumes the slot
## and the next pickup refills it.
##
## A consumable is a pure data record (this file's `Consumable` inner class).
## Its `effect` is a String tag that `roguelike.gd` interprets — that keeps
## this file dependency-free and easy to extend without touching the run
## controller.
##
## Public surface:
##   * `IDS`                    — every defined consumable id (drop pool).
##   * `create(id) -> Consumable`
##   * `random_drop_id(rng?)`   — returns a random drop, weighted by rarity.
##   * `pretty(id) -> Dictionary` — quick lookup of name/glyph/color.

class Consumable extends RefCounted:
	var id: String = ""
	var display_name: String = ""
	var glyph: String = ""
	var description: String = ""
	## How long the effect lasts (seconds). 0 = instantaneous.
	var duration: float = 0.0
	## Effect tag the run controller pattern-matches on.
	var effect: String = ""
	var color: Color = Color(0.6, 1.0, 0.6)
	## Drop weight — bigger = more common.
	var weight: float = 1.0


const IDS := [
	"freeze_ball",
	"aim_shot",
	"mega_hit",
	"slow_mo",
]


static func create(id: String) -> Consumable:
	var c := Consumable.new()
	c.id = id
	match id:
		"freeze_ball":
			c.display_name = "FREEZE BALL"
			c.glyph = "❄"
			c.description = "Freeze every ball in place for 1.4s. The clock keeps moving — use it to reposition."
			c.duration = 1.4
			c.effect = "freeze"
			c.color = Color(0.55, 0.85, 1.0)
			c.weight = 1.2
		"aim_shot":
			c.display_name = "AIM SHOT"
			c.glyph = "✚"
			c.description = "Your next paddle hit launches the ball at the steepest angle toward the AI's blind spot."
			c.duration = 0.0
			c.effect = "aim"
			c.color = Color(1.0, 0.8, 0.35)
			c.weight = 1.2
		"mega_hit":
			c.display_name = "MEGA HIT"
			c.glyph = "✦"
			c.description = "Next paddle hit is worth 3× chips and grants a guaranteed +1 mult."
			c.duration = 0.0
			c.effect = "mega"
			c.color = Color(1.0, 0.55, 0.95)
			c.weight = 1.0
		"slow_mo":
			c.display_name = "SLOW MO"
			c.glyph = "◐"
			c.description = "Engine timescale drops to 0.45 for 1.6s. Easier reads, faster nerves."
			c.duration = 1.6
			c.effect = "slow_mo"
			c.color = Color(0.6, 1.0, 0.6)
			c.weight = 0.9
		_:
			# Unknown id — return a safe placeholder so the HUD never crashes.
			c.display_name = "???"
			c.glyph = "?"
			c.description = "Unknown consumable."
			c.color = Color(0.7, 0.7, 0.7)
	return c


static func random_drop_id(rng: RandomNumberGenerator = null) -> String:
	if rng == null:
		rng = RandomNumberGenerator.new()
		rng.randomize()
	var total := 0.0
	for id in IDS:
		total += create(id).weight
	var roll: float = rng.randf() * total
	var cursor := 0.0
	for id in IDS:
		cursor += create(id).weight
		if roll <= cursor:
			return id
	return IDS[0]


static func pretty(id: String) -> Dictionary:
	var c := create(id)
	return {
		"id": c.id,
		"name": c.display_name,
		"glyph": c.glyph,
		"color": c.color,
	}
