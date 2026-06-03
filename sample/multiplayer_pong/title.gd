extends Node2D
## Pong Royale — title screen, reimagined as the original Xbox dashboard.
##
## Visual recipe:
##   * deep-black background with a slowly-rotating polar wireframe grid
##   * big glowing green→yellow orb on the left, X glyph etched in the
##     middle, ghost reflection underneath, breathing pulse + ribbon arcs
##   * vertical menu of dashboard "tabs" on the right — each item is a
##     mini-orb + connector wire + tab body + right-side bracket fin,
##     focus-driven yellow highlight that slides in via a tween
##   * "(A) SELECT" footer hint, "PRESS START" feel
##   * CRT shader on top so it reads like a CRT booting up
##
## Built entirely in code so the .tscn stays a one-node stub.

const Palette = preload("res://theme/palette.gd")
const SaveData = preload("res://services/save_data.gd")
const CRT_SHADER = preload("res://theme/crt_shader.gdshader")

const VIEWPORT_SIZE := Vector2(640, 400)
const ORB_CENTER := Vector2(176, 208)
const ORB_RADIUS := 76.0
const MENU_ANCHOR := Vector2(304.0, 110.0)
const MENU_ITEM_SIZE := Vector2(316, 44)
const MENU_ITEM_SEP := 6.0

# Colour tokens for the dashboard look.
const GRID_COLOR := Color(0.06, 0.50, 0.08, 0.65)
const GRID_GLOW := Color(0.10, 0.40, 0.10, 0.18)
const ORB_GREEN_OUTER := Color(0.18, 0.55, 0.20, 1.0)
const ORB_GREEN_INNER := Color(0.65, 0.95, 0.30, 1.0)
const ORB_YELLOW_HOT := Color(1.0, 0.95, 0.45, 1.0)
const ORB_X_COLOR := Color(0.95, 1.0, 0.85, 1.0)
const ORB_X_OUTLINE := Color(0.05, 0.20, 0.05, 1.0)

var _grid_node: Node2D
var _orb_node: Node2D
var _menu_layer: CanvasLayer
var _menu_root: Control
var _high_score_label: Label
var _menu_items: Array = []
var _t: float = 0.0

# ----- Xbox services HUD (top-left card) -----
var _user_panel: Control
var _avatar_node: Control
var _avatar_texture: ImageTexture = null
var _gamertag_label: Label
var _gdk_status_node: Control
var _gdk_status_label: Label
var _gamer_picture_op = null
var _loaded_gamer_picture_xuid: String = ""
var _pending_gamer_picture_xuid: String = ""
const _GDK_STATE_OFFLINE: int = 0
const _GDK_STATE_INIT: int = 1
const _GDK_STATE_READY: int = 2
var _gdk_state: int = _GDK_STATE_OFFLINE


# ===========================================================================
# Inner class — dashboard menu item (mini-orb + wire + tab + fin + label).
# ===========================================================================

