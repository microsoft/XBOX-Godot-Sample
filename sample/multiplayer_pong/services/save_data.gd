extends RefCounted
## Typed wrapper around the dictionary persisted by `PlayFabService.save_game`.
##
## Use this when reading/writing the save in code so we have one place to
## adjust schema. Backed by a plain `Dictionary` for easy JSON round-trip.

const KEY_HIGH_SCORE := "high_score"
const KEY_TOTAL_RUNS := "total_runs"
const KEY_LAST_WAVE := "last_wave"
const KEY_LAST_SCORE := "last_score"
const KEY_PLAYER_NAME := "player_name"
const KEY_VERSION := "version"
const KEY_UNLOCKED_SKINS := "unlocked_skins"
const KEY_SELECTED_SKIN := "selected_skin"
const KEY_BEST_COMBO := "best_combo"
const KEY_BOSSES_DEFEATED := "bosses_defeated"

const CURRENT_VERSION := 2
const DEFAULT_SKIN := "xbox"
const FREE_SKINS: Array[String] = ["xbox", "classic"]

var high_score: int = 0
var total_runs: int = 0
var last_wave: int = 0
var last_score: int = 0
var player_name: String = "PLAYER"
var unlocked_skins: Array[String] = ["xbox", "classic"]
var selected_skin: String = DEFAULT_SKIN
var best_combo: int = 0
var bosses_defeated: Array[String] = []


func load_from_dict(data: Dictionary) -> void:
	high_score = int(data.get(KEY_HIGH_SCORE, 0))
	total_runs = int(data.get(KEY_TOTAL_RUNS, 0))
	last_wave = int(data.get(KEY_LAST_WAVE, 0))
	last_score = int(data.get(KEY_LAST_SCORE, 0))
	player_name = String(data.get(KEY_PLAYER_NAME, "PLAYER"))
	best_combo = int(data.get(KEY_BEST_COMBO, 0))
	# Always keep the free defaults available even on legacy saves that
	# predate the cosmetic system.
	var seen: Dictionary = {}
	unlocked_skins = []
	for s in FREE_SKINS:
		if not seen.has(s):
			unlocked_skins.append(s)
			seen[s] = true
	for raw in data.get(KEY_UNLOCKED_SKINS, []):
		var s := String(raw)
		if not seen.has(s):
			unlocked_skins.append(s)
			seen[s] = true
	selected_skin = String(data.get(KEY_SELECTED_SKIN, DEFAULT_SKIN))
	if not seen.has(selected_skin):
		selected_skin = DEFAULT_SKIN
	bosses_defeated = []
	var boss_seen: Dictionary = {}
	for raw in data.get(KEY_BOSSES_DEFEATED, []):
		var s := String(raw)
		if not boss_seen.has(s):
			bosses_defeated.append(s)
			boss_seen[s] = true


func to_dict() -> Dictionary:
	return {
		KEY_VERSION: CURRENT_VERSION,
		KEY_HIGH_SCORE: high_score,
		KEY_TOTAL_RUNS: total_runs,
		KEY_LAST_WAVE: last_wave,
		KEY_LAST_SCORE: last_score,
		KEY_PLAYER_NAME: player_name,
		KEY_UNLOCKED_SKINS: unlocked_skins.duplicate(),
		KEY_SELECTED_SKIN: selected_skin,
		KEY_BEST_COMBO: best_combo,
		KEY_BOSSES_DEFEATED: bosses_defeated.duplicate(),
	}


func record_run(final_score: int, final_wave: int) -> void:
	total_runs += 1
	last_score = final_score
	last_wave = final_wave
	if final_score > high_score:
		high_score = final_score


func record_combo(combo: int) -> void:
	if combo > best_combo:
		best_combo = combo


func record_boss(boss_id: String) -> bool:
	if boss_id == "" or bosses_defeated.has(boss_id):
		return false
	bosses_defeated.append(boss_id)
	return true


func unlock_skin(skin_id: String) -> bool:
	if skin_id == "" or unlocked_skins.has(skin_id):
		return false
	unlocked_skins.append(skin_id)
	return true


func is_skin_unlocked(skin_id: String) -> bool:
	return unlocked_skins.has(skin_id)


func set_selected_skin(skin_id: String) -> bool:
	if skin_id == "" or not is_skin_unlocked(skin_id):
		return false
	if selected_skin == skin_id:
		return false
	selected_skin = skin_id
	return true
