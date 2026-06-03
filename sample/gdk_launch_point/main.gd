extends Control
## GDK Launch Point scenario shell for the Godot GDK sample surface.

const GDK_EXTENSION_PATH = "res://addons/godot_gdk/godot_gdk.gdextension"
const GAMEINPUT_EXTENSION_PATH = "res://addons/godot_gameinput/godot_gameinput.gdextension"
const DEFAULT_ACHIEVEMENT_ID = "1"
const ACHIEVEMENT_STEP = 25
const DEMO_MPA_CONNECTION_STRING = "godot-gdk-launch-point://sample-session"
const DEMO_MPA_GROUP_ID = "gdk-launch-point-mpa-group"
const DEMO_MPA_MAX_PLAYERS = 4
const DEMO_MPA_CURRENT_PLAYERS = 1
const MAX_LOG_LINES = 80
const XBOX_BACKGROUND := Color(0.02, 0.03, 0.02)
const XBOX_PANEL := Color(0.05, 0.08, 0.05, 0.94)
const XBOX_PANEL_STRONG := Color(0.07, 0.12, 0.06, 0.96)
const XBOX_PANEL_CARD := Color(0.04, 0.07, 0.04, 0.94)
const XBOX_PANEL_GROUP := Color(0.08, 0.14, 0.06, 0.96)
const XBOX_BORDER := Color(0.38, 0.68, 0.17, 0.82)
const XBOX_BORDER_STRONG := Color(0.66, 0.96, 0.24, 0.96)
const XBOX_GLOW := Color(0.23, 0.64, 0.12, 0.30)
const XBOX_GLOW_STRONG := Color(0.36, 0.95, 0.14, 0.42)
const XBOX_TEXT := Color(0.91, 0.97, 0.90)
const XBOX_TEXT_SOFT := Color(0.69, 0.82, 0.68)
const XBOX_ACCENT := Color(0.77, 0.98, 0.29)
const XBOX_ACCENT_BRIGHT := Color(0.88, 1.0, 0.48)
const XBOX_STATUS_READY := Color(0.79, 1.0, 0.42)
const XBOX_STATUS_IDLE := Color(0.79, 0.90, 0.78)
const XBOX_STATUS_WARN := Color(0.93, 0.96, 0.63)
const XBOX_BUTTON := Color(0.14, 0.31, 0.09, 0.95)
const XBOX_BUTTON_HOVER := Color(0.19, 0.42, 0.11, 0.98)
const XBOX_BUTTON_PRESSED := Color(0.10, 0.22, 0.07, 0.98)
const XBOX_BUTTON_SUBTLE := Color(0.08, 0.13, 0.07, 0.95)
const XBOX_BUTTON_SUBTLE_HOVER := Color(0.12, 0.20, 0.08, 0.98)
const XBOX_BUTTON_DISABLED := Color(0.07, 0.09, 0.07, 0.85)

const RUMBLE_DEMO_LOW = 0.45
const RUMBLE_DEMO_HIGH = 0.25
const RUMBLE_DEMO_DURATION = 0.4

@onready var title_label: Label = $OuterMargin/RootVBox/HeaderPanel/HeaderVBox/TitleLabel
@onready var tagline_label: Label = $OuterMargin/RootVBox/HeaderPanel/HeaderVBox/TaglineLabel
@onready var back_button: Button = $OuterMargin/RootVBox/HeaderPanel/HeaderVBox/HeaderInfo/BackButton
@onready var breadcrumb_label: Label = $OuterMargin/RootVBox/HeaderPanel/HeaderVBox/HeaderInfo/BreadcrumbLabel
@onready var runtime_status_label: Label = $OuterMargin/RootVBox/HeaderPanel/HeaderVBox/HeaderInfo/RuntimeStatusLabel
@onready var group_title_label: Label = $OuterMargin/RootVBox/BodySplit/MenuPanel/MenuVBox/GroupTitleLabel
@onready var group_description: RichTextLabel = $OuterMargin/RootVBox/BodySplit/MenuPanel/MenuVBox/GroupDescription
@onready var scenario_grid: GridContainer = $OuterMargin/RootVBox/BodySplit/MenuPanel/MenuVBox/ScenarioScroll/ScenarioGrid
@onready var selected_details: RichTextLabel = $OuterMargin/RootVBox/BodySplit/StatusPanel/StatusVBox/SelectedDetails
@onready var state_details: RichTextLabel = $OuterMargin/RootVBox/BodySplit/StatusPanel/StatusVBox/StateDetails
@onready var event_log: RichTextLabel = $OuterMargin/RootVBox/LogPanel/LogVBox/EventLog
@onready var clear_log_button: Button = $OuterMargin/RootVBox/LogPanel/LogVBox/LogHeader/ClearLogButton
@onready var header_panel: PanelContainer = $OuterMargin/RootVBox/HeaderPanel
@onready var menu_panel: PanelContainer = $OuterMargin/RootVBox/BodySplit/MenuPanel
@onready var status_panel: PanelContainer = $OuterMargin/RootVBox/BodySplit/StatusPanel
@onready var log_panel: PanelContainer = $OuterMargin/RootVBox/LogPanel
@onready var selected_heading_label: Label = $OuterMargin/RootVBox/BodySplit/StatusPanel/StatusVBox/SelectedHeadingLabel
@onready var state_heading_label: Label = $OuterMargin/RootVBox/BodySplit/StatusPanel/StatusVBox/StateHeadingLabel
@onready var log_title_label: Label = $OuterMargin/RootVBox/LogPanel/LogVBox/LogHeader/LogTitleLabel

var _gdk_extension = null
var _gdk_load_attempted = false
var _gameinput_extension = null
var _gameinput_load_attempted = false
var _gameinput_signals_bound = false
var _gameinput_last_event = "No GameInput events yet."
var _scenario_root: Dictionary = {}
var _scenario_stack: Array[Dictionary] = []
var _current_group: Dictionary = {}
var _selected_entry: Dictionary = {}
var _event_log_lines: Array[String] = []
var _demo_achievement_id = DEFAULT_ACHIEVEMENT_ID
var _last_selected_status = ""
var _last_mpa_event_text = "No invite events yet."