class MenuItem extends Button:
	signal activated_clean

	var label_text: String = ""
	var _hover_t: float = 0.0
	var _hover_tween: Tween
	var _pulse_t: float = 0.0

	func _init(p_text: String) -> void:
		label_text = p_text
		text = ""
		flat = true
		focus_mode = Control.FOCUS_ALL
		custom_minimum_size = Vector2(316, 44)
		var empty := StyleBoxEmpty.new()
		add_theme_stylebox_override("focus", empty)
		add_theme_stylebox_override("hover", empty)
		add_theme_stylebox_override("pressed", empty)
		add_theme_stylebox_override("normal", empty)
		add_theme_stylebox_override("disabled", empty)
		focus_entered.connect(_on_focus_in)
		focus_exited.connect(_on_focus_out)
		mouse_entered.connect(grab_focus)
		pressed.connect(func(): activated_clean.emit())

	func _process(delta: float) -> void:
		_pulse_t += delta
		if _hover_t > 0.01:
			queue_redraw()

	func _on_focus_in() -> void:
		_animate_hover(1.0)

	func _on_focus_out() -> void:
		_animate_hover(0.0)

	func _animate_hover(target: float) -> void:
		if _hover_tween and _hover_tween.is_valid():
			_hover_tween.kill()
		_hover_tween = create_tween()
		_hover_tween.tween_method(_set_hover, _hover_t, target, 0.20) \
			.set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)

	func _set_hover(v: float) -> void:
		_hover_t = v
		queue_redraw()

	func _draw() -> void:
		var t: float = _hover_t
		var s: Vector2 = self.size

		# Per-item palette (duplicated locally because inner classes can't
		# see the outer class' constants directly).
		var orb_inner: Color = Color(0.65, 0.95, 0.30, 1.0)
		var orb_yellow: Color = Color(1.0, 0.95, 0.45, 1.0)
		var tab_dim: Color = Color(0.10, 0.50, 0.10, 0.55)
		var tab_yellow: Color = Color(1.0, 0.92, 0.30, 0.85)
		var border_dim: Color = Color(0.20, 0.85, 0.30, 1.0)
		var border_yellow: Color = Color(1.0, 0.95, 0.45, 1.0)
		var wire_dim: Color = Color(0.30, 0.85, 0.30, 0.70)
		var wire_yellow: Color = Color(1.0, 0.95, 0.45, 1.0)
		var text_dim: Color = Color(0.90, 1.0, 0.90, 1.0)
		var text_hot: Color = Color(0.10, 0.10, 0.05, 1.0)

		# ----- Mini-orb -----
		var orb_center := Vector2(20, s.y * 0.5)
		var orb_radius: float = lerpf(8.0, 11.0, t) + sin(_pulse_t * 4.0) * 0.6 * t
		# Halo.
		for i in range(3):
			var ring_t := float(i) / 3.0
			var ring_color: Color = orb_inner.lerp(orb_yellow, t).darkened(0.2)
			ring_color.a = 0.18 * (1.0 - ring_t) * (0.5 + 0.5 * t)
			draw_circle(orb_center, orb_radius * (1.6 + ring_t * 0.8), ring_color)
		# Filled orb (back to front, blends to white at the surface).
		var orb_color: Color = orb_inner.lerp(orb_yellow, t)
		for i in range(5, 0, -1):
			var ratio := float(i) / 5.0
			var c: Color = orb_color.lerp(Color(1, 1, 1, orb_color.a), 1.0 - ratio)
			c.a = orb_color.a
			draw_circle(orb_center, orb_radius * ratio, c)
		# Specular dot.
		draw_circle(orb_center + Vector2(-orb_radius * 0.35, -orb_radius * 0.35),
				orb_radius * 0.18, Color(1, 1, 1, 0.55 + 0.30 * t))

		# ----- Connector wire -----
		var wire_color: Color = wire_dim.lerp(wire_yellow, t)
		var wire_start := orb_center + Vector2(orb_radius + 1.0, 0.0)
		var wire_end := Vector2(60, s.y * 0.5)
		draw_line(wire_start, wire_end, wire_color, 1.5, true)
		draw_circle(wire_end, 2.0, wire_color)

		# ----- Tab body -----
		var tab_x: float = 60.0
		var tab_w: float = 220.0
		var tab_y: float = s.y * 0.5 - 16.0
		var tab_h: float = 32.0
		var tab_rect := Rect2(tab_x, tab_y, tab_w, tab_h)
		var bg_color: Color = tab_dim.lerp(tab_yellow, t)
		bg_color.a = 0.40 + 0.35 * t
		draw_rect(tab_rect, bg_color, true)
		# Glassy top half.
		var hi := bg_color
		hi.a = 0.18 + 0.20 * t
		draw_rect(Rect2(tab_x, tab_y, tab_w, tab_h * 0.45), hi, true)
		var border_color: Color = border_dim.lerp(border_yellow, t)
		draw_rect(tab_rect, border_color, false, 1.5)

		# ----- Right fin / bracket -----
		var fin_left: float = tab_x + tab_w + 4
		var fin_top: float = tab_y + 5
		var fin_bot: float = tab_y + tab_h - 5
		var fin_pts := PackedVector2Array([
			Vector2(fin_left, fin_top),
			Vector2(fin_left + 18, fin_top),
			Vector2(fin_left + 14, fin_bot),
			Vector2(fin_left, fin_bot),
			Vector2(fin_left, fin_top),
		])
		draw_polyline(fin_pts, border_color, 1.5, true)
		# Inner tick mark.
		var midy: float = fin_top + (fin_bot - fin_top) * 0.5
		draw_line(Vector2(fin_left + 4, midy), Vector2(fin_left + 12, midy),
				border_color, 1.0, true)

		# ----- Label text -----
		var font: Font = ThemeDB.fallback_font
		if font == null:
			return
		var font_size: int = 16
		var text_color: Color = text_dim.lerp(text_hot, t * 0.7)
		var text_pos := Vector2(tab_x + 14, tab_y + 22)
		draw_string_outline(font, text_pos, label_text, HORIZONTAL_ALIGNMENT_LEFT, -1,
				font_size, 2, Color(0, 0, 0, 0.55))
		draw_string(font, text_pos, label_text, HORIZONTAL_ALIGNMENT_LEFT, -1,
				font_size, text_color)


