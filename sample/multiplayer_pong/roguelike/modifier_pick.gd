extends CanvasLayer
## Modifier pick overlay shown between waves in Pong Royale.
##
## Pure script (no .tscn). Adds itself on top of the run as a CanvasLayer,
## pauses gameplay via the controller's state machine, and emits
## `modifier_chosen(id)` once the player confirms a card.
##
## Navigation:
##   * Mouse: hover + click
##   * D-pad / arrows: ui_left / ui_right between cards
##   * A button / Enter: ui_accept on the focused card

signal modifier_chosen(id: String)

const Modifiers = preload("res://roguelike/modifiers.gd")
const Palette = preload("res://theme/palette.gd")

const VIEWPORT_SIZE := Vector2(640, 400)

var _ids: Array = []
var _buttons: Array = []


func show_pick(ids: Array) -> void:
	_ids = ids
	_build()


func _build() -> void:
	layer = 80

	# Anchor everything to a Control root sized to the viewport. CanvasLayer has
	# no rect of its own, so anchor presets on direct children of the layer are
	# unreliable — wrapping in a Control fixes anchor math + lets the root
	# absorb mouse input.
	var root := Control.new()
	root.set_anchors_preset(Control.PRESET_FULL_RECT)
	root.mouse_filter = Control.MOUSE_FILTER_STOP
	add_child(root)

	# Dimmer behind the cards so the arena reads as paused.
	var dim := ColorRect.new()
	dim.color = Color(0.0, 0.0, 0.0, 0.65)
	dim.set_anchors_preset(Control.PRESET_FULL_RECT)
	dim.mouse_filter = Control.MOUSE_FILTER_STOP
	root.add_child(dim)

	# Single VBox does the entire layout: header (top), card row (expands to
	# fill the middle), hint footer (bottom).  Anchors-only Controls
	# (PRESET_TOP_WIDE / PRESET_BOTTOM_WIDE) collapse to height 0 outside a
	# Container, so we lean on a real Container instead.
	var col := VBoxContainer.new()
	col.set_anchors_preset(Control.PRESET_FULL_RECT)
	col.offset_left = 16
	col.offset_right = -16
	col.offset_top = 18
	col.offset_bottom = -18
	col.add_theme_constant_override("separation", 14)
	col.mouse_filter = Control.MOUSE_FILTER_IGNORE
	root.add_child(col)

	var header := Label.new()
	header.text = "★  PICK A MODIFIER  ★"
	header.add_theme_font_size_override("font_size", 22)
	header.add_theme_color_override("font_color", Palette.XBOX_GREEN_GLOW)
	header.add_theme_constant_override("outline_size", 4)
	header.add_theme_color_override("font_outline_color", Color(0.05, 0.08, 0.05, 1.0))
	header.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	header.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	header.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	header.size_flags_vertical = Control.SIZE_SHRINK_BEGIN
	header.autowrap_mode = TextServer.AUTOWRAP_OFF
	header.clip_text = false
	header.mouse_filter = Control.MOUSE_FILTER_IGNORE
	col.add_child(header)

	# Card row, centered horizontally and expanding to take the leftover space
	# between the header and the hint.
	var row_wrap := CenterContainer.new()
	row_wrap.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	row_wrap.size_flags_vertical = Control.SIZE_EXPAND_FILL
	row_wrap.mouse_filter = Control.MOUSE_FILTER_IGNORE
	col.add_child(row_wrap)

	var row := HBoxContainer.new()
	row.add_theme_constant_override("separation", 18)
	row.alignment = BoxContainer.ALIGNMENT_CENTER
	row.mouse_filter = Control.MOUSE_FILTER_IGNORE
	row_wrap.add_child(row)

	for i in range(_ids.size()):
		var btn := _build_card(_ids[i])
		row.add_child(btn)
		_buttons.append(btn)

	# Footer hint sits at the bottom of the same VBox.
	var hint := Label.new()
	hint.text = "← →  CHOOSE     A / ENTER  CONFIRM"
	hint.add_theme_font_size_override("font_size", 11)
	hint.add_theme_color_override("font_color", Palette.TEXT_SECONDARY)
	hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	hint.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hint.size_flags_vertical = Control.SIZE_SHRINK_END
	hint.mouse_filter = Control.MOUSE_FILTER_IGNORE
	col.add_child(hint)

	# Wire focus chain so left/right cycles between cards, then grab focus on the middle.
	if _buttons.size() > 0:
		for i in range(_buttons.size()):
			var b: Button = _buttons[i]
			var left_idx: int = (i - 1 + _buttons.size()) % _buttons.size()
			var right_idx: int = (i + 1) % _buttons.size()
			b.focus_neighbor_left = _buttons[left_idx].get_path()
			b.focus_neighbor_right = _buttons[right_idx].get_path()
		# Wait two frames so layout settles before grab_focus, otherwise the
		# initial focus rect can land on (0, 0) before the HBox has been laid out.
		await get_tree().process_frame
		await get_tree().process_frame
		var middle: int = _buttons.size() / 2
		_buttons[middle].grab_focus()


