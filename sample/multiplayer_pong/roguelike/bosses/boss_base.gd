extends RefCounted
## Boss configuration base — pure data, consumed by `roguelike.gd` and
## `wave_director.gd`. Subclasses tweak the defaults to define a specific
## boss encounter (paddle profile, ball behaviour, win condition, label).

# Stable snake_case id, set by each boss subclass. Used by the unlock /
# cosmetic systems (e.g. ball skins) to detect which boss was just defeated.
@export var id: String = "boss"
@export var display_name: String = "BOSS"
@export var win_rallies: int = 5

# Boss profile knobs ----------------------------------------------------------
@export var paddle_speed: float = 240.0
@export var paddle_reaction: float = 12.0
@export var paddle_height_multiplier: float = 1.0
## Bosses default to perfect tracking (0.0). Subclasses can soften the read by
## raising this so even bosses feel "human" instead of frame-perfect.
@export var paddle_tracking_jitter: float = 0.0

# Ball profile
@export var ball_bounce_multiplier: float = 1.1
@export var ball_passive_acceleration: float = 1.0
@export var twin_balls: bool = false

# Wave score target multiplier (boss waves are bigger blinds).
@export var target_multiplier: float = 2.5

# Hint shown during the boss intro / HUD callout.
@export var tagline: String = ""


func to_wave_config(wave_index: int) -> Dictionary:
	return {
		"is_boss": true,
		"wave_index": wave_index,
		"boss": self,
		"label": "BOSS · %s" % display_name,
		"paddle_speed": paddle_speed,
		"paddle_reaction": paddle_reaction,
		"paddle_height_multiplier": paddle_height_multiplier,
		"paddle_tracking_jitter": paddle_tracking_jitter,
		"ball_bounce_multiplier": ball_bounce_multiplier,
		"ball_passive_acceleration": ball_passive_acceleration,
		"twin_balls": twin_balls,
		"target_multiplier": target_multiplier,
		"tagline": tagline,
	}