# ===========================================================================
# Dashboard build
# ===========================================================================

func _ready() -> void:
	_build_background()
	_build_central_orb()
	_build_menu()
	_build_user_panel()
	_build_footer()
	_build_crt_overlay()
	_refresh_high_score()
	_focus_first()

	var pf := get_node_or_null("/root/PlayFabService")
	if pf and pf.has_signal("save_committed"):
		if not pf.save_committed.is_connected(_refresh_high_score):
			pf.save_committed.connect(_refresh_high_score)
	if pf and pf.has_method("request_save_refresh"):
		pf.request_save_refresh(false)

	_connect_gdk_signals()
	_refresh_user_panel()


func _process(delta: float) -> void:
	_t += delta
	if _grid_node:
		_grid_node.queue_redraw()
	if _orb_node:
		_orb_node.queue_redraw()


# ----- Background polar grid -----

func _build_background() -> void:
	var bg := ColorRect.new()
	bg.color = Color(0.01, 0.04, 0.01, 1.0)
	bg.size = VIEWPORT_SIZE
	bg.z_index = -100
	add_child(bg)

	_grid_node = Node2D.new()
	_grid_node.z_index = -50
	_grid_node.draw.connect(_draw_grid)
	add_child(_grid_node)


func _draw_grid() -> void:
	var center: Vector2 = ORB_CENTER
	var rotation_offset: float = _t * 0.05

	# Soft horizon glow band.
	_grid_node.draw_rect(Rect2(0, VIEWPORT_SIZE.y * 0.55,
			VIEWPORT_SIZE.x, VIEWPORT_SIZE.y * 0.20), GRID_GLOW)

	# Concentric ellipses (squashed for a perspective feel).
	var max_r: float = 760.0
	var step: float = 36.0
	var seg_count: int = 72
	var r: float = step
	while r < max_r:
		var alpha: float = clampf(1.0 - r / max_r, 0.0, 1.0) * 0.5 + 0.10
		var ring_color: Color = Color(GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b,
				GRID_COLOR.a * alpha)
		var pts := PackedVector2Array()
		for i in range(seg_count + 1):
			var theta: float = float(i) / float(seg_count) * TAU + rotation_offset
			var x: float = center.x + cos(theta) * r * 1.10
			var y: float = center.y + sin(theta) * r * 0.78
			pts.append(Vector2(x, y))
		_grid_node.draw_polyline(pts, ring_color, 1.0, true)
		r += step

	# Radial spokes.
	var spokes: int = 36
	for i in range(spokes):
		var theta: float = float(i) / float(spokes) * TAU + rotation_offset
		var dx: float = cos(theta)
		var dy: float = sin(theta) * 0.78
		var p1: Vector2 = center + Vector2(dx, dy) * 30.0
		var p2: Vector2 = center + Vector2(dx, dy) * max_r
		var c := Color(GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b, GRID_COLOR.a * 0.30)
		_grid_node.draw_line(p1, p2, c, 1.0, true)


# ----- Central X orb -----

