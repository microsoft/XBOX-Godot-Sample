extends Node2D
## Ball cosmetic picker for Pong Royale.
##
## Reads the registry in `ball_skins.gd` and the unlock state from
## `PlayFabService`, then paints a grid of cards. Locked cards show their
## unlock requirement; unlocked cards can be activated with A / Enter /
## click. B / Esc returns to the title screen.
##
## Built entirely in code so the scene file stays a one-node stub.

const Palette = preload("res://theme/palette.gd")
const BallSkins = preload("res://roguelike/ball_skins.gd")
const CRT_SHADER = preload("res://theme/crt_shader.gdshader")

const VIEWPORT_SIZE := Vector2(640, 400)
const COLUMNS := 4
const CARD_SIZE := Vector2(140, 130)
const CARD_GAP := Vector2(12, 12)
const GRID_TOP := 86.0

var _cards: Array = []
var _selected_id: String = BallSkins.DEFAULT_SKIN
var _detail_label: Label
var _selected_label: Label


func _ready() -> void:
	_selected_id = _current_selected()
	_build_background()
	_build_header()
	_build_grid()
	_build_footer()
	_build_crt_overlay()
	_focus_initial()
	set_process_unhandled_input(true)


func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_cancel"):
		_back_to_title()
		get_viewport().set_input_as_handled()


# ---------------------------------------------------------------------------
# Layout
# ---------------------------------------------------------------------------

func _build_background() -> void:
	var bg := ColorRect.new()
	bg.color = Palette.BACKGROUND
	bg.size = VIEWPORT_SIZE
	bg.z_index = -100
	add_child(bg)


func _build_header() -> void:
	var layer := CanvasLayer.new()
	layer.layer = 1
	add_child(layer)

	var root := Control.new()
	root.set_anchors_preset(Control.PRESET_FULL_RECT)
	root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	layer.add_child(root)

	var header := Label.new()
	header.text = "▣  BALL CUSTOMIZATION  ▣"
	header.add_theme_font_size_override("font_size", 20)
	header.add_theme_color_override("font_color", Palette.XBOX_GREEN_GLOW)
	header.add_theme_constant_override("outline_size", 4)
	header.add_theme_color_override("font_outline_color", Color(0.04, 0.10, 0.04, 1.0))
	header.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	header.position = Vector2(0, 22)
	header.size = Vector2(VIEWPORT_SIZE.x, 28)
	root.add_child(header)

	var sub := Label.new()
	sub.text = "PICK A BALL SKIN.  UNLOCKS PERSIST ACROSS RUNS."
	sub.add_theme_font_size_override("font_size", 10)
	sub.add_theme_color_override("font_color", Palette.TEXT_SECONDARY)
	sub.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	sub.position = Vector2(0, 50)
	sub.size = Vector2(VIEWPORT_SIZE.x, 14)
	root.add_child(sub)

	_detail_label = Label.new()
	_detail_label.text = ""
	_detail_label.add_theme_font_size_override("font_size", 10)
	_detail_label.add_theme_color_override("font_color", Palette.TEXT_PRIMARY)
	_detail_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_detail_label.position = Vector2(0, VIEWPORT_SIZE.y - 50)
	_detail_label.size = Vector2(VIEWPORT_SIZE.x, 14)
	root.add_child(_detail_label)

	_selected_label = Label.new()
	_selected_label.text = ""
	_selected_label.add_theme_font_size_override("font_size", 10)
	_selected_label.add_theme_color_override("font_color", Palette.BOSS_HUD)
	_selected_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_selected_label.position = Vector2(0, VIEWPORT_SIZE.y - 36)
	_selected_label.size = Vector2(VIEWPORT_SIZE.x, 14)
	root.add_child(_selected_label)
	_refresh_selected_label()


func _build_grid() -> void:
	var ids := BallSkins.all_ids()
	var total_cols: int = COLUMNS
	var total_rows: int = int(ceil(float(ids.size()) / float(total_cols)))
	var grid_w: float = float(total_cols) * CARD_SIZE.x + float(total_cols - 1) * CARD_GAP.x
	var grid_h: float = float(total_rows) * CARD_SIZE.y + float(total_rows - 1) * CARD_GAP.y
	var origin := Vector2(
		(VIEWPORT_SIZE.x - grid_w) * 0.5,
		GRID_TOP + maxf(0.0, (VIEWPORT_SIZE.y - GRID_TOP - 70.0 - grid_h) * 0.5)
	)

	for i in range(ids.size()):
		var col: int = i % total_cols
		var row: int = i / total_cols
		var pos := origin + Vector2(
			float(col) * (CARD_SIZE.x + CARD_GAP.x),
			float(row) * (CARD_SIZE.y + CARD_GAP.y)
		)
		var card := _build_card(ids[i], pos)
		add_child(card)
		_cards.append(card)

	# Wire focus chain (grid).
	for i in range(_cards.size()):
		var card: Button = _cards[i]
		var col: int = i % total_cols
		var row: int = i / total_cols
		var left_idx: int = i - 1 if col > 0 else i
		var right_idx: int = i + 1 if col < total_cols - 1 and i + 1 < _cards.size() else i
		var up_idx: int = i - total_cols if row > 0 else i
		var down_idx: int = i + total_cols if i + total_cols < _cards.size() else i
		card.focus_neighbor_left = _cards[left_idx].get_path()
		card.focus_neighbor_right = _cards[right_idx].get_path()
		card.focus_neighbor_top = _cards[up_idx].get_path()
		card.focus_neighbor_bottom = _cards[down_idx].get_path()


