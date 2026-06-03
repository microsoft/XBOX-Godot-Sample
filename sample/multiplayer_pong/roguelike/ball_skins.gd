extends RefCounted
## Ball cosmetic skins for Pong Royale.
##
## Tiny registry of unlockable ball appearances + a single helper that
## applies a chosen skin to an existing Ball instance. Two skins ship free
## (the green Xbox sphere and the original yellow dot); the rest are gated
## behind in-run achievements (boss kills, big combos, high scores).
##
## Skins fall into two flavours:
##   * `texture` — load a PNG asset from `theme/skins/`.
##   * `procedural` — generate a 32×32 disc programmatically so we don't
##     have to ship dozens of art files.
##
## Generated `ImageTexture`s are cached so each skin is only built once per
## process.

const Palette = preload("res://theme/palette.gd")

const DEFAULT_SKIN := "xbox"
const SKIN_SIZE := 32

# ---------------------------------------------------------------------------
# Registry — keep order stable so the picker UI is deterministic.
# ---------------------------------------------------------------------------

const SKIN_ORDER: Array[String] = [
	"xbox",
	"classic",
	"comet",
	"twin_star",
	"velocity",
	"hot_streak",
	"champion",
]

const SKINS := {
	"xbox": {
		"id": "xbox",
		"name": "Xbox Sphere",
		"tagline": "Where it all started.",
		"kind": "texture",
		"texture_path": "res://theme/skins/ball_xbox.png",
		"tint": Color(1.0, 1.0, 1.0, 1.0),
		"swatch_a": Color(0.06, 0.49, 0.06, 1.0),
		"swatch_b": Color(0.95, 1.0, 0.95, 1.0),
		"unlock": {"type": "free"},
		"unlock_hint": "Default skin.",
	},
	"classic": {
		"id": "classic",
		"name": "Classic Pong",
		"tagline": "The original yellow pip.",
		"kind": "texture",
		"texture_path": "res://ball.png",
		"tint": Color(1.0, 0.92, 0.35, 1.0),
		"swatch_a": Color(1.0, 0.92, 0.35, 1.0),
		"swatch_b": Color(0.95, 0.65, 0.20, 1.0),
		"unlock": {"type": "free"},
		"unlock_hint": "Default skin.",
	},
	"comet": {
		"id": "comet",
		"name": "Crimson Comet",
		"tagline": "Earned by tearing down The Wall.",
		"kind": "procedural",
		"generator": "comet",
		"tint": Color(1.0, 1.0, 1.0, 1.0),
		"swatch_a": Color(0.95, 0.30, 0.20, 1.0),
		"swatch_b": Color(1.0, 0.85, 0.20, 1.0),
		"unlock": {"type": "boss", "value": "the_wall"},
		"unlock_hint": "Defeat THE WALL.",
	},
	"twin_star": {
		"id": "twin_star",
		"name": "Twin Star",
		"tagline": "Two cores, one trajectory.",
		"kind": "procedural",
		"generator": "twin",
		"tint": Color(1.0, 1.0, 1.0, 1.0),
		"swatch_a": Color(0.55, 0.40, 1.0, 1.0),
		"swatch_b": Color(0.30, 1.0, 0.95, 1.0),
		"unlock": {"type": "boss", "value": "the_twins"},
		"unlock_hint": "Defeat THE TWINS.",
	},
	"velocity": {
		"id": "velocity",
		"name": "Velocity",
		"tagline": "Speed Demon couldn't catch you.",
		"kind": "procedural",
		"generator": "velocity",
		"tint": Color(1.0, 1.0, 1.0, 1.0),
		"swatch_a": Color(1.0, 0.95, 0.20, 1.0),
		"swatch_b": Color(1.0, 0.55, 0.10, 1.0),
		"unlock": {"type": "boss", "value": "speed_demon"},
		"unlock_hint": "Defeat SPEED DEMON.",
	},
	"hot_streak": {
		"id": "hot_streak",
		"name": "Hot Streak",
		"tagline": "Born from a 25-hit combo.",
		"kind": "procedural",
		"generator": "hot",
		"tint": Color(1.0, 1.0, 1.0, 1.0),
		"swatch_a": Color(1.0, 0.40, 0.10, 1.0),
		"swatch_b": Color(1.0, 0.95, 0.30, 1.0),
		"unlock": {"type": "combo", "value": 25},
		"unlock_hint": "Reach a 25-hit combo in one rally.",
	},
	"champion": {
		"id": "champion",
		"name": "Champion",
		"tagline": "Reserved for high-scoring legends.",
		"kind": "procedural",
		"generator": "champion",
		"tint": Color(1.0, 1.0, 1.0, 1.0),
		"swatch_a": Color(1.0, 0.80, 0.20, 1.0),
		"swatch_b": Color(1.0, 1.0, 0.85, 1.0),
		"unlock": {"type": "score", "value": 5000},
		"unlock_hint": "Score at least 5,000 in a single run.",
	},
}