func _build_card(id: String) -> Button:
	var mod = Modifiers.create(id)

	var btn := Button.new()
	btn.custom_minimum_size = Vector2(160, 200)
	btn.toggle_mode = false
	btn.focus_mode = Control.FOCUS_ALL
	btn.flat = false
	btn.text = ""

	var bg := StyleBoxFlat.new()
	bg.bg_color = Color(Palette.SURFACE.r, Palette.SURFACE.g, Palette.SURFACE.b, 0.95)
	bg.border_color = mod.color
	bg.border_width_left = 2
	bg.border_width_right = 2
	bg.border_width_top = 2
	bg.border_width_bottom = 2
	bg.corner_radius_top_left = 4
	bg.corner_radius_top_right = 4
	bg.corner_radius_bottom_left = 4
	bg.corner_radius_bottom_right = 4
	btn.add_theme_stylebox_override("normal", bg)
	btn.add_theme_stylebox_override("hover", _focused_style(mod.color))
	btn.add_theme_stylebox_override("focus", _focused_style(mod.color))
	btn.add_theme_stylebox_override("pressed", _focused_style(mod.color))

	# Layered content (Button can't host children for layout; use a Panel sibling
	# trick? Buttons CAN host children — just position absolutely.)
	var v := VBoxContainer.new()
	v.alignment = BoxContainer.ALIGNMENT_CENTER
	v.set_anchors_preset(Control.PRESET_FULL_RECT)
	v.add_theme_constant_override("separation", 6)
	v.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.offset_left = 8
	v.offset_right = -8
	v.offset_top = 8
	v.offset_bottom = -8
	btn.add_child(v)

	var rarity := Label.new()
	rarity.text = mod.rarity.to_upper()
	rarity.add_theme_font_size_override("font_size", 11)
	rarity.add_theme_color_override("font_color", mod.color)
	rarity.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	rarity.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(rarity)

	var glyph := Label.new()
	glyph.text = mod.glyph
	glyph.add_theme_font_size_override("font_size", 56)
	glyph.add_theme_color_override("font_color", mod.color)
	glyph.add_theme_constant_override("outline_size", 4)
	glyph.add_theme_color_override("font_outline_color", Color(0.0, 0.0, 0.0, 0.7))
	glyph.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	glyph.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(glyph)

	var name_label := Label.new()
	name_label.text = mod.display_name
	name_label.add_theme_font_size_override("font_size", 14)
	name_label.add_theme_color_override("font_color", Palette.TEXT_PRIMARY)
	name_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	name_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	name_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(name_label)

	var desc := Label.new()
	desc.text = mod.description
	desc.add_theme_font_size_override("font_size", 11)
	desc.add_theme_color_override("font_color", Palette.TEXT_SECONDARY)
	desc.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	desc.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	desc.custom_minimum_size = Vector2(140, 0)
	desc.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(desc)

	# Chip/mult summary line.
	var stat_text := ""
	if mod.chip_bonus != 0:
		stat_text += "+%d chips" % mod.chip_bonus
	if mod.mult_bonus != 0.0:
		if stat_text != "":
			stat_text += "   "
		stat_text += "+%.1f mult" % mod.mult_bonus
	if stat_text != "":
		var stats := Label.new()
		stats.text = stat_text
		stats.add_theme_font_size_override("font_size", 11)
		stats.add_theme_color_override("font_color", Palette.XBOX_GREEN_GLOW)
		stats.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		stats.mouse_filter = Control.MOUSE_FILTER_IGNORE
		v.add_child(stats)

	btn.pressed.connect(_on_card_pressed.bind(id))
	return btn


func _focused_style(border: Color) -> StyleBoxFlat:
	var sb := StyleBoxFlat.new()
	sb.bg_color = Color(border.r * 0.25, border.g * 0.25, border.b * 0.25, 0.95)
	sb.border_color = border
	sb.border_width_left = 4
	sb.border_width_right = 4
	sb.border_width_top = 4
	sb.border_width_bottom = 4
	sb.corner_radius_top_left = 4
	sb.corner_radius_top_right = 4
	sb.corner_radius_bottom_left = 4
	sb.corner_radius_bottom_right = 4
	return sb


func _on_card_pressed(id: String) -> void:
	modifier_chosen.emit(id)
