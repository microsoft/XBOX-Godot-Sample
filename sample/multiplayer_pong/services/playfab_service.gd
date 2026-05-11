extends Node
## Sample-local PlayFab wrapper for Pong Royale.
##
## The sample scenes use a small synchronous API for save data and leaderboard
## reads, while this autoload translates those requests to the real `PlayFab`
## singleton's async sign-in, leaderboard, and Game Saves flows.

const SaveData = preload("res://services/save_data.gd")
const PLAYFAB_EXTENSION_PATH := "res://addons/godot_playfab/godot_playfab.gdextension"
const SAVE_FILE_NAME := "pong_royale_save.json"

const MODE_ROGUELIKE := "roguelike"
const MODE_VERSUS := "versus"

## PlayFab Universal Leaderboard names. The roguelike high-score table is
## published as the "PongLB" leaderboard backed by the "PongLBScore" stat.
const LEADERBOARD_NAMES := {
	MODE_ROGUELIKE: "PongLB",
}

## Hard cap for player names displayed on the leaderboard. Xbox gamertags
## currently max out at 16 characters; we keep the same ceiling so the
## leaderboard column never overflows the row layout.
const PLAYER_NAME_MAX_LEN := 16

signal leaderboard_updated(mode: String)
signal save_committed()

var _playfab_extension: Variant = null
var _save_cache: Dictionary = {}
var _leaderboards: Dictionary = {}
var _playfab_user: Variant = null
var _active_local_id: int = 0
var _game_saves_folder: String = ""
var _game_saves_synced: bool = false
var _pending_run: Dictionary = {}
var _warning_keys: Dictionary = {}
var _gdk_signals_connected: bool = false


func _ready() -> void:
	_reset_caches()
	call_deferred("_connect_gdk_signals")
	call_deferred("request_save_refresh", false)


func request_save_refresh(prompt_for_user: bool = false) -> void:
	call_deferred("_refresh_save_async", prompt_for_user)


func request_leaderboard_refresh(mode: String, limit: int = 10, prompt_for_user: bool = false) -> void:
	call_deferred("_refresh_leaderboard_async", mode, limit, prompt_for_user)


func load_game() -> Dictionary:
	return _save_cache.duplicate(true)


func save_game(data: Dictionary, prompt_for_user: bool = false) -> void:
	var normalized: Dictionary = _normalize_save_data(data)
	_save_cache = normalized
	save_committed.emit()
	call_deferred("_save_game_async", normalized.duplicate(true), prompt_for_user)


func get_leaderboard(mode: String, limit: int = 10) -> Array:
	var board: Array = (_leaderboards.get(mode, []) as Array).duplicate(true)
	if board.size() > limit:
		board.resize(limit)
	return board


func set_pending_run(score: int, wave: int, max_combo: int) -> void:
	_pending_run = {
		"max_combo": max_combo,
		"score": score,
		"wave": wave,
	}


func take_pending_run() -> Dictionary:
	var run: Dictionary = _pending_run.duplicate(true)
	_pending_run.clear()
	return run


func submit_run_async(player_name: String, score: int, wave: int, max_combo: int) -> bool:
	var leaderboard_name: String = await _resolve_leaderboard_player_name(player_name, true)
	var save_name: String = _sanitize_name(leaderboard_name)
	var latest_data: Variant = await _refresh_save_async(true)
	var save := SaveData.new()
	if latest_data is Dictionary:
		save.load_from_dict(latest_data)
	else:
		save.load_from_dict(_save_cache)
	save.player_name = save_name
	save.record_run(score, wave)
	save.record_combo(max_combo)

	var save_ok: bool = await _save_game_async(save.to_dict(), true)
	var metadata: Dictionary = {
		"max_combo": max_combo,
		"name": leaderboard_name,
		"ts": int(Time.get_unix_time_from_system()),
		"wave": wave,
	}
	var leaderboard_ok: bool = await _submit_score_async(MODE_ROGUELIKE, leaderboard_name, score, metadata, true)
	return bool(save_ok) and bool(leaderboard_ok)