func _gdk():
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	if not _gdk_load_attempted and _gdk_extension == null and FileAccess.file_exists(GDK_EXTENSION_PATH):
		_gdk_load_attempted = true
		_gdk_extension = load(GDK_EXTENSION_PATH)

	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	return null

func _gameinput():
	if Engine.has_singleton("GameInput"):
		return Engine.get_singleton("GameInput")

	if not _gameinput_load_attempted and _gameinput_extension == null and FileAccess.file_exists(GAMEINPUT_EXTENSION_PATH):
		_gameinput_load_attempted = true
		_gameinput_extension = load(GAMEINPUT_EXTENSION_PATH)

	if Engine.has_singleton("GameInput"):
		return Engine.get_singleton("GameInput")

	return null

func _ready() -> void:
	title_label.text = "GDK Launch Point"
	back_button.pressed.connect(_on_back_pressed)
	clear_log_button.pressed.connect(_on_clear_log_pressed)
	_apply_launch_point_theme()

	_load_sample_config()
	_bind_gdk_signals()
	_bind_gameinput_signals()
	_scenario_root = _build_scenario_catalog()
	_current_group = _scenario_root

	_render_current_group()
	_refresh_state_panel()
	_log_event("GDK Launch Point ready. Open a group to explore the current GDK addon surface.")

func _load_sample_config() -> void:
	var cfg = ConfigFile.new()
	var load_error = cfg.load("res://sample_config.cfg")
	if load_error != OK:
		load_error = cfg.load("res://sample_config.cfg.template")

	if load_error == OK:
		_demo_achievement_id = str(cfg.get_value("achievements", "demo_achievement_id", DEFAULT_ACHIEVEMENT_ID))

func _bind_gdk_signals() -> void:
	var gdk = _gdk()
	if gdk == null:
		_log_event("GDK addon files are not loaded yet. Build the repo before launching this sample.")
		return

	gdk.initialized.connect(_on_runtime_initialized)
	gdk.shutdown_completed.connect(_on_runtime_shutdown)
	gdk.runtime_error.connect(_on_runtime_error)
	gdk.users.user_added.connect(_on_user_added)
	gdk.users.user_changed.connect(_on_user_changed)
	gdk.users.user_removed.connect(_on_user_removed)
	gdk.users.primary_user_changed.connect(_on_primary_user_changed)
	gdk.achievements.achievements_updated.connect(_on_achievements_updated)
	gdk.achievements.achievement_unlocked.connect(_on_achievement_unlocked)
	gdk.multiplayer_activity.activities_updated.connect(_on_mpa_activities_updated)
	gdk.multiplayer_activity.pending_invite_received.connect(_on_pending_invite_received)
	gdk.multiplayer_activity.invite_accepted.connect(_on_invite_accepted)

func _build_scenario_catalog() -> Dictionary:
	return _group(
		"root",
		"GDK Launch Point",
		"Scenario-driven entry point for runtime, users, achievements, and multiplayer activity. Open a group to run sample actions and watch the event log update.",
		[
			_group(
				"runtime",
				"Runtime",
				"Initialize, inspect, and shut down the root GDK runtime singleton.",
				[
					_scenario("runtime_initialize", "Initialize Runtime", "Call GDK.initialize() and report the resulting GDKResult.", Callable(self, "_scenario_initialize_runtime")),
					_scenario("runtime_last_error", "Show Last Error", "Read GDK.get_last_error() and add the current error state to the log.", Callable(self, "_scenario_show_last_error")),
					_scenario("runtime_shutdown", "Shutdown Runtime", "Call GDK.shutdown() so the shell returns to an uninitialized state.", Callable(self, "_scenario_shutdown_runtime"))
				]
			),
			_group(
				"users",
				"Users",
				"Explore the current GDK users surface with explicit sign-in and profile scenarios.",
				[
					_scenario("users_silent_sign_in", "Silent Sign-In", "Attempt add_default_user_async() for a non-guest silent sign-in and track the returned completion signal.", Callable(self, "_scenario_silent_sign_in")),
					_scenario("users_sign_in_ui", "User Picker", "Launch add_user_with_ui_async() for an explicit sign-in or guest-capable picker flow. This does not replace the session primary user once one is established.", Callable(self, "_scenario_sign_in_with_ui")),
					_scenario("users_summary", "Log User Summary", "Log the current primary user plus the signed-in user count.", Callable(self, "_scenario_log_user_summary")),
					_scenario("users_gamer_picture", "Load Gamer Picture", "Request the primary user's gamer picture and log the returned image details.", Callable(self, "_scenario_load_gamer_picture"))
				]
			),
			_group(
				"achievements",
				"Achievements",
				"Drive the achievement manager flow against the configured demo achievement ID.",
				[
					_scenario("achievements_query", "Query Player Achievements", "Fetch achievement state for the primary user and refresh the cached achievement summary.", Callable(self, "_scenario_query_achievements")),
					_scenario("achievements_increment", "Increment Demo Achievement", "Advance the configured demo achievement in 25%% steps using update_achievement_async().", Callable(self, "_scenario_increment_achievement")),
					_scenario("achievements_snapshot", "Log Achievement Snapshot", "Write the cached demo achievement state to the event log.", Callable(self, "_scenario_log_achievement_snapshot"))
				]
			),
			_group(
				"multiplayer_activity",
				"Multiplayer Activity",
				"Exercise the merged GDK.multiplayer_activity service with local activity, cache refresh, and invite UI scenarios.",
				[
					_scenario("mpa_set_activity", "Set Local Activity", "Publish a demo multiplayer activity for the primary user.", Callable(self, "_scenario_set_mpa_activity")),
					_scenario("mpa_refresh_activity", "Refresh Local Activity", "Fetch the primary user's activity into the local cache with get_activities_async().", Callable(self, "_scenario_refresh_mpa_activity")),
					_scenario("mpa_log_snapshot", "Log Activity Snapshot", "Write the cached local multiplayer activity summary to the event log.", Callable(self, "_scenario_log_mpa_snapshot")),
					_scenario("mpa_invite_ui", "Show Invite UI", "Open the system invite UI for the current multiplayer activity.", Callable(self, "_scenario_show_mpa_invite_ui")),
					_scenario("mpa_clear_activity", "Clear Local Activity", "Delete the primary user's current multiplayer activity.", Callable(self, "_scenario_clear_mpa_activity"))
				]
			),
			_group(
				"gameinput",
				"GameInput",
				"Drive the godot_gameinput addon: initialize the runtime, list connected devices, query battery + device info, and pulse rumble on the primary controller.",
				[
					_scenario("gameinput_initialize", "Initialize GameInput", "Call GameInput.initialize() and report whether the GameInput runtime is ready.", Callable(self, "_scenario_gameinput_initialize")),
					_scenario("gameinput_shutdown", "Shutdown GameInput", "Call GameInput.shutdown() and confirm the runtime returns to an uninitialized state.", Callable(self, "_scenario_gameinput_shutdown")),
					_scenario("gameinput_list_devices", "List Devices", "Poll GameInput, enumerate gamepads, and log each device's display name + ID.", Callable(self, "_scenario_gameinput_list_devices")),
					_scenario("gameinput_inspect_primary", "Inspect Primary Device", "Show the primary device's display name, vendor/product IDs, battery level, and rumble support.", Callable(self, "_scenario_gameinput_inspect_primary")),
					_scenario("gameinput_rumble_pulse", "Rumble Pulse", "Run a short low+high frequency rumble on the primary device, then stop haptics.", Callable(self, "_scenario_gameinput_rumble_pulse")),
					_scenario("gameinput_stop_rumble", "Stop Rumble", "Call GameInput.stop_haptics() on the primary device immediately.", Callable(self, "_scenario_gameinput_stop_rumble"))
				]
			)
		]
	)

