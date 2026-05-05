extends RefCounted
## Particle effect helpers for Pong Royale.
##
## Lightweight CPUParticles2D bursts spawned at gameplay events. Each helper
## returns the spawned node so callers can tweak/own it; the helpers also
## auto-free the node when the burst finishes via the `finished` signal so the
## scene doesn't leak particle layers across waves.
##
## Intentionally ALL static — pure helpers, no state, safe to call from the
## roguelike controller, the HUD, or the modifier pick overlay.
##
## All bursts use `one_shot = true` so they emit a single batch and stop. We
## always set `emitting = true` immediately so the batch fires the same frame.

const Palette = preload("res://theme/palette.gd")


# ---------------------------------------------------------------------------
# Public bursts — call from gameplay events.
# ---------------------------------------------------------------------------

## Hit burst — sparks that fly outward from the paddle's contact point.
## `direction_x` should be +1 for the player paddle (sparks fly right), -1
## for the AI paddle (sparks fly left).
static func burst_hit(parent: Node, at: Vector2, color: Color, direction_x: float = 1.0) -> CPUParticles2D:
	var p := _make_burst(parent, at, color, 14)
	p.lifetime = 0.45
	p.spread = 55.0
	p.direction = Vector2(direction_x, 0.0)
	p.initial_velocity_min = 140.0
	p.initial_velocity_max = 320.0
	p.scale_amount_min = 1.4
	p.scale_amount_max = 2.6
	p.angular_velocity_min = -180.0
	p.angular_velocity_max = 180.0
	p.linear_accel_min = -200.0
	p.linear_accel_max = -120.0
	return p


## Score / wave-clear celebration — radial burst with fountain feel.
static func burst_score(parent: Node, at: Vector2, color: Color, big: bool = false) -> CPUParticles2D:
	var p := _make_burst(parent, at, color, 36 if big else 22)
	p.lifetime = 0.85 if big else 0.6
	p.spread = 180.0
	p.direction = Vector2.UP
	p.initial_velocity_min = 120.0
	p.initial_velocity_max = 320.0 if big else 240.0
	p.scale_amount_min = 1.6
	p.scale_amount_max = 3.2 if big else 2.6
	p.gravity = Vector2(0.0, 380.0)
	p.color_ramp = _spark_ramp(color)
	return p


## Miss / life-lost burst — short red sparks at the goal.
static func burst_miss(parent: Node, at: Vector2) -> CPUParticles2D:
	var p := _make_burst(parent, at, Palette.TEXT_DANGER, 24)
	p.lifetime = 0.6
	p.spread = 180.0
	p.direction = Vector2.RIGHT
	p.initial_velocity_min = 160.0
	p.initial_velocity_max = 320.0
	p.scale_amount_min = 1.6
	p.scale_amount_max = 3.2
	p.angular_velocity_min = -240.0
	p.angular_velocity_max = 240.0
	p.color_ramp = _spark_ramp(Palette.TEXT_DANGER)
	return p


## Boss-clear / mega celebration — wide arc, longer-lived, gold-tinted.
static func burst_celebration(parent: Node, at: Vector2, color: Color = Palette.XBOX_GREEN_GLOW) -> CPUParticles2D:
	var p := _make_burst(parent, at, color, 64)
	p.lifetime = 1.2
	p.spread = 180.0
	p.direction = Vector2.UP
	p.initial_velocity_min = 160.0
	p.initial_velocity_max = 380.0
	p.scale_amount_min = 1.8
	p.scale_amount_max = 3.6
	p.gravity = Vector2(0.0, 420.0)
	p.color_ramp = _spark_ramp(color)
	return p


## Aim-shot indicator — soft yellow puff at the player paddle so the player
## sees the buff is armed.
static func burst_aim(parent: Node, at: Vector2) -> CPUParticles2D:
	var p := _make_burst(parent, at, Color(1.0, 0.86, 0.31), 20)
	p.lifetime = 0.5
	p.spread = 180.0
	p.initial_velocity_min = 40.0
	p.initial_velocity_max = 110.0
	p.scale_amount_min = 1.6
	p.scale_amount_max = 3.2
	p.angular_velocity_min = -120.0
	p.angular_velocity_max = 120.0
	return p


## Freeze-ball indicator — cool cyan crystals at the ball.
static func burst_freeze(parent: Node, at: Vector2) -> CPUParticles2D:
	var p := _make_burst(parent, at, Color(0.55, 0.90, 1.0), 28)
	p.lifetime = 0.7
	p.spread = 180.0
	p.initial_velocity_min = 60.0
	p.initial_velocity_max = 160.0
	p.scale_amount_min = 1.4
	p.scale_amount_max = 2.6
	p.angular_velocity_min = -240.0
	p.angular_velocity_max = 240.0
	return p


# ---------------------------------------------------------------------------
# Internal helpers
# ---------------------------------------------------------------------------

static func _make_burst(parent: Node, at: Vector2, color: Color, amount: int) -> CPUParticles2D:
	var p := CPUParticles2D.new()
	p.position = at
	p.amount = amount
	p.one_shot = true
	p.explosiveness = 1.0
	p.lifetime = 0.5
	p.color = color
	p.gravity = Vector2.ZERO
	p.damping_min = 60.0
	p.damping_max = 140.0
	p.scale_amount_min = 1.5
	p.scale_amount_max = 2.5
	# Use a simple square mesh by default — looks like CRT pixels.
	p.draw_order = CPUParticles2D.DRAW_ORDER_LIFETIME
	parent.add_child(p)
	# emit immediately, then auto-free on finish so the scene stays clean.
	p.emitting = true
	p.finished.connect(p.queue_free)
	return p


static func _spark_ramp(base: Color) -> Gradient:
	var g := Gradient.new()
	var bright := Color(min(base.r * 1.5, 1.0), min(base.g * 1.5, 1.0), min(base.b * 1.5, 1.0), 1.0)
	var fade := Color(base.r, base.g, base.b, 0.0)
	g.set_color(0, bright)
	g.set_color(1, fade)
	g.set_offset(0, 0.0)
	g.set_offset(1, 1.0)
	return g