## Returns the gamertag of the signed-in Xbox user when available, otherwise
## falls back to a sanitized typed-in name. Used as the displayed leaderboard
## entry name so scores are tied to the player's Xbox identity.
func get_local_player_display_name(fallback: String = "") -> String:
	var gdk: Variant = _get_gdk()
	if gdk != null and gdk.is_initialized() and gdk.users != null:
		var primary: Variant = gdk.users.get_primary_user()
		if primary != null and bool(primary.signed_in):
			var gamertag: String = String(primary.gamertag).strip_edges()
			if not gamertag.is_empty():
				return _truncate_player_name(gamertag)
	if not fallback.is_empty():
		return _truncate_player_name(fallback.strip_edges())
	return "PLAYER"


func _resolve_leaderboard_player_name(typed_name: String, prompt_for_user: bool) -> String:
	var gdk_user: Variant = await _ensure_gdk_user(prompt_for_user)
	if gdk_user != null and bool(gdk_user.signed_in):
		var gamertag: String = String(gdk_user.gamertag).strip_edges()
		if not gamertag.is_empty():
			return _truncate_player_name(gamertag)
	if not typed_name.is_empty():
		return _truncate_player_name(typed_name.strip_edges())
	return "PLAYER"


func _truncate_player_name(value: String) -> String:
	if value.is_empty():
		return "PLAYER"
	if value.length() <= PLAYER_NAME_MAX_LEN:
		return value
	return value.substr(0, PLAYER_NAME_MAX_LEN)


func get_selected_skin() -> String:
	var save := SaveData.new()
	save.load_from_dict(_save_cache)
	return save.selected_skin


func set_selected_skin(skin_id: String) -> bool:
	var save := SaveData.new()
	save.load_from_dict(_save_cache)
	if not save.set_selected_skin(skin_id):
		return false
	save_game(save.to_dict(), false)
	return true


func is_skin_unlocked(skin_id: String) -> bool:
	var save := SaveData.new()
	save.load_from_dict(_save_cache)
	return save.is_skin_unlocked(skin_id)


func unlock_skin(skin_id: String) -> bool:
	var save := SaveData.new()
	save.load_from_dict(_save_cache)
	if not save.unlock_skin(skin_id):
		return false
	save_game(save.to_dict(), false)
	return true


func get_unlocked_skins() -> Array:
	var save := SaveData.new()
	save.load_from_dict(_save_cache)
	return save.unlocked_skins.duplicate()


func _connect_gdk_signals() -> void:
	if _gdk_signals_connected:
		return

	var gdk: Variant = _get_gdk()
	if gdk == null:
		return

	if not gdk.initialized.is_connected(_on_gdk_initialized):
		gdk.initialized.connect(_on_gdk_initialized)

	if gdk.users != null:
		if not gdk.users.user_changed.is_connected(_on_gdk_user_changed):
			gdk.users.user_changed.connect(_on_gdk_user_changed)

	_gdk_signals_connected = true


func _on_gdk_initialized() -> void:
	request_save_refresh(false)
	request_leaderboard_refresh(MODE_ROGUELIKE, 10, false)


func _on_gdk_user_changed(user: Variant, change_kind: String) -> void:
	var local_id: int = _local_id_from_object(user)
	if change_kind == "removed":
		if local_id != _active_local_id:
			return

		_reset_user_state()
		save_committed.emit()
		leaderboard_updated.emit(MODE_ROGUELIKE)
		return

	if change_kind != "added" and change_kind != "signed_in_again":
		return

	var gdk: Variant = _get_gdk()
	var primary_user: Variant = gdk.users.get_primary_user() if gdk != null and gdk.users != null else null
	if primary_user == null or _local_id_from_object(primary_user) != local_id:
		return

	if local_id != _active_local_id:
		_reset_user_state()
		save_committed.emit()
		leaderboard_updated.emit(MODE_ROGUELIKE)
		_active_local_id = local_id

	if user != null:
		request_save_refresh(false)
		request_leaderboard_refresh(MODE_ROGUELIKE, 10, false)


func _refresh_save_async(prompt_for_user: bool = false) -> Dictionary:
	var save_path: String = await _ensure_save_path(prompt_for_user)
	if save_path.is_empty():
		return _save_cache.duplicate(true)

	_save_cache = _load_save_file(save_path)
	save_committed.emit()
	return _save_cache.duplicate(true)


