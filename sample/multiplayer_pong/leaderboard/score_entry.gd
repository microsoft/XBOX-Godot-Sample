extends Node2D
## Game-over confirmation -> submits the run to the PlayFab "PongLB" leaderboard
## under the signed-in player's Xbox gamertag and persists the updated save
## through Game Saves before returning to the title.
##
## Inputs come in via PlayFabService pending-run state set by the roguelike
## controller. If those are missing (for example when launching this scene
## directly), we treat the run as 0 / 1 so the screen still boots cleanly.

const Palette = preload("res://theme/palette.gd")
const SaveData = preload("res://services/save_data.gd")
const CRT_SHADER = preload("res://theme/crt_shader.gdshader")

const VIEWPORT_SIZE := Vector2(640, 400)

var _score: int = 0
var _wave: int = 1
var _max_combo: int = 0
var _gamertag_label: Label
var _score_label: Label
var _high_score_label: Label
var _confirm_button: Button


func _ready() -> void:
	_read_pending_run()
	_build_background()
	_build_ui()
	_build_crt_overlay()
	_refresh_gamertag_label()


func _read_pending_run() -> void:
	var pf := get_node_or_null("/root/PlayFabService")
	if pf == null or not pf.has_method("take_pending_run"):
		return
	var run: Dictionary = pf.take_pending_run()
	_score = int(run.get("score", 0))
	_wave = int(run.get("wave", 1))
	_max_combo = int(run.get("max_combo", 0))


func _build_background() -> void:
	var bg := ColorRect.new()
	bg.color = Palette.BACKGROUND
	bg.size = VIEWPORT_SIZE
	bg.z_index = -100
	add_child(bg)


func _build_ui() -> void:
	var ui := CanvasLayer.new()
	ui.layer = 5
	add_child(ui)

	var center := CenterContainer.new()
	center.set_anchors_preset(Control.PRESET_FULL_RECT)
	ui.add_child(center)

	var v := VBoxContainer.new()
	v.alignment = BoxContainer.ALIGNMENT_CENTER
	v.add_theme_constant_override("separation", 14)
	center.add_child(v)

	var title := _label("◆  GAME OVER  ◆", 36, Palette.TEXT_DANGER)
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	v.add_child(title)

	_score_label = _label("FINAL SCORE  %06d" % _score, 22, Palette.XBOX_GREEN_GLOW)
	_score_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	v.add_child(_score_label)

	var wave_label := _label("REACHED WAVE %02d" % _wave, 14, Palette.TEXT_SECONDARY)
	wave_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	v.add_child(wave_label)

	if _max_combo >= 2:
		var combo_label := _label("MAX COMBO  x%d" % _max_combo, 12, Palette.PADDLE_PLAYER)
		combo_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		v.add_child(combo_label)

	var spacer := Control.new()
	spacer.custom_minimum_size = Vector2(0, 12)
	v.add_child(spacer)

	var caption := _label("SUBMIT AS", 12, Palette.TEXT_PRIMARY)
	caption.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	v.add_child(caption)

	_gamertag_label = _label("PLAYER", 22, Palette.XBOX_GREEN_GLOW)
	_gamertag_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	v.add_child(_gamertag_label)

	_confirm_button = Button.new()
	_confirm_button.text = "▶  SUBMIT TO PongLB"
	_confirm_button.custom_minimum_size = Vector2(240, 36)
	_confirm_button.add_theme_color_override("font_color", Palette.XBOX_GREEN_GLOW)
	_confirm_button.add_theme_font_size_override("font_size", 14)
	_confirm_button.focus_mode = Control.FOCUS_ALL
	_confirm_button.pressed.connect(_on_confirm_pressed)
	v.add_child(_confirm_button)

	var hint := _label("ESC / B  ·  Skip and return to title", 10, Palette.TEXT_MUTED)
	hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	v.add_child(hint)

	var pf := get_node_or_null("/root/PlayFabService")
	if pf != null:
		var data: Dictionary = pf.load_game()
		var save := SaveData.new()
		save.load_from_dict(data)
		var hi := maxi(save.high_score, _score)
		_high_score_label = _label("HIGH SCORE  %06d" % hi, 12, Palette.TEXT_SECONDARY)
		_high_score_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		v.add_child(_high_score_label)

	_confirm_button.grab_focus()


func _build_crt_overlay() -> void:
	var crt := CanvasLayer.new()
	crt.layer = 100
	add_child(crt)
	var rect := ColorRect.new()
	rect.size = VIEWPORT_SIZE
	rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var mat := ShaderMaterial.new()
	mat.shader = CRT_SHADER
	rect.material = mat
	crt.add_child(rect)


# ---------------------------------------------------------------------------

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_cancel"):
		get_tree().change_scene_to_file("res://title.tscn")


func _on_confirm_pressed() -> void:
	var pf := get_node_or_null("/root/PlayFabService")
	if pf == null or not pf.has_method("submit_run_async"):
		get_tree().change_scene_to_file("res://title.tscn")
		return

	if _confirm_button != null and _confirm_button.disabled:
		return

	var fallback := _resolve_gamertag()
	_confirm_button.disabled = true
	_confirm_button.text = "SYNCING..."
	var ok = await pf.submit_run_async(fallback, _score, _wave, _max_combo)
	if not ok:
		push_warning("[Pong PlayFab] Failed to sync the completed run.")

	get_tree().change_scene_to_file("res://title.tscn")


func _refresh_gamertag_label() -> void:
	if _gamertag_label == null:
		return
	_gamertag_label.text = _resolve_gamertag()


func _resolve_gamertag() -> String:
	var pf := get_node_or_null("/root/PlayFabService")
	if pf != null and pf.has_method("get_local_player_display_name"):
		return String(pf.get_local_player_display_name(""))
	return "PLAYER"


func _label(text: String, font_size: int, color: Color) -> Label:
	var l := Label.new()
	l.text = text
	l.add_theme_font_size_override("font_size", font_size)
	l.add_theme_color_override("font_color", color)
	l.mouse_filter = Control.MOUSE_FILTER_IGNORE
	return l