func _build_card(skin_id: String, pos: Vector2) -> Button:
	var skin: Dictionary = BallSkins.get_skin(skin_id)
	var unlocked: bool = _is_unlocked(skin_id)
	var btn := Button.new()
	btn.position = pos
	btn.custom_minimum_size = CARD_SIZE
	btn.size = CARD_SIZE
	btn.text = ""
	btn.flat = false
	btn.focus_mode = Control.FOCUS_ALL
	btn.set_meta("skin_id", skin_id)
	btn.set_meta("unlocked", unlocked)

	var border_color: Color
	if not unlocked:
		border_color = Color(0.30, 0.30, 0.30, 1.0)
	elif skin_id == _selected_id:
		border_color = Palette.BOSS_HUD
	else:
		border_color = Palette.XBOX_GREEN_GLOW
	btn.add_theme_stylebox_override("normal", _stylebox(Palette.SURFACE, border_color, 2))
	btn.add_theme_stylebox_override("hover", _stylebox(Palette.SURFACE_HIGH, border_color, 3))
	btn.add_theme_stylebox_override("focus", _stylebox(Palette.SURFACE_HIGH, border_color, 4))
	btn.add_theme_stylebox_override("pressed", _stylebox(Palette.SURFACE_HIGH, border_color, 4))

	# Body container.
	var v := VBoxContainer.new()
	v.set_anchors_preset(Control.PRESET_FULL_RECT)
	v.offset_left = 8
	v.offset_right = -8
	v.offset_top = 8
	v.offset_bottom = -8
	v.alignment = BoxContainer.ALIGNMENT_CENTER
	v.add_theme_constant_override("separation", 4)
	v.mouse_filter = Control.MOUSE_FILTER_IGNORE
	btn.add_child(v)

	# Ball preview as a TextureRect (or hidden if locked).
	var preview := TextureRect.new()
	preview.texture = BallSkins.get_texture(skin_id)
	preview.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	preview.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	preview.custom_minimum_size = Vector2(48, 48)
	preview.modulate = skin.get("tint", Color(1, 1, 1, 1)) if unlocked else Color(0.25, 0.25, 0.25, 0.6)
	preview.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(preview)

	var name_label := Label.new()
	name_label.text = String(skin.get("name", skin_id))
	name_label.add_theme_font_size_override("font_size", 12)
	name_label.add_theme_color_override("font_color",
			Palette.TEXT_PRIMARY if unlocked else Palette.TEXT_MUTED)
	name_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	name_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	name_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(name_label)

	var status := Label.new()
	if not unlocked:
		status.text = "🔒 LOCKED"
		status.add_theme_color_override("font_color", Palette.TEXT_MUTED)
	elif skin_id == _selected_id:
		status.text = "● EQUIPPED"
		status.add_theme_color_override("font_color", Palette.BOSS_HUD)
	else:
		status.text = "OWNED"
		status.add_theme_color_override("font_color", Palette.XBOX_GREEN_GLOW)
	status.add_theme_font_size_override("font_size", 9)
	status.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	status.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(status)

	btn.pressed.connect(_on_card_pressed.bind(skin_id))
	btn.focus_entered.connect(_on_card_focused.bind(skin_id))
	btn.mouse_entered.connect(btn.grab_focus)
	return btn


func _build_footer() -> void:
	var layer := CanvasLayer.new()
	layer.layer = 2
	add_child(layer)
	var root := Control.new()
	root.set_anchors_preset(Control.PRESET_FULL_RECT)
	root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	layer.add_child(root)

	var hint := Label.new()
	hint.text = "↑↓←→  BROWSE     A / ENTER  EQUIP     B / ESC  BACK"
	hint.add_theme_font_size_override("font_size", 11)
	hint.add_theme_color_override("font_color", Palette.TEXT_SECONDARY)
	hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	hint.position = Vector2(0, VIEWPORT_SIZE.y - 18)
	hint.size = Vector2(VIEWPORT_SIZE.x, 14)
	root.add_child(hint)


func _build_crt_overlay() -> void:
	var layer := CanvasLayer.new()
	layer.layer = 100
	add_child(layer)
	var rect := ColorRect.new()
	rect.size = VIEWPORT_SIZE
	rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var mat := ShaderMaterial.new()
	mat.shader = CRT_SHADER
	rect.material = mat
	layer.add_child(rect)


