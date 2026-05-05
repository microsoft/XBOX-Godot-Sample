extends CanvasLayer
## Pong Royale roguelike HUD (v2 — Highscore Hustle).
##
## Built programmatically. Driven by `roguelike.gd` setters.
## Layout:
##   * top-left: LIVES ♥♥♥
##   * top-right: SCORE
##   * second row (top-left): WAVE label  •  TARGET progress
##   * center-top: big COMBO indicator (pulses on each hit)
##   * bottom-right: active-modifier strip (tinted chips with glyphs)
##   * bottom-center: status text  ("WAVE CLEAR", "GAME OVER", ...)
##   * dead-center: BOSS callout panel (boss waves only)
##   * dead-center overlay: countdown text
##
## Public API consumed by roguelike.gd:
##   set_lives(int), set_score(int), set_wave(String),
##   set_target_progress(current: int, target: int),
##   set_combo(int), pulse_combo(int),
##   set_modifiers(Array of Modifier instances),
##   set_status(String), show_boss(name, tagline), hide_boss(),
##   show_countdown(text)

const Palette = preload("res://theme/palette.gd")

const FONT_SIZE_BIG := 32
const FONT_SIZE_LABEL := 11
const FONT_SIZE_SCORE := 22
const FONT_SIZE_COMBO := 38
const FONT_SIZE_TARGET := 14

var lives_label: Label
var score_label: Label
var wave_label: Label
var target_label: Label
var target_bar_fill: ColorRect
var target_bar_back: ColorRect
var combo_label: Label
var status_label: Label
var boss_panel: PanelContainer
var boss_name_label: Label
var boss_tagline_label: Label
var countdown_label: Label
var modifier_strip: HBoxContainer
var consumable_strip: HBoxContainer

var _combo_tween: Tween


func _ready() -> void:
	layer = 50
	_build()