func _save_game_async(data: Dictionary, prompt_for_user: bool = false) -> bool:
	var normalized: Dictionary = _normalize_save_data(data)
	_save_cache = normalized
	save_committed.emit()

	var save_path: String = await _ensure_save_path(prompt_for_user)
	if save_path.is_empty():
		return false
	if not _write_save_file(save_path, normalized):
		return false

	var pf: Variant = _get_playfab()
	if pf == null:
		return false

	var user: Variant = await _ensure_playfab_user(prompt_for_user)
	if user == null:
		return false

	var description: String = _build_save_description(normalized)
	var description_result: PlayFabResult = await pf.game_saves.set_save_description_async(user, description)
	if description_result == null or not description_result.ok:
		_warn(_result_message(description_result, "Failed to update the Game Saves description."))

	var upload_result: PlayFabResult = await pf.game_saves.upload_with_ui_async(user)
	if upload_result == null or not upload_result.ok:
		_warn(_result_message(upload_result, "Failed to upload the Pong Royale save."))
		return false

	return true


func _refresh_leaderboard_async(mode: String, limit: int = 10, prompt_for_user: bool = false) -> Array:
	var leaderboard_name: String = _leaderboard_name(mode)
	if leaderboard_name.is_empty():
		return get_leaderboard(mode, limit)

	var pf: Variant = _get_playfab()
	if pf == null:
		return get_leaderboard(mode, limit)

	var user: Variant = await _ensure_playfab_user(prompt_for_user)
	if user == null:
		return get_leaderboard(mode, limit)

	var page_size: int = maxi(limit, 10)
	var result: PlayFabResult = await pf.leaderboards.get_leaderboard_async(user, leaderboard_name, 1, page_size)
	if result == null or not result.ok:
		_warn(_result_message(result, "Failed to refresh the Pong Royale leaderboard."))
		return get_leaderboard(mode, limit)

	var response: Dictionary = result.data if result.data is Dictionary else {}
	var rankings: Array = response.get("rankings", [])
	var entries: Array = []
	for ranking: Variant in rankings:
		if ranking is Dictionary:
			entries.append(_map_leaderboard_row(ranking))

	_leaderboards[mode] = entries
	leaderboard_updated.emit(mode)
	return get_leaderboard(mode, limit)


func _submit_score_async(mode: String, player_name: String, score: int, metadata: Dictionary, prompt_for_user: bool) -> bool:
	var leaderboard_name: String = _leaderboard_name(mode)
	if leaderboard_name.is_empty():
		return false

	var pf: Variant = _get_playfab()
	if pf == null:
		return false

	var user: Variant = await _ensure_playfab_user(prompt_for_user)
	if user == null:
		return false

	# NOTE: PongLB is configured server-side without per-entry metadata
	# storage, so the PlayFab service rejects any non-empty metadata blob with
	# E_PF_METADATA_LENGTH_EXCEEDED (0x892357DA). The player gamertag is
	# already surfaced via the PlayFab entity displayName populated during the
	# Xbox sign-in handshake, and lastUpdated covers the row timestamp; the
	# typed `metadata` argument and the `player_name` parameter are kept for
	# API compatibility / future use but no longer transmitted.
	var _ignored_payload := metadata
	var _ignored_player_name := player_name

	var result: PlayFabResult = await pf.leaderboards.submit_score_async(
		user,
		leaderboard_name,
		score,
		[],
		""
	)
	if result == null or not result.ok:
		_warn(_result_message(result, "Failed to submit the Pong Royale score."))
		return false

	await _refresh_leaderboard_async(mode, 10, false)
	return true


func _ensure_save_path(prompt_for_user: bool) -> String:
	var folder: String = await _ensure_game_saves_folder(prompt_for_user)
	if folder.is_empty():
		return ""
	return folder.path_join(SAVE_FILE_NAME)