# ---------------------------------------------------------------------------
# Interaction
# ---------------------------------------------------------------------------

func _focus_initial() -> void:
	# Land on the currently selected card if present, else the first card.
	for card in _cards:
		if String(card.get_meta("skin_id", "")) == _selected_id:
			card.grab_focus()
			_on_card_focused(_selected_id)
			return
	if _cards.size() > 0:
		_cards[0].grab_focus()
		_on_card_focused(String(_cards[0].get_meta("skin_id", "")))


func _on_card_focused(skin_id: String) -> void:
	if _detail_label == null:
		return
	var skin: Dictionary = BallSkins.get_skin(skin_id)
	var unlocked: bool = _is_unlocked(skin_id)
	if unlocked:
		_detail_label.text = String(skin.get("tagline", ""))
		_detail_label.add_theme_color_override("font_color", Palette.TEXT_PRIMARY)
	else:
		_detail_label.text = "🔒  " + String(skin.get("unlock_hint", ""))
		_detail_label.add_theme_color_override("font_color", Palette.TEXT_DANGER)


func _on_card_pressed(skin_id: String) -> void:
	if not _is_unlocked(skin_id):
		_flash_locked(skin_id)
		return
	if _selected_id == skin_id:
		return
	_selected_id = skin_id
	var pf := get_node_or_null("/root/PlayFabService")
	if pf and pf.has_method("set_selected_skin"):
		pf.set_selected_skin(skin_id)
	_refresh_card_styles()
	_refresh_selected_label()


func _flash_locked(skin_id: String) -> void:
	# Shake the focused card briefly so the player sees the rejection.
	for card in _cards:
		if String(card.get_meta("skin_id", "")) != skin_id:
			continue
		var t := create_tween()
		var origin: Vector2 = card.position
		t.tween_property(card, "position", origin + Vector2(6, 0), 0.04)
		t.tween_property(card, "position", origin - Vector2(6, 0), 0.06)
		t.tween_property(card, "position", origin, 0.04)
		break


func _back_to_title() -> void:
	get_tree().change_scene_to_file("res://title.tscn")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

func _is_unlocked(skin_id: String) -> bool:
	var pf := get_node_or_null("/root/PlayFabService")
	if pf and pf.has_method("is_skin_unlocked"):
		return pf.is_skin_unlocked(skin_id)
	return skin_id in ["xbox", "classic"]


func _current_selected() -> String:
	var pf := get_node_or_null("/root/PlayFabService")
	if pf and pf.has_method("get_selected_skin"):
		return String(pf.get_selected_skin())
	return BallSkins.DEFAULT_SKIN


func _refresh_card_styles() -> void:
	for card in _cards:
		var skin_id: String = String(card.get_meta("skin_id", ""))
		var unlocked: bool = bool(card.get_meta("unlocked", false))
		var border: Color
		if not unlocked:
			border = Color(0.30, 0.30, 0.30, 1.0)
		elif skin_id == _selected_id:
			border = Palette.BOSS_HUD
		else:
			border = Palette.XBOX_GREEN_GLOW
		card.add_theme_stylebox_override("normal", _stylebox(Palette.SURFACE, border, 2))
		card.add_theme_stylebox_override("focus", _stylebox(Palette.SURFACE_HIGH, border, 4))
		# Update the status label child (last child of the VBox).
		var v := card.get_child(0) as VBoxContainer
		if v == null or v.get_child_count() < 3:
			continue
		var status := v.get_child(2) as Label
		if status == null:
			continue
		if not unlocked:
			status.text = "🔒 LOCKED"
			status.add_theme_color_override("font_color", Palette.TEXT_MUTED)
		elif skin_id == _selected_id:
			status.text = "● EQUIPPED"
			status.add_theme_color_override("font_color", Palette.BOSS_HUD)
		else:
			status.text = "OWNED"
			status.add_theme_color_override("font_color", Palette.XBOX_GREEN_GLOW)


func _refresh_selected_label() -> void:
	if _selected_label == null:
		return
	var skin: Dictionary = BallSkins.get_skin(_selected_id)
	_selected_label.text = "EQUIPPED  ▶  %s" % String(skin.get("name", _selected_id)).to_upper()


func _stylebox(bg: Color, border: Color, border_width: int) -> StyleBoxFlat:
	var sb := StyleBoxFlat.new()
	sb.bg_color = Color(bg.r, bg.g, bg.b, 0.95)
	sb.border_color = border
	sb.border_width_left = border_width
	sb.border_width_right = border_width
	sb.border_width_top = border_width
	sb.border_width_bottom = border_width
	sb.corner_radius_top_left = 4
	sb.corner_radius_top_right = 4
	sb.corner_radius_bottom_left = 4
	sb.corner_radius_bottom_right = 4
	return sb