func _group(id: String, title: String, description: String, children: Array) -> Dictionary:
	return {
		"id": id,
		"title": title,
		"description": description,
		"children": children
	}

func _scenario(id: String, title: String, description: String, action: Callable) -> Dictionary:
	return {
		"id": id,
		"title": title,
		"description": description,
		"action": action
	}

func _entry_has_children(entry: Dictionary) -> bool:
	if not entry.has("children"):
		return false

	var children = entry["children"]
	return children is Array and children.size() > 0

func _render_current_group() -> void:
	group_title_label.text = str(_current_group.get("title", "Scenario Group"))
	group_description.text = str(_current_group.get("description", ""))
	back_button.visible = not _scenario_stack.is_empty()
	breadcrumb_label.text = _build_breadcrumb()

	for child in scenario_grid.get_children():
		child.queue_free()

	var children = _current_group.get("children", [])
	for entry in children:
		scenario_grid.add_child(_create_scenario_card(entry))

	_update_selected_details(_selected_entry, _last_selected_status)

func _build_breadcrumb() -> String:
	var segments: Array[String] = []
	segments.append("Home")

	for group in _scenario_stack:
		if group.get("id", "") == "root":
			continue
		segments.append(str(group.get("title", "Group")))

	if _current_group.get("id", "") != "root":
		segments.append(str(_current_group.get("title", "Group")))

	return " > ".join(segments)

func _apply_launch_point_theme() -> void:
	RenderingServer.set_default_clear_color(XBOX_BACKGROUND)
	_install_background_layers()

	header_panel.add_theme_stylebox_override("panel",
		_make_panel_style(XBOX_PANEL_STRONG, XBOX_BORDER_STRONG, XBOX_GLOW_STRONG, 2, 22, 16.0, 28))
	menu_panel.add_theme_stylebox_override("panel",
		_make_panel_style(XBOX_PANEL, XBOX_BORDER, XBOX_GLOW, 1, 20, 14.0, 22))
	status_panel.add_theme_stylebox_override("panel",
		_make_panel_style(XBOX_PANEL, XBOX_BORDER, XBOX_GLOW, 1, 20, 14.0, 22))
	log_panel.add_theme_stylebox_override("panel",
		_make_panel_style(XBOX_PANEL, XBOX_BORDER, XBOX_GLOW, 1, 20, 14.0, 22))

	_apply_button_theme(back_button, false)
	_apply_button_theme(clear_log_button, false)

	title_label.add_theme_color_override("font_color", XBOX_ACCENT_BRIGHT)
	tagline_label.add_theme_color_override("font_color", XBOX_TEXT_SOFT)
	breadcrumb_label.add_theme_color_override("font_color", XBOX_TEXT_SOFT)
	runtime_status_label.add_theme_color_override("font_color", XBOX_STATUS_IDLE)
	group_title_label.add_theme_color_override("font_color", XBOX_ACCENT)
	selected_heading_label.add_theme_color_override("font_color", XBOX_ACCENT)
	state_heading_label.add_theme_color_override("font_color", XBOX_ACCENT)
	log_title_label.add_theme_color_override("font_color", XBOX_ACCENT)
	group_description.add_theme_color_override("default_color", XBOX_TEXT_SOFT)
	selected_details.add_theme_color_override("default_color", XBOX_TEXT)
	state_details.add_theme_color_override("default_color", XBOX_TEXT)
	event_log.add_theme_color_override("default_color", XBOX_TEXT_SOFT)

func _install_background_layers() -> void:
	if has_node("BackgroundBase"):
		return

	var background := ColorRect.new()
	background.name = "BackgroundBase"
	background.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	background.color = XBOX_BACKGROUND
	background.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(background)
	move_child(background, 0)

	var top_glow := Panel.new()
	top_glow.name = "TopGlow"
	top_glow.anchor_left = 0.5
	top_glow.anchor_right = 0.5
	top_glow.offset_left = -320.0
	top_glow.offset_top = -32.0
	top_glow.offset_right = 320.0
	top_glow.offset_bottom = 108.0
	top_glow.mouse_filter = Control.MOUSE_FILTER_IGNORE
	top_glow.add_theme_stylebox_override("panel",
		_make_glow_style(Color(0.52, 0.96, 0.18, 0.10), Color(0.29, 0.84, 0.12, 0.36), 88))
	add_child(top_glow)
	move_child(top_glow, 1)

	var bottom_glow := Panel.new()
	bottom_glow.name = "BottomGlow"
	bottom_glow.anchor_left = 0.5
	bottom_glow.anchor_top = 1.0
	bottom_glow.anchor_right = 0.5
	bottom_glow.anchor_bottom = 1.0
	bottom_glow.offset_left = -280.0
	bottom_glow.offset_top = -96.0
	bottom_glow.offset_right = 280.0
	bottom_glow.offset_bottom = -12.0
	bottom_glow.mouse_filter = Control.MOUSE_FILTER_IGNORE
	bottom_glow.add_theme_stylebox_override("panel",
		_make_glow_style(Color(0.60, 0.98, 0.22, 0.16), Color(0.33, 0.96, 0.14, 0.46), 96))
	add_child(bottom_glow)
	move_child(bottom_glow, 2)