func _build() -> void:
	# Top HUD container.
	var top_root := MarginContainer.new()
	top_root.set_anchors_preset(Control.PRESET_TOP_WIDE)
	top_root.add_theme_constant_override("margin_left", 12)
	top_root.add_theme_constant_override("margin_right", 12)
	top_root.add_theme_constant_override("margin_top", 8)
	top_root.add_theme_constant_override("margin_bottom", 0)
	top_root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(top_root)

	var v := VBoxContainer.new()
	v.alignment = BoxContainer.ALIGNMENT_BEGIN
	v.add_theme_constant_override("separation", 4)
	v.mouse_filter = Control.MOUSE_FILTER_IGNORE
	top_root.add_child(v)

	# Row 1: lives | score
	var row1 := HBoxContainer.new()
	row1.alignment = BoxContainer.ALIGNMENT_BEGIN
	row1.add_theme_constant_override("separation", 24)
	row1.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(row1)

	lives_label = _label("LIVES ♥♥♥", FONT_SIZE_BIG, Palette.PADDLE_PLAYER)
	row1.add_child(lives_label)

	var spacer := Control.new()
	spacer.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	spacer.mouse_filter = Control.MOUSE_FILTER_IGNORE
	row1.add_child(spacer)

	var score_box := VBoxContainer.new()
	score_box.alignment = BoxContainer.ALIGNMENT_END
	score_box.mouse_filter = Control.MOUSE_FILTER_IGNORE
	row1.add_child(score_box)

	var score_caption := _label("SCORE", FONT_SIZE_LABEL, Palette.TEXT_MUTED)
	score_caption.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	score_box.add_child(score_caption)

	score_label = _label("000000", FONT_SIZE_SCORE, Palette.XBOX_GREEN_GLOW)
	score_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	score_box.add_child(score_label)

	# Row 2: wave label + target progress (label + bar).
	var row2 := HBoxContainer.new()
	row2.alignment = BoxContainer.ALIGNMENT_BEGIN
	row2.add_theme_constant_override("separation", 16)
	row2.mouse_filter = Control.MOUSE_FILTER_IGNORE
	v.add_child(row2)

	wave_label = _label("WAVE 01", FONT_SIZE_LABEL, Palette.TEXT_SECONDARY)
	row2.add_child(wave_label)

	var target_box := VBoxContainer.new()
	target_box.add_theme_constant_override("separation", 1)
	target_box.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	target_box.mouse_filter = Control.MOUSE_FILTER_IGNORE
	row2.add_child(target_box)

	target_label = _label("TARGET 0 / 0", FONT_SIZE_TARGET, Palette.XBOX_GREEN_GLOW)
	target_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	target_box.add_child(target_label)

	# Progress bar (back rect + fill rect, manually drawn for crisp blocks).
	var bar_row := MarginContainer.new()
	bar_row.add_theme_constant_override("margin_top", 2)
	bar_row.mouse_filter = Control.MOUSE_FILTER_IGNORE
	target_box.add_child(bar_row)

	var bar_holder := Control.new()
	bar_holder.custom_minimum_size = Vector2(220, 6)
	bar_holder.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	bar_holder.size_flags_vertical = Control.SIZE_FILL
	bar_holder.mouse_filter = Control.MOUSE_FILTER_IGNORE
	bar_row.add_child(bar_holder)

	target_bar_back = ColorRect.new()
	target_bar_back.color = Color(Palette.XBOX_GREEN_DEEP.r, Palette.XBOX_GREEN_DEEP.g, Palette.XBOX_GREEN_DEEP.b, 0.55)
	target_bar_back.set_anchors_preset(Control.PRESET_FULL_RECT)
	target_bar_back.mouse_filter = Control.MOUSE_FILTER_IGNORE
	bar_holder.add_child(target_bar_back)

	target_bar_fill = ColorRect.new()
	target_bar_fill.color = Palette.XBOX_GREEN_GLOW
	target_bar_fill.position = Vector2.ZERO
	target_bar_fill.size = Vector2(0.0, 6.0)
	target_bar_fill.mouse_filter = Control.MOUSE_FILTER_IGNORE
	bar_holder.add_child(target_bar_fill)

	# Combo indicator (third row in top stack — never overlaps boss callout).
	combo_label = _label("", FONT_SIZE_COMBO, Palette.XBOX_GREEN_GLOW)
	combo_label.add_theme_constant_override("outline_size", 4)
	combo_label.add_theme_color_override("font_outline_color", Color(0.05, 0.08, 0.05, 1.0))
	combo_label.visible = false
	combo_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	combo_label.pivot_offset = Vector2(60, 24)
	v.add_child(combo_label)

	# Bottom-right modifier strip.
	var mod_anchor := MarginContainer.new()
	mod_anchor.set_anchors_preset(Control.PRESET_BOTTOM_WIDE)
	mod_anchor.add_theme_constant_override("margin_bottom", 8)
	mod_anchor.add_theme_constant_override("margin_right", 12)
	mod_anchor.add_theme_constant_override("margin_left", 12)
	mod_anchor.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(mod_anchor)

	var bottom_v := VBoxContainer.new()
	bottom_v.alignment = BoxContainer.ALIGNMENT_CENTER
	bottom_v.mouse_filter = Control.MOUSE_FILTER_IGNORE
	bottom_v.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	mod_anchor.add_child(bottom_v)

	# Status text first (above mods so mods sit at the very bottom).
	status_label = _label("", FONT_SIZE_LABEL, Palette.TEXT_PRIMARY)
	status_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	status_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	bottom_v.add_child(status_label)

	modifier_strip = HBoxContainer.new()
	modifier_strip.alignment = BoxContainer.ALIGNMENT_END
	modifier_strip.add_theme_constant_override("separation", 4)
	modifier_strip.mouse_filter = Control.MOUSE_FILTER_IGNORE
	modifier_strip.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	bottom_v.add_child(modifier_strip)

	# Bottom-left consumable strip (3 slots, fills as drops accumulate).
	var cons_anchor := MarginContainer.new()
	cons_anchor.set_anchors_preset(Control.PRESET_BOTTOM_LEFT)
	cons_anchor.add_theme_constant_override("margin_bottom", 8)
	cons_anchor.add_theme_constant_override("margin_left", 12)
	cons_anchor.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(cons_anchor)

	var cons_v := VBoxContainer.new()
	cons_v.alignment = BoxContainer.ALIGNMENT_BEGIN
	cons_v.add_theme_constant_override("separation", 2)
	cons_v.mouse_filter = Control.MOUSE_FILTER_IGNORE
	cons_anchor.add_child(cons_v)

	var cons_caption := _label("CONSUMABLES  X · Y · RB", FONT_SIZE_LABEL, Palette.TEXT_MUTED)
	cons_v.add_child(cons_caption)

	consumable_strip = HBoxContainer.new()
	consumable_strip.alignment = BoxContainer.ALIGNMENT_BEGIN
	consumable_strip.add_theme_constant_override("separation", 6)
	consumable_strip.mouse_filter = Control.MOUSE_FILTER_IGNORE
	cons_v.add_child(consumable_strip)

	# Boss callout (centered, top).
	boss_panel = PanelContainer.new()
	boss_panel.set_anchors_preset(Control.PRESET_CENTER_TOP)
	boss_panel.position = Vector2(-140, 18)
	boss_panel.custom_minimum_size = Vector2(280, 0)
	boss_panel.add_theme_stylebox_override("panel", _boss_panel_style())
	boss_panel.visible = false
	boss_panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(boss_panel)

	var boss_v := VBoxContainer.new()
	boss_v.add_theme_constant_override("separation", 4)
	boss_v.alignment = BoxContainer.ALIGNMENT_CENTER
	boss_panel.add_child(boss_v)

	var boss_caption := _label("⚠  BOSS WAVE  ⚠", FONT_SIZE_LABEL, Palette.BOSS_HUD)
	boss_caption.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	boss_v.add_child(boss_caption)

	boss_name_label = _label("", 22, Palette.TEXT_DANGER)
	boss_name_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	boss_v.add_child(boss_name_label)

	boss_tagline_label = _label("", FONT_SIZE_LABEL, Palette.TEXT_SECONDARY)
	boss_tagline_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	boss_v.add_child(boss_tagline_label)

	# Big countdown text centered on screen.
	countdown_label = _label("", 96, Palette.XBOX_GREEN_GLOW)
	countdown_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	countdown_label.set_anchors_preset(Control.PRESET_CENTER)
	countdown_label.position = Vector2(-80, -60)
	countdown_label.custom_minimum_size = Vector2(160, 120)
	countdown_label.visible = false
	countdown_label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(countdown_label)


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