# Cache: skin_id -> Texture2D
static var _texture_cache: Dictionary = {}


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

static func get_skin(id: String) -> Dictionary:
	return SKINS.get(id, SKINS[DEFAULT_SKIN])


static func has_skin(id: String) -> bool:
	return SKINS.has(id)


static func all_ids() -> Array[String]:
	return SKIN_ORDER.duplicate()


static func get_texture(id: String) -> Texture2D:
	if _texture_cache.has(id):
		return _texture_cache[id]
	var skin := get_skin(id)
	var tex: Texture2D = null
	match String(skin.get("kind", "")):
		"texture":
			tex = _load_texture_resource(String(skin.get("texture_path", "")))
		"procedural":
			tex = _build_procedural(skin)
	if tex == null:
		# Last-ditch fallback: a flat green disc so the ball is never missing.
		tex = _build_procedural({
			"generator": "fallback",
			"swatch_a": Palette.XBOX_GREEN,
			"swatch_b": Palette.XBOX_GREEN_GLOW,
		})
	_texture_cache[id] = tex
	return tex


static func apply(ball: Node, skin_id: String) -> void:
	if ball == null:
		return
	var sprite := ball.get_node_or_null("Sprite2D") as Sprite2D
	if sprite == null:
		return
	var skin := get_skin(skin_id)
	sprite.texture = get_texture(String(skin.get("id", DEFAULT_SKIN)))
	sprite.modulate = skin.get("tint", Color(1, 1, 1, 1))
	# Larger procedural skins want a slight scale-down so they read like a
	# ball, not a coin. The texture-based defaults already match the
	# original sprite's footprint.
	if String(skin.get("kind", "")) == "procedural":
		sprite.scale = Vector2(0.55, 0.55)
	elif String(skin.get("id", "")) == "xbox":
		# The Xbox PNG is large (~256px) — shrink to match the existing
		# collision shape footprint.
		sprite.scale = Vector2(0.06, 0.06)
	else:
		sprite.scale = Vector2.ONE


# ---------------------------------------------------------------------------
# Unlock evaluation helpers (used by run-end hooks and the picker UI).
# ---------------------------------------------------------------------------

static func unlock_progress_label(skin_id: String) -> String:
	var skin := get_skin(skin_id)
	return String(skin.get("unlock_hint", ""))


static func skins_unlocked_by_boss(boss_id: String) -> Array[String]:
	var out: Array[String] = []
	for id in SKIN_ORDER:
		var u: Dictionary = SKINS[id].get("unlock", {})
		if u.get("type", "") == "boss" and String(u.get("value", "")) == boss_id:
			out.append(id)
	return out


static func skins_unlocked_by_combo(combo: int) -> Array[String]:
	var out: Array[String] = []
	for id in SKIN_ORDER:
		var u: Dictionary = SKINS[id].get("unlock", {})
		if u.get("type", "") == "combo" and combo >= int(u.get("value", 0)):
			out.append(id)
	return out


static func skins_unlocked_by_score(score: int) -> Array[String]:
	var out: Array[String] = []
	for id in SKIN_ORDER:
		var u: Dictionary = SKINS[id].get("unlock", {})
		if u.get("type", "") == "score" and score >= int(u.get("value", 0)):
			out.append(id)
	return out


# ---------------------------------------------------------------------------
# Internal: texture loading
# ---------------------------------------------------------------------------