func _build_central_orb() -> void:
	_orb_node = Node2D.new()
	_orb_node.z_index = 0
	_orb_node.position = ORB_CENTER
	_orb_node.draw.connect(_draw_orb)
	add_child(_orb_node)


func _draw_orb() -> void:
	var pulse_scale: float = 1.0 + sin(_t * 1.4) * 0.025
	var rot_t: float = sin(_t * 0.4) * 0.05
	# Reflection underneath (drawn first, low alpha).
	_draw_orb_at(Vector2(0, ORB_RADIUS * 1.55), ORB_RADIUS * 0.85, 0.22, rot_t)
	# Main orb.
	_draw_orb_at(Vector2.ZERO, ORB_RADIUS * pulse_scale, 1.0, rot_t)


func _draw_orb_at(offset: Vector2, radius: float, alpha_mult: float, rot_t: float) -> void:
	# Outer halo rings.
	for i in range(4):
		var ring_t: float = float(i) / 4.0
		var ring_color: Color = ORB_GREEN_OUTER
		ring_color.a = 0.15 * (1.0 - ring_t) * alpha_mult
		_orb_node.draw_circle(offset, radius * (1.0 + ring_t * 0.8), ring_color)

	# Concentric green→yellow gradient (back to front).
	var layers: int = 16
	for i in range(layers, 0, -1):
		var ratio: float = float(i) / float(layers)
		var c: Color = ORB_GREEN_OUTER.lerp(ORB_YELLOW_HOT, 1.0 - ratio)
		c.a *= alpha_mult
		_orb_node.draw_circle(offset, radius * ratio, c)

	# Bright specular highlight.
	var spec_pos: Vector2 = offset + Vector2(-radius * 0.30, -radius * 0.32)
	var spec := Color(1.0, 1.0, 0.8, 0.9 * alpha_mult)
	_orb_node.draw_circle(spec_pos, radius * 0.20, spec)
	_orb_node.draw_circle(spec_pos, radius * 0.08, Color(1.0, 1.0, 1.0, 1.0 * alpha_mult))

	# X glyph — rotated thick polylines.
	var x_size: float = radius * 0.78
	var x_thick: float = radius * 0.16
	var hs: float = x_size * 0.5
	var c1: float = cos(rot_t)
	var s1: float = sin(rot_t)
	var rotate := func(p: Vector2) -> Vector2:
		return Vector2(p.x * c1 - p.y * s1, p.x * s1 + p.y * c1)
	var p1: Vector2 = offset + rotate.call(Vector2(-hs, -hs))
	var p2: Vector2 = offset + rotate.call(Vector2(hs, hs))
	var p3: Vector2 = offset + rotate.call(Vector2(hs, -hs))
	var p4: Vector2 = offset + rotate.call(Vector2(-hs, hs))
	var x_outline := ORB_X_OUTLINE
	x_outline.a *= alpha_mult
	_orb_node.draw_line(p1, p2, x_outline, x_thick + 4.0, true)
	_orb_node.draw_line(p3, p4, x_outline, x_thick + 4.0, true)
	var x_color := ORB_X_COLOR
	x_color.a *= alpha_mult
	_orb_node.draw_line(p1, p2, x_color, x_thick, true)
	_orb_node.draw_line(p3, p4, x_color, x_thick, true)

	# Energy ribbon arcs (only on the main orb, not the reflection).
	if alpha_mult > 0.5:
		var ribbon_color: Color = ORB_GREEN_INNER
		ribbon_color.a = 0.45
		for i in range(3):
			var theta: float = TAU * float(i) / 3.0 + _t * 0.7
			var p_a: Vector2 = offset + Vector2(cos(theta), sin(theta)) * radius * 1.18
			var p_b: Vector2 = offset + Vector2(cos(theta + 0.6), sin(theta + 0.6)) * radius * 1.05
			var p_c: Vector2 = offset + Vector2(cos(theta + 1.1), sin(theta + 1.1)) * radius * 1.22
			_orb_node.draw_polyline(PackedVector2Array([p_a, p_b, p_c]),
					ribbon_color, 1.5, true)


# ----- Menu (right side) -----