func _make_panel_style(background: Color, border: Color, shadow: Color,
		border_width: int = 1, corner_radius: int = 18, padding: float = 14.0,
		shadow_size: int = 20) -> StyleBoxFlat:
	var style := StyleBoxFlat.new()
	style.bg_color = background
	style.border_color = border
	style.border_width_left = border_width
	style.border_width_top = border_width
	style.border_width_right = border_width
	style.border_width_bottom = border_width
	style.corner_radius_top_left = corner_radius
	style.corner_radius_top_right = corner_radius
	style.corner_radius_bottom_right = corner_radius
	style.corner_radius_bottom_left = corner_radius
	style.shadow_color = shadow
	style.shadow_size = shadow_size
	style.shadow_offset = Vector2.ZERO
	style.content_margin_left = padding
	style.content_margin_top = padding
	style.content_margin_right = padding
	style.content_margin_bottom = padding
	return style

func _make_glow_style(background: Color, shadow: Color, shadow_size: int) -> StyleBoxFlat:
	var style := StyleBoxFlat.new()
	style.bg_color = background
	style.corner_radius_top_left = 120
	style.corner_radius_top_right = 120
	style.corner_radius_bottom_right = 120
	style.corner_radius_bottom_left = 120
	style.shadow_color = shadow
	style.shadow_size = shadow_size
	style.shadow_offset = Vector2.ZERO
	return style

func _make_button_style(background: Color, border: Color, shadow: Color) -> StyleBoxFlat:
	var style := StyleBoxFlat.new()
	style.bg_color = background
	style.border_color = border
	style.border_width_left = 1
	style.border_width_top = 1
	style.border_width_right = 1
	style.border_width_bottom = 1
	style.corner_radius_top_left = 14
	style.corner_radius_top_right = 14
	style.corner_radius_bottom_right = 14
	style.corner_radius_bottom_left = 14
	style.shadow_color = shadow
	style.shadow_size = 16
	style.shadow_offset = Vector2.ZERO
	style.content_margin_left = 18.0
	style.content_margin_top = 10.0
	style.content_margin_right = 18.0
	style.content_margin_bottom = 10.0
	return style

func _apply_button_theme(button: Button, primary: bool) -> void:
	var normal_background = XBOX_BUTTON if primary else XBOX_BUTTON_SUBTLE
	var hover_background = XBOX_BUTTON_HOVER if primary else XBOX_BUTTON_SUBTLE_HOVER
	button.add_theme_stylebox_override("normal",
		_make_button_style(normal_background, XBOX_BORDER, XBOX_GLOW))
	button.add_theme_stylebox_override("hover",
		_make_button_style(hover_background, XBOX_BORDER_STRONG, XBOX_GLOW_STRONG))
	button.add_theme_stylebox_override("pressed",
		_make_button_style(XBOX_BUTTON_PRESSED, XBOX_BORDER_STRONG, XBOX_GLOW_STRONG))
	button.add_theme_stylebox_override("disabled",
		_make_button_style(XBOX_BUTTON_DISABLED, XBOX_BORDER, Color(0.0, 0.0, 0.0, 0.0)))
	button.add_theme_stylebox_override("focus",
		_make_button_style(hover_background, XBOX_BORDER_STRONG, XBOX_GLOW_STRONG))
	button.add_theme_color_override("font_color", XBOX_TEXT)
	button.add_theme_color_override("font_hover_color", XBOX_TEXT)
	button.add_theme_color_override("font_pressed_color", XBOX_TEXT)
	button.add_theme_color_override("font_disabled_color", XBOX_TEXT_SOFT)

func _style_scenario_card(card: PanelContainer, title: Label, description: RichTextLabel,
		button: Button, is_group: bool) -> void:
	var card_background = XBOX_PANEL_GROUP if is_group else XBOX_PANEL_CARD
	var card_border = XBOX_BORDER_STRONG if is_group else XBOX_BORDER
	card.add_theme_stylebox_override("panel",
		_make_panel_style(card_background, card_border, XBOX_GLOW, 1, 20, 14.0, 18))
	title.add_theme_color_override("font_color", XBOX_ACCENT if is_group else XBOX_TEXT)
	description.add_theme_color_override("default_color", XBOX_TEXT_SOFT)
	_apply_button_theme(button, true)

func _create_scenario_card(entry: Dictionary) -> Control:
	var card = PanelContainer.new()
	card.custom_minimum_size = Vector2(0, 170)
	card.size_flags_horizontal = Control.SIZE_EXPAND_FILL

	var layout = VBoxContainer.new()
	layout.add_theme_constant_override("separation", 8)
	card.add_child(layout)

	var title = Label.new()
	title.text = str(entry.get("title", "Scenario"))
	title.add_theme_font_size_override("font_size", 20)
	layout.add_child(title)

	var description = RichTextLabel.new()
	description.bbcode_enabled = false
	description.fit_content = true
	description.scroll_active = false
	description.size_flags_vertical = Control.SIZE_EXPAND_FILL
	description.text = str(entry.get("description", ""))
	layout.add_child(description)

	var button = Button.new()
	if _entry_has_children(entry):
		button.text = "Open Group"
	else:
		button.text = "Run Scenario"
	button.pressed.connect(_on_entry_pressed.bind(entry))
	layout.add_child(button)
	_style_scenario_card(card, title, description, button, _entry_has_children(entry))

	return card

