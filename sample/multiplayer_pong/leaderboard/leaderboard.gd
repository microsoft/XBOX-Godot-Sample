extends Node2D
## PlayFab-backed leaderboard view for the roguelike mode. Built
## programmatically; press ESC / B to return to the title.

const Palette = preload("res://theme/palette.gd")
const CRT_SHADER = preload("res://theme/crt_shader.gdshader")

const VIEWPORT_SIZE := Vector2(640, 400)
const ROW_LIMIT := 10

var _rows_container: VBoxContainer


func _ready() -> void:
    _build_background()
    _build_ui()
    _build_crt_overlay()
    var pf := get_node_or_null("/root/PlayFabService")
    if pf != null and pf.has_signal("leaderboard_updated"):
        pf.leaderboard_updated.connect(func(_mode: String) -> void: _refresh())
    if pf != null and pf.has_method("request_leaderboard_refresh"):
        pf.request_leaderboard_refresh(pf.MODE_ROGUELIKE, ROW_LIMIT, true)
    _refresh()


func _input(event: InputEvent) -> void:
    if event.is_action_pressed("ui_cancel"):
        get_tree().change_scene_to_file("res://title.tscn")


# ---------------------------------------------------------------------------

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

    var margin := MarginContainer.new()
    margin.set_anchors_preset(Control.PRESET_FULL_RECT)
    margin.add_theme_constant_override("margin_left", 32)
    margin.add_theme_constant_override("margin_right", 32)
    margin.add_theme_constant_override("margin_top", 24)
    margin.add_theme_constant_override("margin_bottom", 24)
    ui.add_child(margin)

    var v := VBoxContainer.new()
    v.add_theme_constant_override("separation", 8)
    margin.add_child(v)

    var title := _label("◆  HIGH SCORES — ROGUELIKE  ◆", 24, Palette.XBOX_GREEN_GLOW)
    title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    v.add_child(title)

    var subtitle := _label("(PongLB on PlayFab — submitted as your Xbox gamertag)",
        10, Palette.TEXT_MUTED)
    subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    v.add_child(subtitle)

    var divider := ColorRect.new()
    divider.color = Palette.XBOX_GREEN_DEEP
    divider.custom_minimum_size = Vector2(0, 2)
    v.add_child(divider)

    var header := HBoxContainer.new()
    header.add_theme_constant_override("separation", 16)
    v.add_child(header)
    header.add_child(_header_cell("RANK", 60))
    header.add_child(_header_cell("NAME", 200))
    header.add_child(_header_cell("SCORE", 140))
    header.add_child(_header_cell("WHEN", 160))

    _rows_container = VBoxContainer.new()
    _rows_container.add_theme_constant_override("separation", 4)
    v.add_child(_rows_container)

    var spacer := Control.new()
    spacer.size_flags_vertical = Control.SIZE_EXPAND_FILL
    v.add_child(spacer)

    var hint := _label("ESC / B  ·  Back to title", 12, Palette.TEXT_SECONDARY)
    hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    v.add_child(hint)


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

func _refresh() -> void:
    if _rows_container == null:
        return
    for child in _rows_container.get_children():
        child.queue_free()

    var pf := get_node_or_null("/root/PlayFabService")
    if pf == null:
        var empty := _label("PlayFabService autoload missing.",
            12, Palette.TEXT_DANGER)
        empty.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
        _rows_container.add_child(empty)
        return

    var entries: Array = pf.get_leaderboard(pf.MODE_ROGUELIKE, ROW_LIMIT)
    if entries.is_empty():
        var empty := _label("(no runs yet — go play!)",
            12, Palette.TEXT_MUTED)
        empty.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
        _rows_container.add_child(empty)
        return

    for i in range(entries.size()):
        var entry: Dictionary = entries[i]
        var row := HBoxContainer.new()
        row.add_theme_constant_override("separation", 16)
        row.add_child(_row_cell("%02d" % (i + 1), 60, _rank_color(i)))
        row.add_child(_row_cell(str(entry.get("name", "?")), 200,
            Palette.TEXT_PRIMARY))
        row.add_child(_row_cell("%06d" % int(entry.get("score", 0)), 140,
            Palette.XBOX_GREEN_GLOW))
        row.add_child(_row_cell(_format_ts(entry.get("ts", "")), 160,
            Palette.TEXT_SECONDARY))
        _rows_container.add_child(row)


func _format_ts(ts: Variant) -> String:
    if ts is int or ts is float:
        var unix_time := int(ts)
        if unix_time <= 0:
            return ""
        # PlayFab returns lastUpdated as a UTC time_t. Render as a compact
        # yyyy-mm-dd hh:mm string so the leaderboard "WHEN" column reads
        # naturally instead of showing a raw epoch number.
        return Time.get_datetime_string_from_unix_time(unix_time, true).substr(0, 16).replace("T", " ")
    var s := str(ts)
    if s.length() >= 16:
        return s.substr(0, 16).replace("T", " ")
    return s


func _rank_color(rank_idx: int) -> Color:
    match rank_idx:
        0: return Palette.XBOX_GREEN_GLOW
        1: return Palette.PADDLE_PLAYER
        2: return Palette.XBOX_GREEN
        _: return Palette.TEXT_PRIMARY


func _label(text: String, font_size: int, color: Color) -> Label:
    var l := Label.new()
    l.text = text
    l.add_theme_font_size_override("font_size", font_size)
    l.add_theme_color_override("font_color", color)
    l.mouse_filter = Control.MOUSE_FILTER_IGNORE
    return l


func _header_cell(text: String, min_width: int) -> Label:
    var l := _label(text, 11, Palette.TEXT_MUTED)
    l.custom_minimum_size = Vector2(min_width, 0)
    return l


func _row_cell(text: String, min_width: int, color: Color) -> Label:
    var l := _label(text, 16, color)
    l.custom_minimum_size = Vector2(min_width, 0)
    return l