func _build_menu() -> void:
	_menu_layer = CanvasLayer.new()
	_menu_layer.layer = 5
	add_child(_menu_layer)

	_menu_root = Control.new()
	_menu_root.set_anchors_preset(Control.PRESET_FULL_RECT)
	_menu_root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_menu_layer.add_child(_menu_root)

	# Top wordmark (small) + subtitle.
	var title := _make_label("◆  PONG ROYALE  ◆", 22, Palette.XBOX_GREEN_GLOW)
	title.position = Vector2(20, 22)
	title.size = Vector2(VIEWPORT_SIZE.x - 40, 28)
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title.add_theme_constant_override("outline_size", 4)
	title.add_theme_color_override("font_outline_color", Color(0.04, 0.10, 0.04, 1.0))
	_menu_root.add_child(title)

	var subtitle := _make_label(
		"FIX HACK LEARN  ·  XBOX GDK + GAMEINPUT  ·  SPRING 2026",
		9, Palette.TEXT_SECONDARY)
	subtitle.position = Vector2(20, 50)
	subtitle.size = Vector2(VIEWPORT_SIZE.x - 40, 14)
	subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_menu_root.add_child(subtitle)

	# High-score readout in the top-right corner.
	_high_score_label = _make_label("", 9, Palette.BOSS_HUD)
	_high_score_label.position = Vector2(VIEWPORT_SIZE.x - 220, 70)
	_high_score_label.size = Vector2(200, 14)
	_high_score_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	_menu_root.add_child(_high_score_label)

	# Vertical menu of dashboard tabs.
	var menu_root := Control.new()
	menu_root.position = MENU_ANCHOR
	_menu_root.add_child(menu_root)

	var labels := [
		"▶  START RUN",
		"▶  VERSUS",
		"▶  CUSTOMIZE",
		"▶  LEADERBOARD",
		"▶  QUIT",
	]
	var callbacks := [
		_on_start_run_pressed,
		_on_versus_pressed,
		_on_customize_pressed,
		_on_leaderboard_pressed,
		_on_quit_pressed,
	]
	_menu_items.clear()
	for i in range(labels.size()):
		var item := MenuItem.new(labels[i])
		item.position = Vector2(0, i * (MENU_ITEM_SIZE.y + MENU_ITEM_SEP))
		item.activated_clean.connect(callbacks[i])
		menu_root.add_child(item)
		_menu_items.append(item)

	for i in range(_menu_items.size()):
		var up_idx: int = (i - 1 + _menu_items.size()) % _menu_items.size()
		var down_idx: int = (i + 1) % _menu_items.size()
		_menu_items[i].focus_neighbor_top = _menu_items[up_idx].get_path()
		_menu_items[i].focus_neighbor_bottom = _menu_items[down_idx].get_path()
		# Left/right are no-ops on the title menu so the controller doesn't
		# accidentally drop focus.
		_menu_items[i].focus_neighbor_left = _menu_items[i].get_path()
		_menu_items[i].focus_neighbor_right = _menu_items[i].get_path()


# ----- Footer "(A) SELECT" -----

func _build_footer() -> void:
	var footer := Control.new()
	footer.position = Vector2(VIEWPORT_SIZE.x - 130, VIEWPORT_SIZE.y - 30)
	footer.size = Vector2(120, 22)
	footer.draw.connect(_draw_footer.bind(footer))
	_menu_root.add_child(footer)


func _draw_footer(footer: Control) -> void:
	var center := Vector2(12, 11)
	# Drop shadow.
	footer.draw_circle(center + Vector2(0, 1), 11.0, Color(0, 0, 0, 0.55))
	# Outer ring.
	footer.draw_circle(center, 10.0, Color(0.20, 0.85, 0.20, 1.0))
	footer.draw_circle(center, 9.0, Color(0.05, 0.22, 0.05, 1.0))
	var font: Font = ThemeDB.fallback_font
	if font:
		footer.draw_string(font, center + Vector2(-4, 4), "A",
				HORIZONTAL_ALIGNMENT_LEFT, -1, 14, Color(0.95, 1.0, 0.85, 1.0))
		footer.draw_string(font, Vector2(28, 16), "SELECT",
				HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Palette.TEXT_PRIMARY)