func _on_entry_pressed(entry: Dictionary) -> void:
	_selected_entry = entry
	_last_selected_status = ""
	_update_selected_details(entry, "")

	if _entry_has_children(entry):
		_scenario_stack.append(_current_group)
		_current_group = entry
		_render_current_group()
		_log_event("Opened group: %s" % str(entry.get("title", "Group")))
		return

	var action = entry.get("action", Callable())
	if action is Callable and action.is_valid():
		action.call()
	else:
		_log_event("Scenario %s has no runnable action." % str(entry.get("title", "Scenario")))

func _on_back_pressed() -> void:
	if _scenario_stack.is_empty():
		return

	_selected_entry = {}
	_last_selected_status = ""
	_current_group = _scenario_stack.pop_back()
	_render_current_group()
	_log_event("Navigated up to: %s" % str(_current_group.get("title", "GDK Launch Point")))

func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_cancel") and not _scenario_stack.is_empty():
		_on_back_pressed()
		get_viewport().set_input_as_handled()

func _on_clear_log_pressed() -> void:
	_event_log_lines.clear()
	event_log.text = ""
	_log_event("Event log cleared.")

func _update_selected_details(entry: Dictionary, status: String) -> void:
	if entry.is_empty():
		selected_details.text = "Pick a scenario or group to see more detail."
		return

	var kind = "Group" if _entry_has_children(entry) else "Scenario"
	var text = "%s\n\nType: %s\n\n%s" % [
		str(entry.get("title", "Scenario")),
		kind,
		str(entry.get("description", ""))
	]

	if status != "":
		text += "\n\nLast Result: %s" % status

	selected_details.text = text

func _refresh_state_panel() -> void:
	var gdk = _gdk()
	if gdk == null:
		runtime_status_label.text = "Runtime: addon missing"
		runtime_status_label.add_theme_color_override("font_color", XBOX_STATUS_WARN)
		tagline_label.text = "Build the addon, then bring Launch Point online."
		state_details.text = "GDK singleton unavailable.\n\nExpected file:\nres://addons/godot_gdk/godot_gdk.gdextension"
		return

	runtime_status_label.text = "Runtime: initialized" if gdk.is_initialized() else "Runtime: not initialized"
	runtime_status_label.add_theme_color_override("font_color",
		XBOX_STATUS_READY if gdk.is_initialized() else XBOX_STATUS_IDLE)

	var lines: Array[String] = []
	lines.append("Available: %s" % str(gdk.is_available()))
	lines.append("Initialized: %s" % str(gdk.is_initialized()))

	var users = gdk.users.get_users()
	lines.append("Signed-in users: %d" % users.size())

	var primary_user = gdk.users.get_primary_user()
	if primary_user:
		tagline_label.text = "%s, Launch Point is green-lit." % primary_user.gamertag
		lines.append("")
		lines.append("Primary User")
		lines.append("Gamertag: %s" % primary_user.gamertag)
		lines.append("XUID: %s" % primary_user.xuid)
		lines.append("State: %s" % primary_user.get_sign_in_state_name())
		lines.append("Age Group: %s" % primary_user.get_age_group_name())
		lines.append("Store User: %s" % str(primary_user.store_user))
	else:
		tagline_label.text = "Sign in to light up the full GDK surface."
		lines.append("")
		lines.append("Primary User")
		lines.append("No primary user")

	lines.append("")
	lines.append("Demo Achievement")
	lines.append(_get_achievement_snapshot_text(primary_user))

	lines.append("")
	lines.append("Multiplayer Activity")
	lines.append(_get_mpa_snapshot_text(primary_user))
	lines.append("Invite Events: %s" % _last_mpa_event_text)

	lines.append("")
	lines.append("GameInput")
	for gi_line in _get_gameinput_snapshot_lines():
		lines.append(gi_line)
	lines.append("Last Event: %s" % _gameinput_last_event)

	state_details.text = "\n".join(lines)

func _get_achievement_snapshot_text(user) -> String:
	var gdk = _gdk()
	if gdk == null:
		return "GDK unavailable"
	if user == null:
		return "Sign in to query achievement %s." % _demo_achievement_id

	var achievement = _find_cached_achievement(user, _demo_achievement_id)
	if achievement == null:
		return "No cached state for achievement %s." % _demo_achievement_id

	return "%s (%d%%)" % [achievement.progress_state, achievement.progress_percent]

func _get_mpa_snapshot_text(user) -> String:
	var gdk = _gdk()
	if gdk == null:
		return "GDK unavailable"
	if user == null:
		return "Sign in to set or query multiplayer activity."

	var activity = gdk.multiplayer_activity.get_cached_activity(user.xuid)
	if activity == null:
		return "No cached activity for the primary user."

	return _format_mpa_activity(activity)

func _find_cached_achievement(user, achievement_id: String):
	var gdk = _gdk()
	if gdk == null or user == null:
		return null

	for achievement in gdk.achievements.get_cached_achievements(user):
		if achievement.id == achievement_id:
			return achievement

	return null

func _format_mpa_activity(activity) -> String:
	if activity == null:
		return "No cached activity."

	var max_players = int(activity.max_players)
	var current_players = int(activity.current_players)
	var player_text = "%d/%d players" % [current_players, max_players] if max_players > 0 else "%d players" % current_players
	var restriction_text = activity.join_restriction if activity.join_restriction != "" else "unknown"
	var group_text = activity.group_id if activity.group_id != "" else "no-group"
	var connection_text = activity.connection_string if activity.connection_string != "" else "no connection string"
	return "%s • %s • %s • %s" % [player_text, restriction_text, group_text, connection_text]

func _format_invite_event(invite) -> String:
	if typeof(invite) != TYPE_DICTIONARY:
		return "Unknown invite event"

	var action = String(invite.get("action", "unknown"))
	if action == "invite_handle_accept":
		return "invite from %s to %s" % [
			String(invite.get("sender_xuid", "?")),
			String(invite.get("invited_xuid", "?"))
		]
	if action == "activity_handle_join":
		return "join from %s to %s" % [
			String(invite.get("joiner_xuid", "?")),
			String(invite.get("joinee_xuid", "?"))
		]

	return String(invite.get("raw_uri", "Unknown invite event"))

