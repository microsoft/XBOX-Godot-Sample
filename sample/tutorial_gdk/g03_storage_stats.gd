extends Control

const AddonApi = preload("res://shared/addon_api.gd")

## GDK Tutorial 3 reference scene — Title Storage + user statistics.
##
## GDK-only surfaces, no PlayFab. Buttons drive each step:
##   - List Title Storage blob metadata, then download an existing blob
##     (GDK.title_storage.list_blob_metadata_async / download_blob_async).
##     Untrusted platforms (PC) can't upload to Title Storage, so writes
##     are omitted here — provision blobs from a console or Partner Center.
##   - Stage and flush a couple of title-managed statistics, then query
##     them back (GDK.stats.set_stat_integer / flush_stats_async /
##     query_user_stats_async).
##
## NOTE: scene scripts use `get_node("/root/XboxAuth")` instead of the bare
## `XboxAuth.` reference so the headless parse gate stays clean.
##
## Source: docs/tutorials/gdk/03-storage-stats.md

# Title Storage uses "TrustedPlatform" for binary blobs scoped to the
# signed-in user. Other valid storage types include "GlobalStorage" and
# "Universal"; see the XboxTitleStorage reference for the full set.
const STORAGE_TYPE := "TrustedPlatform"
const BLOB_PATH := "tutorial/save.bin"

# Title-managed statistics declared for the title in Partner Center.
# Substitute with statistics you registered for your own title.
const STAT_HIGH_SCORE := "HighScore"
const STAT_LEVELS_CLEARED := "LevelsCleared"

@onready var _log: RichTextLabel = $Root/LogPanel/Log
@onready var _storage_btn: Button = $Root/Buttons/StorageBtn
@onready var _stats_btn: Button = $Root/Buttons/StatsBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn

var _auth: Node = null

func _ready() -> void:
	_back_btn.pressed.connect(_on_back_pressed)
	_storage_btn.pressed.connect(_on_storage_pressed)
	_stats_btn.pressed.connect(_on_stats_pressed)

	_auth = get_node_or_null("/root/XboxAuth")
	if _auth == null:
		_append("[color=red]XboxAuth autoload missing.[/color]")
		_set_buttons_enabled(false)
		return

	if not Engine.has_singleton("GDK"):
		_append("[color=red]GDK extension is not loaded.[/color]")
		_set_buttons_enabled(false)
		return

	# Surface real-time stat changes for the tracked statistics.
	AddonApi.singleton("GDK").stats.stat_changed.connect(_on_stat_changed)

	_set_buttons_enabled(false)
	_append("Waiting for sign-in…")
	if await _auth.call("sign_in"):
		_append("Signed in.")
		_set_buttons_enabled(true)
	else:
		_append("[color=red]Sign-in failed at %s: %s[/color]" % [
				_auth.call("get_last_error_stage"),
				_auth.call("get_last_error_message")])

# --- Title Storage (Step 1) ---

func _on_storage_pressed() -> void:
	var user = _auth.get("xbox_user")
	if user == null:
		return

	# Untrusted platforms (PC) can't write Title Storage — uploads are
	# rejected with HTTP 403. Provision the blob from a console or Partner
	# Center, then read it back with the list + download surfaces below.

	# 1. List blob metadata so the developer can see what's stored.
	var list = await AddonApi.singleton("GDK").title_storage.list_blob_metadata_async(
		user, STORAGE_TYPE)
	if not list.ok:
		_append("[color=orange][Storage] list failed: %s (%s)[/color]" % [list.message, list.code])
	elif list.data != null:
		_append("[Storage] Listed blob metadata for %s." % STORAGE_TYPE)

	# 2. Download an existing blob.
	var down = await AddonApi.singleton("GDK").title_storage.download_blob_async(
		user, STORAGE_TYPE, BLOB_PATH)
	if not down.ok:
		_append("[color=orange][Storage] download failed: %s[/color]" % down.message)
		return
	var data: PackedByteArray = down.data.get("data", PackedByteArray())
	_append("[color=green][Storage] Downloaded %d bytes: \"%s\"[/color]" % [
			data.size(), data.get_string_from_utf8()])

# --- Statistics (Step 2) ---

func _on_stats_pressed() -> void:
	var user = _auth.get("xbox_user")
	if user == null:
		return

	# Stage real-time tracking so stat_changed fires once values land.
	AddonApi.singleton("GDK").stats.track_stats(
		user, PackedStringArray([STAT_HIGH_SCORE, STAT_LEVELS_CLEARED]))

	# 1. Stage a couple of title-managed statistics.
	AddonApi.singleton("GDK").stats.set_stat_integer(user, STAT_HIGH_SCORE, 12500)
	AddonApi.singleton("GDK").stats.set_stat_integer(user, STAT_LEVELS_CLEARED, 7)

	# 2. Flush the staged values to the Xbox service.
	var flush = await AddonApi.singleton("GDK").stats.flush_stats_async(user)
	if not flush.ok:
		_append("[color=orange][Stats] flush failed: %s (%s)[/color]" % [flush.message, flush.code])
		return
	_append("[Stats] Flushed HighScore=12500, LevelsCleared=7.")

	# 3. Query them back.
	var query = await AddonApi.singleton("GDK").stats.query_user_stats_async(
		user, PackedStringArray([STAT_HIGH_SCORE, STAT_LEVELS_CLEARED]))
	if not query.ok:
		_append("[color=orange][Stats] query failed: %s[/color]" % query.message)
		return
	var stats: Dictionary = query.data
	_append("[color=green][Stats] Queried back: %s[/color]" % str(stats))

func _on_stat_changed(_user, stat_name: String, value) -> void:
	_append("[Stats] tracked change: %s = %s" % [stat_name, str(value)])

func _set_buttons_enabled(enabled: bool) -> void:
	_storage_btn.disabled = not enabled
	_stats_btn.disabled = not enabled

func _append(line: String) -> void:
	_log.append_text(line + "\n")
	print(line)

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