# ----- Xbox services HUD (top-left card) -----
##
## Mirrors the gdk_demo "user panel" but condensed to a corner readout that's
## visible on every visit to the title screen. Listens to GDK signals (and the
## one-shot per-session bootstrap silent sign-in) so the avatar / gamertag /
## status indicator stay in sync without the user having to pull-to-refresh.

func _build_user_panel() -> void:
	_user_panel = Control.new()
	_user_panel.position = Vector2(12, 70)
	_user_panel.size = Vector2(228, 40)
	_user_panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_user_panel.draw.connect(_draw_user_panel_background)
	_menu_root.add_child(_user_panel)

	# Avatar (28x28 circle). Drawn manually so we get a green ring and a
	# graceful '?' fallback when no gamer picture is available yet.
	_avatar_node = Control.new()
	_avatar_node.position = Vector2(4, 4)
	_avatar_node.size = Vector2(32, 32)
	_avatar_node.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_avatar_node.draw.connect(_draw_avatar)
	_user_panel.add_child(_avatar_node)

	# Gamertag (top text row).
	_gamertag_label = _make_label("SIGNED OUT", 11, Palette.TEXT_PRIMARY)
	_gamertag_label.position = Vector2(42, 2)
	_gamertag_label.size = Vector2(184, 16)
	_gamertag_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT
	_gamertag_label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	_gamertag_label.add_theme_constant_override("outline_size", 3)
	_gamertag_label.add_theme_color_override("font_outline_color", Color(0.04, 0.10, 0.04, 0.90))
	_gamertag_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_user_panel.add_child(_gamertag_label)

	# Status row (dot + label) sits below the gamertag.
	_gdk_status_node = Control.new()
	_gdk_status_node.position = Vector2(42, 20)
	_gdk_status_node.size = Vector2(184, 14)
	_gdk_status_node.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_gdk_status_node.draw.connect(_draw_status_dot)
	_user_panel.add_child(_gdk_status_node)

	_gdk_status_label = _make_label("GDK OFFLINE", 9, Palette.TEXT_SECONDARY)
	_gdk_status_label.position = Vector2(12, -1)
	_gdk_status_label.size = Vector2(172, 14)
	_gdk_status_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT
	_gdk_status_label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	_gdk_status_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_gdk_status_node.add_child(_gdk_status_label)


func _draw_user_panel_background() -> void:
	var rect := Rect2(Vector2.ZERO, _user_panel.size)
	var bg := Color(0.04, 0.10, 0.04, 0.55)
	_user_panel.draw_rect(rect, bg, true)
	_user_panel.draw_rect(rect, Color(0.20, 0.85, 0.30, 0.65), false, 1.0)
	# Glassy highlight strip across the top half.
	_user_panel.draw_rect(
			Rect2(Vector2(1, 1), Vector2(rect.size.x - 2, rect.size.y * 0.45)),
			Color(0.20, 0.85, 0.30, 0.10), true)


func _draw_avatar() -> void:
	var center := Vector2(16, 16)
	var ring_color: Color
	match _gdk_state:
		_GDK_STATE_READY:
			ring_color = Color(0.40, 0.95, 0.50, 1.0)
		_GDK_STATE_INIT:
			ring_color = Color(1.00, 0.85, 0.30, 1.0)
		_:
			ring_color = Color(0.85, 0.40, 0.30, 1.0)
	# Outer ring.
	_avatar_node.draw_circle(center, 15.0, ring_color)
	_avatar_node.draw_circle(center, 13.0, Color(0.05, 0.12, 0.05, 1.0))

	if _avatar_texture != null:
		var img_size: Vector2 = Vector2(_avatar_texture.get_size())
		var dest := Rect2(center - Vector2(13, 13), Vector2(26, 26))
		_avatar_node.draw_texture_rect_region(
				_avatar_texture, dest, Rect2(Vector2.ZERO, img_size))
		# Re-draw the ring on top to give a clean circular edge over the
		# square texture rect (cheap circular mask substitute).
		_avatar_node.draw_arc(center, 13.0, 0.0, TAU, 48,
				Color(0.05, 0.12, 0.05, 1.0), 2.0, true)
		_avatar_node.draw_arc(center, 14.0, 0.0, TAU, 48, ring_color, 1.5, true)
	else:
		# Placeholder glyph.
		var font: Font = ThemeDB.fallback_font
		if font != null:
			var glyph: String = "?" if _gdk_state == _GDK_STATE_OFFLINE else "X"
			var glyph_pos: Vector2 = center + Vector2(-5, 5)
			_avatar_node.draw_string(font, glyph_pos, glyph,
					HORIZONTAL_ALIGNMENT_LEFT, -1, 16,
					Color(0.85, 1.0, 0.85, 0.85))


