@tool
extends RefCounted
## Persists the packaging dock's non-ProjectSettings state to a ConfigFile.

const DEFAULT_STATE := {
	"packaging": {
		"source_dir": "",
		"map_file": "",
		"auto_genmap": true,
		"output_dir": "",
		"content_id": "",
		"product_id": "",
		"encrypt_option": 0,
		"encrypt_key": "",
		"updcompat_option": 0,
	},
	"sandbox": {
		"sandbox_id": "",
		"test_account": "",
	},
	"export": {
		"preset_name": "",
		"clean_build": false,
	},
}


func get_default_state() -> Dictionary:
	return {
		"packaging": DEFAULT_STATE["packaging"].duplicate(true),
		"sandbox": DEFAULT_STATE["sandbox"].duplicate(true),
		"export": DEFAULT_STATE["export"].duplicate(true),
	}


func load_state(path: String) -> Dictionary:
	var state := get_default_state()
	var cfg := ConfigFile.new()
	if cfg.load(path) != OK:
		return state

	for section_name in DEFAULT_STATE:
		var section_defaults: Dictionary = DEFAULT_STATE[section_name]
		var section_state: Dictionary = state[section_name]
		for key in section_defaults:
			section_state[key] = cfg.get_value(section_name, key, section_defaults[key])

	return state


func save_state(path: String, state: Dictionary) -> Error:
	var cfg := ConfigFile.new()
	for section_name in DEFAULT_STATE:
		var section_defaults: Dictionary = DEFAULT_STATE[section_name]
		var section_state: Dictionary = state.get(section_name, {})
		for key in section_defaults:
			cfg.set_value(section_name, key, section_state.get(key, section_defaults[key]))
	return cfg.save(path)