func _ensure_game_saves_folder(prompt_for_user: bool) -> String:
	var pf: Variant = _get_playfab()
	if pf == null:
		return ""

	var user: Variant = await _ensure_playfab_user(prompt_for_user)
	if user == null:
		return ""

	var local_id: int = _local_id_from_object(user)
	if local_id == 0:
		_warn_once("missing_local_id", "The active PlayFab user does not expose a valid local_id.")
		return ""

	if local_id != _active_local_id:
		_reset_user_state()
		_active_local_id = local_id
		_playfab_user = user

	var folder_result: PlayFabResult = pf.game_saves.get_folder(user)
	if folder_result != null and folder_result.ok:
		_game_saves_folder = String(folder_result.data)
		if not _game_saves_folder.is_empty() and (not prompt_for_user or _game_saves_synced):
			return _game_saves_folder

	if not prompt_for_user:
		return _game_saves_folder

	var sync_result: PlayFabResult = await pf.game_saves.add_user_with_ui_async(user)
	if sync_result == null or not sync_result.ok:
		_warn(_result_message(sync_result, "Failed to sync the PlayFab Game Saves folder."))
		return _game_saves_folder

	var sync_data: Dictionary = sync_result.data if sync_result.data is Dictionary else {}
	_game_saves_folder = String(sync_data.get("folder", _game_saves_folder))
	_game_saves_synced = not _game_saves_folder.is_empty()
	return _game_saves_folder


func _ensure_playfab_user(prompt_for_user: bool) -> Variant:
	var pf: Variant = _get_playfab()
	if pf == null:
		_warn_once("missing_playfab_singleton", "PlayFab is not available in this sample build.")
		return null

	if not pf.is_initialized():
		var init_result: PlayFabResult = pf.initialize()
		if init_result == null or not init_result.ok:
			_warn_once(
				"playfab_initialize:%s" % str(init_result.code if init_result != null else "unknown"),
				_result_message(init_result, "Failed to initialize PlayFab.")
			)
			return null

	var gdk_user: Variant = await _ensure_gdk_user(prompt_for_user)
	if gdk_user == null:
		return null

	var local_id: int = _local_id_from_object(gdk_user)
	if local_id == 0:
		_warn_once("missing_gdk_local_id", "The active GDK user does not expose a valid local_id.")
		return null

	if _playfab_user != null and _local_id_from_object(_playfab_user) == local_id:
		return _playfab_user

	var existing_user: Variant = pf.users.get_user(local_id)
	if existing_user != null:
		_playfab_user = existing_user
		_active_local_id = local_id
		return existing_user

	var sign_in_result: PlayFabResult = await pf.sign_in_with_xuser_async(gdk_user)
	if sign_in_result == null or not sign_in_result.ok:
		_warn(_result_message(sign_in_result, "Failed to sign the current Xbox user into PlayFab."))
		return null

	_playfab_user = sign_in_result.data
	_active_local_id = local_id
	return _playfab_user


func _ensure_gdk_user(prompt_for_user: bool) -> Variant:
	var gdk: Variant = _get_gdk()
	if gdk == null:
		_warn_once("missing_gdk_singleton", "GDK is not available, so PlayFab sign-in cannot start.")
		return null
	if not gdk.is_initialized():
		_warn_once("gdk_not_initialized", "GDK is still initializing; PlayFab requests will retry once a user is available.")
		return null

	var user: Variant = gdk.users.get_primary_user()
	if user != null and bool(user.signed_in):
		return user
	if not prompt_for_user:
		return null

	var add_user_result: GDKResult = await gdk.users.add_user_with_ui_async()
	if add_user_result == null or not add_user_result.ok:
		_warn(_result_message(add_user_result, "Failed to choose an Xbox user for PlayFab."))
		return null

	return gdk.users.get_primary_user()


func _get_gdk() -> Variant:
	var bootstrap: Variant = get_node_or_null("/root/GDKBootstrap")
	if bootstrap != null and bootstrap.has_method("get_gdk"):
		return bootstrap.get_gdk()
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")
	return null


func _get_playfab() -> Variant:
	if Engine.has_singleton("PlayFab"):
		return Engine.get_singleton("PlayFab")

	if _playfab_extension == null and FileAccess.file_exists(PLAYFAB_EXTENSION_PATH):
		_playfab_extension = load(PLAYFAB_EXTENSION_PATH)

	if Engine.has_singleton("PlayFab"):
		return Engine.get_singleton("PlayFab")

	return null


func _normalize_save_data(data: Dictionary) -> Dictionary:
	var save := SaveData.new()
	save.load_from_dict(data)
	return save.to_dict()