func _draw_status_dot() -> void:
	var center := Vector2(4, 7)
	var dot_color: Color
	match _gdk_state:
		_GDK_STATE_READY:
			dot_color = Color(0.40, 0.95, 0.50, 1.0)
		_GDK_STATE_INIT:
			dot_color = Color(1.00, 0.85, 0.30, 1.0)
		_:
			dot_color = Color(0.85, 0.40, 0.30, 1.0)
	# Subtle halo so the dot reads against the dark surface.
	_gdk_status_node.draw_circle(center, 4.5, Color(dot_color.r, dot_color.g, dot_color.b, 0.30))
	_gdk_status_node.draw_circle(center, 3.0, dot_color)


func _connect_gdk_signals() -> void:
	var gdk = _get_gdk()
	if gdk == null:
		return
	if gdk.has_signal("initialized") and not gdk.initialized.is_connected(_on_gdk_state_changed):
		gdk.initialized.connect(_on_gdk_state_changed)
	if gdk.has_signal("shutdown_completed") and not gdk.shutdown_completed.is_connected(_on_gdk_state_changed):
		gdk.shutdown_completed.connect(_on_gdk_state_changed)
	if gdk.has_signal("availability_changed") and not gdk.availability_changed.is_connected(_on_gdk_availability_changed):
		gdk.availability_changed.connect(_on_gdk_availability_changed)
	var users = gdk.users
	if users == null:
		return
	if users.has_signal("user_added") and not users.user_added.is_connected(_on_gdk_user_signal):
		users.user_added.connect(_on_gdk_user_signal)
	if users.has_signal("user_changed") and not users.user_changed.is_connected(_on_gdk_user_signal):
		users.user_changed.connect(_on_gdk_user_signal)
	if users.has_signal("user_removed") and not users.user_removed.is_connected(_on_gdk_user_removed):
		users.user_removed.connect(_on_gdk_user_removed)
	if users.has_signal("primary_user_changed") and not users.primary_user_changed.is_connected(_on_gdk_primary_changed):
		users.primary_user_changed.connect(_on_gdk_primary_changed)


func _get_gdk():
	var bootstrap := get_node_or_null("/root/GDKBootstrap")
	if bootstrap != null and bootstrap.has_method("get_gdk"):
		return bootstrap.get_gdk()
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")
	return null


func _on_gdk_state_changed() -> void:
	_refresh_user_panel()


func _on_gdk_availability_changed(_available: bool) -> void:
	_refresh_user_panel()


func _on_gdk_user_signal(_user) -> void:
	_refresh_user_panel()


func _on_gdk_user_removed(_local_id: int) -> void:
	_refresh_user_panel()


func _on_gdk_primary_changed(_user) -> void:
	_refresh_user_panel()


func _refresh_user_panel() -> void:
	if _user_panel == null:
		return
	var gdk = _get_gdk()
	if gdk == null:
		_set_panel_state(_GDK_STATE_OFFLINE, "GDK OFFLINE", "—")
		_clear_avatar()
		return
	if not gdk.is_initialized():
		_set_panel_state(_GDK_STATE_INIT, "GDK INIT…", "—")
		_clear_avatar()
		return
	var user = gdk.users.get_primary_user() if gdk.users != null else null
	if user == null:
		_set_panel_state(_GDK_STATE_READY, "GDK READY", "SIGNED OUT")
		_clear_avatar()
		return
	var tag := String(user.gamertag).strip_edges()
	if tag.is_empty():
		tag = "XBOX USER"
	_set_panel_state(_GDK_STATE_READY, "GDK READY · SIGNED IN", tag)
	_load_gamer_picture(user)