func set_lives(lives: int) -> void:
	var hearts := ""
	for i in range(maxi(lives, 0)):
		hearts += "♥"
	lives_label.text = "LIVES %s" % hearts
	lives_label.add_theme_color_override("font_color",
		Palette.PADDLE_PLAYER if lives > 1 else Palette.TEXT_DANGER)


func set_score(score: int) -> void:
	score_label.text = "%06d" % score


func set_wave(label: String) -> void:
	wave_label.text = label


func set_target_progress(current: int, target: int) -> void:
	target_label.text = "TARGET %d / %d" % [current, target]
	if target_bar_back == null or target_bar_fill == null:
		return
	var t: float = 0.0 if target <= 0 else clampf(float(current) / float(target), 0.0, 1.0)
	var w: float = target_bar_back.size.x * t
	target_bar_fill.size = Vector2(w, target_bar_back.size.y)
	# Color shift: green → glow → gold as we approach the target.
	var c: Color = Palette.XBOX_GREEN_GLOW
	if t >= 1.0:
		c = Color(1.0, 0.86, 0.31)
	elif t >= 0.66:
		c = Color(0.78, 1.0, 0.6)
	target_bar_fill.color = c


func set_combo(value: int) -> void:
	if value <= 1:
		combo_label.visible = false
		combo_label.text = ""
		return
	combo_label.visible = true
	combo_label.text = "x%d" % value
	# Color tier
	var c: Color = Palette.XBOX_GREEN_GLOW
	if value >= 12:
		c = Color(1.0, 0.55, 0.95)
	elif value >= 8:
		c = Color(1.0, 0.78, 0.31)
	elif value >= 4:
		c = Palette.PADDLE_PLAYER
	combo_label.add_theme_color_override("font_color", c)


func pulse_combo(_value: int) -> void:
	if _combo_tween != null and _combo_tween.is_valid():
		_combo_tween.kill()
	combo_label.scale = Vector2(1.35, 1.35)
	_combo_tween = create_tween()
	_combo_tween.tween_property(combo_label, "scale", Vector2(1.0, 1.0), 0.18) \
			.set_trans(Tween.TRANS_BACK).set_ease(Tween.EASE_OUT)


func set_modifiers(modifiers: Array) -> void:
	if modifier_strip == null:
		return
	for child in modifier_strip.get_children():
		child.queue_free()
	for mod in modifiers:
		modifier_strip.add_child(_modifier_chip(mod))