func _log_event(message: String) -> void:
	var timestamp = Time.get_time_string_from_system()
	_event_log_lines.append("[%s] %s" % [timestamp, message])

	while _event_log_lines.size() > MAX_LOG_LINES:
		_event_log_lines.remove_at(0)

	event_log.text = "\n".join(_event_log_lines)

func _set_selected_status(entry: Dictionary, status: String) -> void:
	_selected_entry = entry
	_last_selected_status = status
	_update_selected_details(entry, status)

func _describe_result(result) -> String:
	if result == null:
		return "No result object returned."

	var summary = "Success" if result.ok else "Failure"
	if result.message != "":
		summary += ": %s" % result.message

	if result.data != null:
		summary += " (data: %s)" % _describe_value(result.data)

	return summary

func _describe_value(value) -> String:
	if value == null:
		return "null"
	if value is Image:
		return "Image %s" % str(value.get_size())
	if value is Object:
		return value.get_class()
	return str(value)

func _track_async_op(entry: Dictionary, async_signal, label: String, on_complete: Callable = Callable()) -> void:
	if async_signal == null:
		var status = "%s did not return a request." % label
		_log_event(status)
		_set_selected_status(entry, status)
		return

	if typeof(async_signal) == TYPE_SIGNAL:
		async_signal.connect(_on_async_operation_completed.bind(entry, label, on_complete), CONNECT_ONE_SHOT)
		return

	var status = "%s did not return a completion signal." % label
	_log_event(status)
	_set_selected_status(entry, status)

func _on_async_operation_completed(result, entry: Dictionary, label: String, on_complete: Callable) -> void:
	var status = _describe_result(result)
	_log_event("%s -> %s" % [label, status])
	_set_selected_status(entry, status)
	_refresh_state_panel()

	if on_complete.is_valid():
		on_complete.call(result)

func _scenario_initialize_runtime() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var result = gdk.initialize()
	var status = _describe_result(result)
	_log_event("Initialize Runtime -> %s" % status)
	_set_selected_status(entry, status)
	_refresh_state_panel()

func _scenario_show_last_error() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var result = gdk.get_last_error()
	var status = _describe_result(result)
	_log_event("Show Last Error -> %s" % status)
	_set_selected_status(entry, status)

func _scenario_shutdown_runtime() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	if not gdk.is_initialized():
		var idle_status = "Runtime is already shut down."
		_log_event("Shutdown Runtime -> %s" % idle_status)
		_set_selected_status(entry, idle_status)
		return

	gdk.shutdown()
	var status = "Shutdown requested."
	_log_event("Shutdown Runtime -> %s" % status)
	_set_selected_status(entry, status)
	_refresh_state_panel()

func _scenario_silent_sign_in() -> void:
	var gdk = _gdk()
	var entry = _selected_entry
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var op = gdk.users.add_default_user_async()
	_track_async_op(entry, op, "Silent Sign-In", Callable(self, "_after_user_operation"))

func _scenario_sign_in_with_ui() -> void:
	var gdk = _gdk()
	var entry = _selected_entry
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var op = gdk.users.add_user_with_ui_async()
	_track_async_op(entry, op, "User Picker", Callable(self, "_after_user_operation"))

func _after_user_operation(result) -> void:
	if result != null and result.ok and result.data:
		_log_event("Primary user ready: %s" % result.data.gamertag)

func _scenario_log_user_summary() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var users = gdk.users.get_users()
	var primary_user = gdk.users.get_primary_user()
	var status = "Signed-in users: %d" % users.size()
	if primary_user:
		status += " | Primary user: %s (%s)" % [primary_user.gamertag, primary_user.xuid]
	else:
		status += " | No primary user"

	_log_event("Log User Summary -> %s" % status)
	_set_selected_status(entry, status)
	_refresh_state_panel()

func _scenario_load_gamer_picture() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		var status = "Sign in before requesting a gamer picture."
		_log_event("Load Gamer Picture -> %s" % status)
		_set_selected_status(entry, status)
		return

	var op = gdk.users.get_gamer_picture_async(user)
	_track_async_op(entry, op, "Load Gamer Picture", Callable(self, "_after_gamer_picture"))

func _after_gamer_picture(result) -> void:
	if result != null and result.ok and result.data is Image:
		_log_event("Gamer picture image size: %s" % str(result.data.get_size()))

func _scenario_query_achievements() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		var status = "Sign in before querying achievements."
		_log_event("Query Player Achievements -> %s" % status)
		_set_selected_status(entry, status)
		return

	var op = gdk.achievements.query_player_achievements_async(user)
	_track_async_op(entry, op, "Query Player Achievements", Callable(self, "_after_achievement_query"))

func _after_achievement_query(_result) -> void:
	_refresh_state_panel()

func _scenario_increment_achievement() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		var status = "Sign in before updating achievements."
		_log_event("Increment Demo Achievement -> %s" % status)
		_set_selected_status(entry, status)
		return

	var achievement = _find_cached_achievement(user, _demo_achievement_id)
	var next_progress = ACHIEVEMENT_STEP
	if achievement != null:
		next_progress = mini(100, int(achievement.progress_percent) + ACHIEVEMENT_STEP)

	var op = gdk.achievements.update_achievement_async(user, _demo_achievement_id, next_progress)
	_track_async_op(entry, op, "Increment Demo Achievement", Callable(self, "_after_achievement_update"))

func _after_achievement_update(_result) -> void:
	_refresh_state_panel()

func _scenario_log_achievement_snapshot() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	var status = _get_achievement_snapshot_text(user)
	_log_event("Log Achievement Snapshot -> %s" % status)
	_set_selected_status(entry, status)
	_refresh_state_panel()