func _set_panel_state(state: int, status_text: String, gamertag_text: String) -> void:
	_gdk_state = state
	if _gdk_status_label != null:
		_gdk_status_label.text = status_text
	if _gamertag_label != null:
		_gamertag_label.text = gamertag_text
	if _avatar_node != null:
		_avatar_node.queue_redraw()
	if _gdk_status_node != null:
		_gdk_status_node.queue_redraw()


func _clear_avatar() -> void:
	if _gamer_picture_op != null and not _gamer_picture_op.is_done():
		_gamer_picture_op.cancel()
	_gamer_picture_op = null
	_avatar_texture = null
	_loaded_gamer_picture_xuid = ""
	_pending_gamer_picture_xuid = ""
	if _avatar_node != null:
		_avatar_node.queue_redraw()


func _load_gamer_picture(user) -> void:
	if user == null:
		_clear_avatar()
		return
	var requested_xuid: String = String(user.xuid)
	if requested_xuid.is_empty():
		return
	if _loaded_gamer_picture_xuid == requested_xuid and _avatar_texture != null:
		return
	if _gamer_picture_op != null and not _gamer_picture_op.is_done():
		if _pending_gamer_picture_xuid == requested_xuid:
			return
		_gamer_picture_op.cancel()
	var gdk = _get_gdk()
	if gdk == null or gdk.users == null:
		return
	_pending_gamer_picture_xuid = requested_xuid
	var op = gdk.users.get_gamer_picture_async(user)
	_gamer_picture_op = op
	if op == null:
		_pending_gamer_picture_xuid = ""
		return
	if op.is_done():
		_on_gamer_picture_completed(op.get_result(), op, requested_xuid)
	else:
		op.completed.connect(_on_gamer_picture_completed.bind(op, requested_xuid))


func _on_gamer_picture_completed(result, op, requested_xuid: String) -> void:
	# Stale callback (a newer fetch superseded this one) — ignore.
	if op != _gamer_picture_op:
		return
	_pending_gamer_picture_xuid = ""
	var gdk = _get_gdk()
	var primary = gdk.users.get_primary_user() if (gdk != null and gdk.users != null) else null
	if primary == null or String(primary.xuid) != requested_xuid:
		return
	if result == null or not result.ok or result.data == null:
		return
	var image: Image = result.data
	_avatar_texture = ImageTexture.create_from_image(image)
	_loaded_gamer_picture_xuid = requested_xuid
	if _avatar_node != null:
		_avatar_node.queue_redraw()


# ----- CRT overlay -----

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


# ----- Helpers / callbacks -----

func _make_label(text: String, font_size: int, color: Color) -> Label:
	var l := Label.new()
	l.text = text
	l.add_theme_font_size_override("font_size", font_size)
	l.add_theme_color_override("font_color", color)
	return l


func _focus_first() -> void:
	if _menu_items.size() > 0:
		_menu_items[0].grab_focus()


func _refresh_high_score() -> void:
	if _high_score_label == null:
		return
	var data := _load_save()
	if data.high_score > 0:
		_high_score_label.text = "HIGH SCORE  %06d  ::  %s" % [data.high_score, data.player_name]
	else:
		_high_score_label.text = "HIGH SCORE  ------"


func _load_save() -> SaveData:
	var data := SaveData.new()
	var pf := get_node_or_null("/root/PlayFabService")
	if pf == null:
		return data
	data.load_from_dict(pf.load_game())
	return data


func _on_start_run_pressed() -> void:
	get_tree().change_scene_to_file("res://roguelike/roguelike.tscn")


func _on_versus_pressed() -> void:
	get_tree().change_scene_to_file("res://lobby.tscn")


func _on_customize_pressed() -> void:
	get_tree().change_scene_to_file("res://roguelike/skin_select.tscn")


func _on_leaderboard_pressed() -> void:
	get_tree().change_scene_to_file("res://leaderboard/leaderboard.tscn")


func _on_quit_pressed() -> void:
	get_tree().quit()