## Render the 3-slot consumable strip. `slots` is an Array of length 3 with
## either a Consumable id String or `""` for an empty slot.
func set_consumables(slots: Array) -> void:
	if consumable_strip == null:
		return
	for child in consumable_strip.get_children():
		child.queue_free()
	const Consumables = preload("res://roguelike/consumables.gd")
	const SLOT_BUTTONS := ["X", "Y", "RB"]
	for i in range(3):
		var id: String = ""
		if i < slots.size():
			id = String(slots[i])
		var slot := PanelContainer.new()
		slot.custom_minimum_size = Vector2(64, 36)
		slot.mouse_filter = Control.MOUSE_FILTER_PASS
		var sb := StyleBoxFlat.new()
		var c: Color = Palette.TEXT_MUTED
		if id != "":
			var info: Dictionary = Consumables.pretty(id)
			c = info.get("color", Palette.XBOX_GREEN_GLOW)
		sb.bg_color = Color(c.r * 0.15, c.g * 0.15, c.b * 0.15, 0.92)
		sb.border_color = c
		sb.border_width_left = 1
		sb.border_width_right = 1
		sb.border_width_top = 1
		sb.border_width_bottom = 1
		sb.corner_radius_top_left = 3
		sb.corner_radius_top_right = 3
		sb.corner_radius_bottom_left = 3
		sb.corner_radius_bottom_right = 3
		sb.content_margin_left = 4
		sb.content_margin_right = 4
		sb.content_margin_top = 2
		sb.content_margin_bottom = 2
		slot.add_theme_stylebox_override("panel", sb)
		var v := VBoxContainer.new()
		v.alignment = BoxContainer.ALIGNMENT_CENTER
		v.add_theme_constant_override("separation", 0)
		v.mouse_filter = Control.MOUSE_FILTER_IGNORE
		slot.add_child(v)
		var glyph_lbl := Label.new()
		if id == "":
			glyph_lbl.text = "·"
			glyph_lbl.add_theme_color_override("font_color", Palette.TEXT_MUTED)
		else:
			var info2: Dictionary = Consumables.pretty(id)
			glyph_lbl.text = String(info2.get("glyph", "?"))
			glyph_lbl.add_theme_color_override("font_color", c)
		glyph_lbl.add_theme_font_size_override("font_size", 16)
		glyph_lbl.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		glyph_lbl.mouse_filter = Control.MOUSE_FILTER_IGNORE
		v.add_child(glyph_lbl)
		var btn_lbl := Label.new()
		btn_lbl.text = SLOT_BUTTONS[i]
		btn_lbl.add_theme_font_size_override("font_size", 9)
		btn_lbl.add_theme_color_override("font_color", Palette.TEXT_MUTED)
		btn_lbl.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		btn_lbl.mouse_filter = Control.MOUSE_FILTER_IGNORE
		v.add_child(btn_lbl)
		consumable_strip.add_child(slot)


func _modifier_chip(mod) -> Control:
	var chip := PanelContainer.new()
	chip.custom_minimum_size = Vector2(28, 28)
	chip.tooltip_text = "%s\n%s" % [mod.display_name, mod.description]
	chip.mouse_filter = Control.MOUSE_FILTER_PASS

	var sb := StyleBoxFlat.new()
	sb.bg_color = Color(mod.color.r * 0.25, mod.color.g * 0.25, mod.color.b * 0.25, 0.95)
	sb.border_color = mod.color
	sb.border_width_left = 1
	sb.border_width_right = 1
	sb.border_width_top = 1
	sb.border_width_bottom = 1
	sb.corner_radius_top_left = 3
	sb.corner_radius_top_right = 3
	sb.corner_radius_bottom_left = 3
	sb.corner_radius_bottom_right = 3
	sb.content_margin_left = 4
	sb.content_margin_right = 4
	sb.content_margin_top = 2
	sb.content_margin_bottom = 2
	chip.add_theme_stylebox_override("panel", sb)

	var lbl := Label.new()
	lbl.text = mod.glyph
	lbl.add_theme_font_size_override("font_size", 16)
	lbl.add_theme_color_override("font_color", mod.color)
	lbl.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	lbl.mouse_filter = Control.MOUSE_FILTER_IGNORE
	chip.add_child(lbl)

	return chip


func set_status(text: String) -> void:
	status_label.text = text


func show_boss(name_text: String, tagline: String) -> void:
	boss_name_label.text = name_text
	boss_tagline_label.text = tagline
	boss_panel.visible = true


func hide_boss() -> void:
	boss_panel.visible = false


func show_countdown(value: String) -> void:
	countdown_label.text = value
	countdown_label.visible = value != ""


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

func _label(text: String, font_size: int, color: Color) -> Label:
	var l := Label.new()
	l.text = text
	l.add_theme_font_size_override("font_size", font_size)
	l.add_theme_color_override("font_color", color)
	l.mouse_filter = Control.MOUSE_FILTER_IGNORE
	return l


func _boss_panel_style() -> StyleBoxFlat:
	var sb := StyleBoxFlat.new()
	sb.bg_color = Color(Palette.SURFACE.r, Palette.SURFACE.g, Palette.SURFACE.b, 0.92)
	sb.border_color = Palette.TEXT_DANGER
	sb.border_width_left = 2
	sb.border_width_right = 2
	sb.border_width_top = 2
	sb.border_width_bottom = 2
	sb.corner_radius_top_left = 4
	sb.corner_radius_top_right = 4
	sb.corner_radius_bottom_left = 4
	sb.corner_radius_bottom_right = 4
	sb.content_margin_left = 16
	sb.content_margin_right = 16
	sb.content_margin_top = 10
	sb.content_margin_bottom = 10
	return sb