func _load_save_file(path: String) -> Dictionary:
	var save := SaveData.new()
	if not FileAccess.file_exists(path):
		return save.to_dict()

	var file: FileAccess = FileAccess.open(path, FileAccess.READ)
	if file == null:
		_warn("Failed to open the Game Saves file for reading: %s" % path)
		return save.to_dict()

	var parsed: Variant = JSON.parse_string(file.get_as_text())
	file.close()
	if parsed is Dictionary:
		save.load_from_dict(parsed)
	else:
		_warn("The Game Saves file did not contain a valid JSON dictionary: %s" % path)
	return save.to_dict()


func _write_save_file(path: String, data: Dictionary) -> bool:
	var dir_error: Error = DirAccess.make_dir_recursive_absolute(path.get_base_dir())
	if dir_error != OK and dir_error != ERR_ALREADY_EXISTS:
		_warn("Failed to create the Game Saves folder: %s" % path.get_base_dir())
		return false

	var file: FileAccess = FileAccess.open(path, FileAccess.WRITE)
	if file == null:
		_warn("Failed to open the Game Saves file for writing: %s" % path)
		return false

	file.store_string(JSON.stringify(data, "\t"))
	file.close()
	return true


func _build_save_description(data: Dictionary) -> String:
	var save := SaveData.new()
	save.load_from_dict(data)
	return "%s HS %06d W%02d" % [
		_sanitize_name(save.player_name),
		save.high_score,
		save.last_wave,
	]


func _leaderboard_name(mode: String) -> String:
	return String(LEADERBOARD_NAMES.get(mode, ""))


func _map_leaderboard_row(ranking: Dictionary) -> Dictionary:
	var metadata: Dictionary = _parse_leaderboard_metadata(String(ranking.get("metadata", "")))
	var scores_variant: Variant = ranking.get("scores", [])
	var scores: Array = scores_variant if scores_variant is Array else []
	var score_value: int = int(scores[0]) if scores.size() > 0 else 0
	var name: String = String(metadata.get("name", ""))
	if name.is_empty():
		name = String(ranking.get("display_name", ""))
	if name.is_empty():
		var entity_variant: Variant = ranking.get("entity", {})
		var entity: Dictionary = entity_variant if entity_variant is Dictionary else {}
		var entity_id: String = String(entity.get("id", ""))
		# PlayFab entity ids are 16-char hex blobs; keep just the leading
		# 8 chars so the leaderboard column doesn't fill with an opaque UUID
		# when the player has no display_name set yet.
		if entity_id.length() > 8:
			entity_id = entity_id.substr(0, 8)
		name = entity_id if not entity_id.is_empty() else "PLAYER"

	return {
		"name": _truncate_player_name(name.strip_edges()),
		"score": score_value,
		"ts": metadata.get("ts", ranking.get("last_updated", "")),
	}


func _parse_leaderboard_metadata(text: String) -> Dictionary:
	if text.is_empty():
		return {}
	var parsed: Variant = JSON.parse_string(text)
	if parsed is Dictionary:
		return parsed
	return {}


func _sanitize_name(player_name: String) -> String:
	var trimmed: String = player_name.strip_edges()
	if trimmed.is_empty():
		trimmed = "PLAYER"
	return trimmed.substr(0, 8).to_upper()


func _local_id_from_object(object: Variant) -> int:
	if object == null:
		return 0

	var local_id_variant: Variant = object.get("local_id")
	if local_id_variant is int:
		return int(local_id_variant)

	if object.has_method("get_local_id"):
		var method_value: Variant = object.call("get_local_id")
		if method_value is int:
			return int(method_value)

	return 0


func _reset_caches() -> void:
	var save := SaveData.new()
	_save_cache = save.to_dict()
	_leaderboards = {
		MODE_ROGUELIKE: [],
	}
	_pending_run = {}


func _reset_user_state() -> void:
	_playfab_user = null
	_active_local_id = 0
	_game_saves_folder = ""
	_game_saves_synced = false
	_reset_caches()


func _warn(message: String) -> void:
	if message.is_empty():
		return
	push_warning("[Pong PlayFab] %s" % message)


func _warn_once(key: String, message: String) -> void:
	if _warning_keys.has(key):
		return
	_warning_keys[key] = true
	_warn(message)


func _result_message(result: Variant, fallback: String) -> String:
	if result != null and String(result.message) != "":
		return String(result.message)
	return fallback