func _scenario_set_mpa_activity() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		var status = "Sign in before setting multiplayer activity."
		_log_event("Set Local Activity -> %s" % status)
		_set_selected_status(entry, status)
		return

	var op = gdk.multiplayer_activity.set_activity_async(
		user,
		DEMO_MPA_CONNECTION_STRING,
		"followed",
		DEMO_MPA_MAX_PLAYERS,
		DEMO_MPA_CURRENT_PLAYERS,
		DEMO_MPA_GROUP_ID,
		false
	)
	_track_async_op(entry, op, "Set Local Activity", Callable(self, "_after_mpa_set"))

func _after_mpa_set(_result) -> void:
	_refresh_state_panel()

func _scenario_refresh_mpa_activity() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		var status = "Sign in before querying multiplayer activity."
		_log_event("Refresh Local Activity -> %s" % status)
		_set_selected_status(entry, status)
		return

	var op = gdk.multiplayer_activity.get_activities_async(user, [user.xuid])
	_track_async_op(entry, op, "Refresh Local Activity", Callable(self, "_after_mpa_refresh"))

func _after_mpa_refresh(_result) -> void:
	_refresh_state_panel()

func _scenario_log_mpa_snapshot() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	var status = _get_mpa_snapshot_text(user)
	_log_event("Log Activity Snapshot -> %s" % status)
	_set_selected_status(entry, status)
	_refresh_state_panel()

func _scenario_show_mpa_invite_ui() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		var status = "Sign in before showing the invite UI."
		_log_event("Show Invite UI -> %s" % status)
		_set_selected_status(entry, status)
		return

	var op = gdk.multiplayer_activity.show_invite_ui_async(user)
	_track_async_op(entry, op, "Show Invite UI", Callable(self, "_after_mpa_invite_ui"))

func _after_mpa_invite_ui(_result) -> void:
	_refresh_state_panel()

func _scenario_clear_mpa_activity() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		var status = "Sign in before clearing multiplayer activity."
		_log_event("Clear Local Activity -> %s" % status)
		_set_selected_status(entry, status)
		return

	var op = gdk.multiplayer_activity.delete_activity_async(user)
	_track_async_op(entry, op, "Clear Local Activity", Callable(self, "_after_mpa_clear"))

func _after_mpa_clear(_result) -> void:
	_refresh_state_panel()

func _on_runtime_initialized() -> void:
	_log_event("Runtime initialized.")
	_refresh_state_panel()

func _on_runtime_shutdown() -> void:
	_log_event("Runtime shutdown completed.")
	_refresh_state_panel()

func _on_runtime_error(result) -> void:
	_log_event("Runtime error: %s" % result.message)
	_refresh_state_panel()

func _on_user_added(user) -> void:
	_log_event("User added: %s" % user.gamertag)
	_refresh_state_panel()

func _on_user_changed(user, change_kind: String) -> void:
	_log_event("User changed (%s): %s" % [change_kind, user.gamertag])
	_refresh_state_panel()

func _on_user_removed(local_id: int) -> void:
	_log_event("User removed: %d" % local_id)
	_refresh_state_panel()

func _on_primary_user_changed(user) -> void:
	if user:
		_log_event("Primary user changed: %s" % user.gamertag)
	else:
		_log_event("Primary user cleared.")
	_refresh_state_panel()

func _on_achievements_updated(user) -> void:
	if user != null:
		_log_event("Achievements refreshed for %s." % user.gamertag)
	_refresh_state_panel()

func _on_achievement_unlocked(user, achievement_id: String) -> void:
	if user != null:
		_log_event("Achievement unlocked for %s: %s" % [user.gamertag, achievement_id])
	else:
		_log_event("Achievement unlocked: %s" % achievement_id)
	_refresh_state_panel()

func _on_mpa_activities_updated(xuids: PackedStringArray) -> void:
	var gdk = _gdk()
	var primary_user = gdk.users.get_primary_user() if gdk != null else null
	if primary_user != null and xuids.has(primary_user.xuid):
		_log_event("Multiplayer activity cache updated for %s." % primary_user.gamertag)
	elif xuids.size() > 0:
		_log_event("Multiplayer activity cache updated for %s." % ", ".join(PackedStringArray(xuids)))
	_refresh_state_panel()

func _on_pending_invite_received(invite: Dictionary) -> void:
	_last_mpa_event_text = "Pending invite — %s" % _format_invite_event(invite)
	_log_event(_last_mpa_event_text)
	_refresh_state_panel()

func _on_invite_accepted(invite: Dictionary) -> void:
	_last_mpa_event_text = "Accepted invite — %s" % _format_invite_event(invite)
	_log_event(_last_mpa_event_text)
	_refresh_state_panel()

# === GameInput integration ===========================================

func _set_scenario_status(status: String) -> void:
	_set_selected_status(_selected_entry, status)

func _bind_gameinput_signals() -> void:
	if _gameinput_signals_bound:
		return
	var gi = _gameinput()
	if gi == null:
		_log_event("GameInput addon files are not loaded yet. Build the repo before launching this sample.")
		return
	gi.device_connected.connect(_on_gameinput_device_connected)
	gi.device_disconnected.connect(_on_gameinput_device_disconnected)
	_gameinput_signals_bound = true

func _on_gameinput_device_connected(device) -> void:
	var label := _format_gameinput_device_label(device)
	_gameinput_last_event = "Connected — %s" % label
	_log_event("GameInput device connected: %s" % label)
	_refresh_state_panel()

func _on_gameinput_device_disconnected(device_id: int) -> void:
	_gameinput_last_event = "Disconnected — device #%d" % device_id
	_log_event("GameInput device disconnected: #%d" % device_id)
	_refresh_state_panel()

func _format_gameinput_device_label(device) -> String:
	if device == null:
		return "<null device>"
	var device_name: String = ""
	if device.has_method("get_display_name"):
		device_name = str(device.get_display_name())
	var device_id: int = -1
	if device.has_method("get_device_id"):
		device_id = int(device.get_device_id())
	if device_name == "":
		device_name = "Device"
	return "%s (#%d)" % [device_name, device_id]