static func _load_texture_resource(path: String) -> Texture2D:
	if path == "":
		return null
	if ResourceLoader.exists(path):
		var res := load(path)
		if res is Texture2D:
			return res
	# Fallback: try Image.load() against the globalised path. This works in
	# the editor / dev runs even when the .import file hasn't been
	# generated yet.
	var img := Image.new()
	var abs_path := ProjectSettings.globalize_path(path)
	if abs_path != "" and FileAccess.file_exists(abs_path):
		if img.load(abs_path) == OK:
			return ImageTexture.create_from_image(img)
	return null


# ---------------------------------------------------------------------------
# Internal: procedural skin builders
# ---------------------------------------------------------------------------

static func _build_procedural(skin: Dictionary) -> ImageTexture:
	var generator: String = String(skin.get("generator", "fallback"))
	var color_a: Color = skin.get("swatch_a", Color(0.5, 1.0, 0.5, 1.0))
	var color_b: Color = skin.get("swatch_b", Color(1.0, 1.0, 1.0, 1.0))
	var img := Image.create(SKIN_SIZE, SKIN_SIZE, false, Image.FORMAT_RGBA8)
	img.fill(Color(0, 0, 0, 0))
	var center := Vector2(SKIN_SIZE * 0.5, SKIN_SIZE * 0.5)
	var radius := SKIN_SIZE * 0.46
	for y in range(SKIN_SIZE):
		for x in range(SKIN_SIZE):
			var p := Vector2(x + 0.5, y + 0.5)
			var d := p.distance_to(center)
			if d > radius:
				continue
			var t := clampf(d / radius, 0.0, 1.0)
			var c := _shade_pixel(generator, x, y, t, p, center, radius, color_a, color_b)
			# Anti-alias the silhouette edge.
			if d > radius - 1.0:
				c.a *= clampf(radius - d, 0.0, 1.0)
			# Specular highlight common to all procedural skins.
			var spec := Vector2(SKIN_SIZE * 0.36, SKIN_SIZE * 0.36)
			var sd := p.distance_to(spec)
			if sd < radius * 0.32:
				var st := 1.0 - sd / (radius * 0.32)
				c = c.lerp(Color(1.0, 1.0, 1.0, c.a), 0.55 * st)
			img.set_pixel(x, y, c)
	return ImageTexture.create_from_image(img)


static func _shade_pixel(generator: String, x: int, y: int, t: float, p: Vector2, center: Vector2, radius: float, color_a: Color, color_b: Color) -> Color:
	var c: Color = color_a.lerp(color_b, t)
	match generator:
		"comet":
			# Sweeping diagonal streak across the disc.
			var streak := absf((p.x - p.y) - (center.x - center.y))
			if streak < radius * 0.45:
				c = c.lerp(Color(1.0, 1.0, 0.85, c.a), 0.5 * (1.0 - streak / (radius * 0.45)))
			# Trailing tail darker on the bottom-right.
			if p.x > center.x and p.y > center.y:
				c = c.darkened(0.15)
		"twin":
			# Two-tone split — left half color_a, right half color_b.
			if p.x < center.x:
				c = color_a
			else:
				c = color_b
			# Soft seam.
			if absf(p.x - center.x) < 1.5:
				c = color_a.lerp(color_b, 0.5)
		"velocity":
			# Horizontal motion stripes.
			if int(y) % 4 < 2:
				c = c.darkened(0.35)
			# Forward arrow notch on the right.
			if p.x > center.x + radius * 0.45 and absf(p.y - center.y) < radius * 0.25:
				c = Color(1.0, 1.0, 0.9, c.a)
		"hot":
			# Stochastic flicker pattern.
			var n := sin(x * 1.3 + y * 0.7) * cos(x * 0.5 - y * 1.1)
			if n > 0.25:
				c = c.lightened(0.35)
			elif n < -0.4:
				c = c.darkened(0.25)
		"champion":
			# Inner halo ring + diagonal shimmer.
			var ring_dist: float = absf(p.distance_to(center) - radius * 0.65)
			if ring_dist < 1.2:
				c = Color(1.0, 1.0, 0.85, c.a)
			elif (x + y) % 4 == 0:
				c = c.lightened(0.20)
		_:
			pass
	return c
