extends Control
## ShamWow-inspired scenario shell for the Godot GDK sample surface.

const GDK_EXTENSION_PATH = "res://addons/godot_gdk/godot_gdk.gdextension"
const DEFAULT_ACHIEVEMENT_ID = "1"
const ACHIEVEMENT_STEP = 25
const DEMO_MPA_CONNECTION_STRING = "godot-gdk-shamwow://sample-session"
const DEMO_MPA_GROUP_ID = "shamwow-mpa-group"
const DEMO_MPA_MAX_PLAYERS = 4
const DEMO_MPA_CURRENT_PLAYERS = 1
const MAX_LOG_LINES = 80

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

var _gdk_extension = null
var _gdk_load_attempted = false
var _scenario_root: Dictionary = {}
var _scenario_stack: Array[Dictionary] = []
var _current_group: Dictionary = {}
var _selected_entry: Dictionary = {}
var _event_log_lines: Array[String] = []
var _demo_achievement_id = DEFAULT_ACHIEVEMENT_ID
var _last_selected_status = ""
var _mpa_set_op = null
var _mpa_get_op = null
var _mpa_delete_op = null
var _mpa_invite_ui_op = null
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

func _ready() -> void:
	title_label.text = "GodotGDK ShamWow"
	back_button.pressed.connect(_on_back_pressed)
	clear_log_button.pressed.connect(_on_clear_log_pressed)

	_load_sample_config()
	_bind_gdk_signals()
	_scenario_root = _build_scenario_catalog()
	_current_group = _scenario_root

	_render_current_group()
	_refresh_state_panel()
	_log_event("Scenario shell ready. Open a group to explore the current GDK addon surface.")

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
		"Scenario Shell",
		"Grouped GDK scenarios for runtime, users, achievements, and multiplayer activity. Open a group to run sample actions and watch the event log update.",
		[
			_group(
				"runtime",
				"Runtime",
				"Initialize, inspect, and shut down the root GDK runtime singleton.",
				[
					_scenario("runtime_initialize", "Initialize Runtime", "Call GDK.initialize() and report the resulting GDKResult.", Callable(self, "_scenario_initialize_runtime")),
					_scenario("runtime_dispatch", "Dispatch Once", "Pump one manual dispatch tick and report how many completions were serviced.", Callable(self, "_scenario_dispatch_once")),
					_scenario("runtime_last_error", "Show Last Error", "Read GDK.get_last_error() and add the current error state to the log.", Callable(self, "_scenario_show_last_error")),
					_scenario("runtime_shutdown", "Shutdown Runtime", "Call GDK.shutdown() so the shell returns to an uninitialized state.", Callable(self, "_scenario_shutdown_runtime"))
				]
			),
			_group(
				"users",
				"Users",
				"Explore the current GDK users surface with explicit sign-in and profile scenarios.",
				[
					_scenario("users_silent_sign_in", "Silent Sign-In", "Attempt add_default_user_async() and track the returned GDKAsyncOp.", Callable(self, "_scenario_silent_sign_in")),
					_scenario("users_sign_in_ui", "User Picker", "Launch add_user_with_ui_async() for an explicit sign-in or user switch flow.", Callable(self, "_scenario_sign_in_with_ui")),
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
	_log_event("Navigated up to: %s" % str(_current_group.get("title", "Scenario Shell")))

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
		tagline_label.text = "Build the addon, then launch the sample again."
		state_details.text = "GDK singleton unavailable.\n\nExpected file:\nres://addons/godot_gdk/godot_gdk.gdextension"
		return

	runtime_status_label.text = "Runtime: initialized" if gdk.is_initialized() else "Runtime: not initialized"

	var lines: Array[String] = []
	lines.append("Available: %s" % str(gdk.is_available()))
	lines.append("Initialized: %s" % str(gdk.is_initialized()))

	var users = gdk.users.get_users()
	lines.append("Signed-in users: %d" % users.size())

	var primary_user = gdk.users.get_primary_user()
	if primary_user:
		tagline_label.text = "%s, you'll be saying \"WOW\" every time." % primary_user.gamertag
		lines.append("")
		lines.append("Primary User")
		lines.append("Gamertag: %s" % primary_user.gamertag)
		lines.append("XUID: %s" % primary_user.xuid)
		lines.append("State: %s" % primary_user.get_sign_in_state_name())
		lines.append("Age Group: %s" % primary_user.get_age_group_name())
		lines.append("Store User: %s" % str(primary_user.store_user))
	else:
		tagline_label.text = "Guest, you'll be saying \"WOW\" every time."
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

func _track_async_op(entry: Dictionary, op, label: String, on_complete: Callable = Callable()) -> void:
	if op == null:
		var status = "%s did not return an operation." % label
		_log_event(status)
		_set_selected_status(entry, status)
		return

	if op.is_done():
		_on_async_operation_completed(op.get_result(), entry, label, on_complete)
	else:
		op.completed.connect(_on_async_operation_completed.bind(entry, label, on_complete), CONNECT_ONE_SHOT)

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

func _scenario_dispatch_once() -> void:
	var entry = _selected_entry
	var gdk = _gdk()
	if gdk == null:
		_set_selected_status(entry, "GDK singleton unavailable.")
		return

	var dispatch_count = gdk.dispatch()
	var status = "dispatch() serviced %d completion(s)." % dispatch_count
	_log_event("Dispatch Once -> %s" % status)
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
	_mpa_set_op = op
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
	_mpa_get_op = op
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
	_mpa_invite_ui_op = op
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
	_mpa_delete_op = op
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

func _on_user_changed(user) -> void:
	_log_event("User changed: %s" % user.gamertag)
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