func _get_gameinput_snapshot_lines() -> Array:
	var lines: Array = []
	var gi = _gameinput()
	if gi == null:
		lines.append("Available: false (addon missing)")
		return lines

	var initialized: bool = bool(gi.is_initialized())
	lines.append("Initialized: %s" % str(initialized))
	if not initialized:
		lines.append("Run 'Initialize GameInput' to query devices.")
		return lines

	gi.poll()
	var devices = gi.get_devices()
	lines.append("Devices: %d" % devices.size())

	var primary = gi.get_primary_device()
	if primary == null:
		lines.append("Primary: none connected")
		return lines

	lines.append("Primary: %s" % _format_gameinput_device_label(primary))
	if primary.has_method("supports_vibration"):
		lines.append("  Vibration: %s" % str(primary.supports_vibration()))
	if primary.has_method("get_battery_level"):
		var battery: float = float(primary.get_battery_level())
		if battery < 0.0:
			lines.append("  Battery: wired/unknown")
		else:
			lines.append("  Battery: %d%%" % int(round(battery * 100.0)))
	return lines

func _scenario_gameinput_initialize() -> void:
	var gi = _gameinput()
	if gi == null:
		_set_scenario_status("GameInput unavailable — build the addon first.")
		return
	_bind_gameinput_signals()
	if gi.is_initialized():
		_set_scenario_status("GameInput already initialized.")
		_log_event("GameInput already initialized.")
		_refresh_state_panel()
		return
	var ok: bool = bool(gi.initialize())
	if ok:
		_set_scenario_status("GameInput.initialize() succeeded.")
		_log_event("GameInput initialized.")
	else:
		_set_scenario_status("GameInput.initialize() returned false (no GameInput available).")
		_log_event("GameInput.initialize() returned false; check editor output / system requirements.")
	_refresh_state_panel()

func _scenario_gameinput_shutdown() -> void:
	var gi = _gameinput()
	if gi == null:
		_set_scenario_status("GameInput unavailable.")
		return
	gi.shutdown()
	_set_scenario_status("GameInput.shutdown() called.")
	_log_event("GameInput shutdown.")
	_refresh_state_panel()

func _scenario_gameinput_list_devices() -> void:
	var gi = _gameinput()
	if gi == null:
		_set_scenario_status("GameInput unavailable.")
		return
	if not gi.is_initialized():
		_set_scenario_status("GameInput not initialized — run 'Initialize GameInput' first.")
		_log_event("GameInput.list_devices skipped — runtime not initialized.")
		return
	gi.poll()
	var devices = gi.get_devices()
	if devices.size() == 0:
		_set_scenario_status("No GameInput devices connected.")
		_log_event("GameInput.list_devices: 0 devices.")
		return
	var summaries: Array = []
	for d in devices:
		summaries.append(_format_gameinput_device_label(d))
	_set_scenario_status("Found %d device(s): %s" % [devices.size(), ", ".join(summaries)])
	_log_event("GameInput.list_devices: %d device(s) — %s" % [devices.size(), ", ".join(summaries)])

func _scenario_gameinput_inspect_primary() -> void:
	var gi = _gameinput()
	if gi == null:
		_set_scenario_status("GameInput unavailable.")
		return
	if not gi.is_initialized():
		_set_scenario_status("GameInput not initialized.")
		return
	gi.poll()
	var primary = gi.get_primary_device()
	if primary == null:
		_set_scenario_status("No primary GameInput device.")
		_log_event("GameInput.inspect_primary: no primary device.")
		return
	var info: Dictionary = primary.get_device_info() if primary.has_method("get_device_info") else {}
	var battery: float = -1.0
	if primary.has_method("get_battery_level"):
		battery = float(primary.get_battery_level())
	var supports_vibration: bool = false
	if primary.has_method("supports_vibration"):
		supports_vibration = bool(primary.supports_vibration())
	var battery_text: String = "wired/unknown" if battery < 0.0 else "%d%%" % int(round(battery * 100.0))
	var summary: String = "%s — vendor=0x%04X product=0x%04X battery=%s vibration=%s" % [
		_format_gameinput_device_label(primary),
		int(info.get("vendor_id", 0)),
		int(info.get("product_id", 0)),
		battery_text,
		str(supports_vibration)
	]
	_set_scenario_status(summary)
	_log_event("GameInput.inspect_primary: %s" % summary)

func _scenario_gameinput_rumble_pulse() -> void:
	var gi = _gameinput()
	if gi == null:
		_set_scenario_status("GameInput unavailable.")
		return
	if not gi.is_initialized():
		_set_scenario_status("GameInput not initialized.")
		return
	gi.poll()
	var primary = gi.get_primary_device()
	if primary == null:
		_set_scenario_status("No primary GameInput device.")
		return
	if not bool(primary.supports_vibration()):
		_set_scenario_status("Primary device does not report rumble support.")
		_log_event("GameInput.rumble_pulse skipped — primary device does not support vibration.")
		return
	var ok: bool = bool(gi.set_vibration(primary, RUMBLE_DEMO_LOW, RUMBLE_DEMO_HIGH))
	if not ok:
		_set_scenario_status("set_vibration() returned false.")
		_log_event("GameInput.set_vibration returned false.")
		return
	_set_scenario_status("Rumbling primary device (low=%.2f high=%.2f) for %.2fs." % [
		RUMBLE_DEMO_LOW, RUMBLE_DEMO_HIGH, RUMBLE_DEMO_DURATION
	])
	_log_event("GameInput.rumble_pulse started on %s." % _format_gameinput_device_label(primary))
	await get_tree().create_timer(RUMBLE_DEMO_DURATION).timeout
	var still = gi.get_primary_device()
	if still != null:
		gi.stop_haptics(still)
	_log_event("GameInput.rumble_pulse stopped.")

func _scenario_gameinput_stop_rumble() -> void:
	var gi = _gameinput()
	if gi == null:
		_set_scenario_status("GameInput unavailable.")
		return
	if not gi.is_initialized():
		_set_scenario_status("GameInput not initialized.")
		return
	gi.poll()
	var primary = gi.get_primary_device()
	if primary == null:
		_set_scenario_status("No primary GameInput device.")
		return
	gi.stop_haptics(primary)
	_set_scenario_status("Stopped haptics on primary device.")
	_log_event("GameInput.stop_haptics called on %s." % _format_gameinput_device_label(primary))
