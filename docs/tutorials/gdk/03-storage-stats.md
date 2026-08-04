# GDK Tutorial 3 — Title Storage and stats

## What you'll build

Add a GDK-only storage and progression scene. The Title Storage half lists blob metadata for storage type `TrustedPlatform` and downloads an existing blob — untrusted platforms (PC) can't upload, so it only reads. The Stats half tracks two title-managed stats, stages integer values, flushes them to Xbox services, and queries them back.

## Prerequisites

- Complete [GDK Tutorial 2](02-achievement.md).
- Configure Title Storage for your Partner Center title and sandbox, and provision the blob at `tutorial/save.bin` from a console or Partner Center — untrusted platforms (PC) can't upload.
- Declare title-managed statistics named `HighScore` and `LevelsCleared`, or update the constants in the script.
- Stay in the same Xbox sandbox used by `MicrosoftGame.config`.

## Relevant addon surfaces

- [`GDKTitleStorage`](../../../addons/godot_gdk/doc_classes/GDKTitleStorage.xml) — `list_blob_metadata_async`, `download_blob_async`.
- [`GDKStats`](../../../addons/godot_gdk/doc_classes/GDKStats.xml) — `track_stats`, `set_stat_integer`, `flush_stats_async`, `query_user_stats_async`, `stat_changed`.
- [`GDKUser`](../../../addons/godot_gdk/doc_classes/GDKUser.xml) — `GdkAuth.xbox_user`.

## Steps

### Step 1 — Use the GDK-only scene script

The complete reference script below is copy-pasteable. It waits for `GdkAuth`, lists metadata and downloads an existing blob, then sets and queries stats.

```gdscript
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
## NOTE: scene scripts use `get_node("/root/GdkAuth")` instead of the bare
## `GdkAuth.` reference so the headless parse gate stays clean.
##
## Source: docs/tutorials/gdk/03-storage-stats.md

# Title Storage uses "TrustedPlatform" for binary blobs scoped to the
# signed-in user. Other valid storage types include "GlobalStorage" and
# "Universal"; see the GDKTitleStorage reference for the full set.
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

	_auth = get_node_or_null("/root/GdkAuth")
	if _auth == null:
		_append("[color=red]GdkAuth autoload missing.[/color]")
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
```

### Step 2 — Substitute your service fixture names

For your own title, keep `STORAGE_TYPE := "TrustedPlatform"` unless you intentionally configured another type. Replace `STAT_HIGH_SCORE` and `STAT_LEVELS_CLEARED` with statistic names declared for your Partner Center title.

### Step 3 — Keep storage and stats independent

The two buttons intentionally run separate flows. Storage failures should not prevent stat testing, and stat configuration failures should not block Title Storage validation.

## Verify

Run `g03_storage_stats`, click **Storage list + download**, then **Stats flush + query**. Output should show listed blob metadata, a downloaded payload, flushed values, tracked changes, and queried stat data.

## Common failures

| Output | Diagnosis | Fix |
|---|---|---|
| `download failed` | No blob provisioned, or blob path/type mismatch. Untrusted platforms (PC) can't upload, so the blob must already exist. | Provision the blob from a console or Partner Center using the same `STORAGE_TYPE` and `BLOB_PATH`. |
| `flush failed` | Statistic names are not registered for this title. | Create the stats or update the constants. |
| No `stat_changed` output | You did not track the stat names before setting/flushing. | Call `track_stats` with the same names you set. |

## Reference implementation

- Scene: [`sample/tutorial_gdk/g03_storage_stats.tscn`](../../../sample/tutorial_gdk/g03_storage_stats.tscn)
- Script: [`sample/tutorial_gdk/g03_storage_stats.gd`](../../../sample/tutorial_gdk/g03_storage_stats.gd)

## Next

Continue to [GDK Tutorial 4 — Multiplayer Activity](04-mpa.md).
