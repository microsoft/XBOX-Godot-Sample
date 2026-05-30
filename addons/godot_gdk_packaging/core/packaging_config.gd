@tool
extends RefCounted
## Settings resolver — collapses CLI flags, .gdk_packaging.cfg,
## MicrosoftGame.config, project.godot, and built-in defaults into a single
## flat dictionary the `PackagingService` can consume.
##
## Precedence (highest wins):
##   1. CLI flags                (from packaging_cli.gd → options)
##   2. res://.gdk_packaging.cfg (via PackagingSettingsStore)
##   3. MicrosoftGame.config     (identity / product fields)
##   4. project.godot            (only the app_name / app_version pair)
##   5. Built-in defaults         (PackagingSettingsStore.get_default_state)

const PackagingSettingsStoreScript = preload("res://addons/godot_gdk_packaging/core/packaging_settings_store.gd")
const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")

const PACKAGING_SETTINGS_PATH := "res://.gdk_packaging.cfg"

## CLI-key → resolved-key mapping. The CLI uses kebab-case so the docs and
## flag table read cleanly; resolved keys are snake_case so service callers
## can use them as keyword-style indexes.
const _CLI_KEY_REMAP := {
	"source-dir":     "source_dir",
	"map-file":       "map_file",
	"output-dir":     "output_dir",
	"content-id":     "content_id",
	"product-id":     "product_id",
	"encrypt":        "encrypt",
	"encrypt-key":    "encrypt_key",
	"updcompat":      "updcompat",
	"no-prepare":     "no_prepare",
	"preset":         "preset_name",
	"release":        "release",
	"content-dir":    "content_dir",
	"package":        "package_path",
	"package-name":   "package_name",
	"aumid":          "aumid",
	"action":         "action",
	"sandbox-id":     "sandbox_id",
	"output":         "output",
	"overwrite":      "overwrite",
	"config":         "config_path",
	"verbose":        "verbose",
	"no-json":        "no_json",
}

## Resolved dict shape (every field defaults to a safe empty value):
##   project_dir       : String   — absolute path to project root (= res://)
##   app_name          : String   — from project.godot application/config/name
##   app_version       : String   — from project.godot application/config/version
##   config_path       : String   — absolute path to MicrosoftGame.config
##   config_exists     : bool
##   identity_name     : String   — Identity Name="…"
##   identity_publisher: String   — Identity Publisher="…"
##   identity_version  : String   — Identity Version="…"
##   product_id        : String   — MSStore ProductId
##   executable        : String   — Executable Name
##   source_dir        : String   — pack content directory
##   map_file          : String   — pack layout file
##   output_dir        : String   — pack/export output directory
##   content_id        : String   — pack /contentid value (CLI override; defaults to product_id)
##   encrypt           : String   — "none" | "license" | "key:<ekb>"
##   encrypt_key       : String   — EKB path (also derivable from encrypt key:…)
##   updcompat         : int      — 1, 2, or 3 (default 3)
##   no_prepare        : bool
##   preset_name       : String   — Godot Windows-Desktop export preset
##   release           : bool      — use --export-release instead of --export-debug
##   content_dir       : String   — register_loose target
##   package_path      : String   — install target .msixvc
##   package_name      : String   — uninstall/launch/terminate target
##   aumid             : String
##   action            : String   — sandbox sub-action
##   sandbox_id        : String
##   output            : String   — config_template output path
##   overwrite         : bool      — config_template overwrite
##   verbose           : bool
##   no_json           : bool
##   raw_settings      : Dictionary — the entire settings-store state for callers
##                                    that want the dock-shaped data verbatim
##   raw_config_info   : Dictionary — the parsed MicrosoftGame.config dictionary

const _FLAT_DEFAULTS := {
	"app_name": "",
	"app_version": "",
	"config_path": "",
	"config_exists": false,
	"identity_name": "",
	"identity_publisher": "",
	"identity_version": "",
	"product_id": "",
	"executable": "",
	"source_dir": "",
	"map_file": "",
	"output_dir": "",
	"content_id": "",
	"encrypt": "none",
	"encrypt_key": "",
	"updcompat": 3,
	"no_prepare": false,
	"preset_name": "",
	"release": false,
	"content_dir": "",
	"package_path": "",
	"package_name": "",
	"aumid": "",
	"action": "get",
	"sandbox_id": "",
	"output": "",
	"overwrite": false,
	"verbose": false,
	"no_json": false,
}


## Resolves a complete config dict given the parsed CLI options. The
## optional [param project_root] lets tests inject a fixture directory;
## defaults to `ProjectSettings.globalize_path("res://")` in production.
##
## [param settings_path] overrides the .gdk_packaging.cfg lookup (defaults to
## `res://.gdk_packaging.cfg`); pass an empty string to skip the layer.
##
## [param config_path_override] overrides MicrosoftGame.config lookup; if
## empty, the resolver uses `<project_root>/MicrosoftGame.config`.
static func resolve(
		cli_options: Dictionary,
		project_root: String = "",
		settings_path: String = PACKAGING_SETTINGS_PATH,
		config_path_override: String = ""
) -> Dictionary:
	var resolved: Dictionary = _FLAT_DEFAULTS.duplicate(true)
	resolved["raw_settings"] = {}
	resolved["raw_config_info"] = {}

	# Layer 4: project.godot.
	if project_root.is_empty():
		project_root = ProjectSettings.globalize_path("res://")
	resolved["project_dir"] = project_root
	resolved["app_name"] = str(ProjectSettings.get_setting("application/config/name", ""))
	resolved["app_version"] = str(ProjectSettings.get_setting("application/config/version", ""))

	# Layer 3: MicrosoftGame.config. CLI --config selects the file used by
	# this lower-precedence layer as well as the final resolved config_path.
	var config_path: String = config_path_override
	if config_path.is_empty():
		config_path = str(cli_options.get("config", ""))
	if config_path.is_empty():
		config_path = project_root.path_join("MicrosoftGame.config")
	resolved["config_path"] = config_path
	resolved["config_exists"] = FileAccess.file_exists(config_path)
	if resolved["config_exists"]:
		var config_info: Dictionary = _read_config_info(config_path)
		resolved["raw_config_info"] = config_info
		resolved["identity_name"] = config_info.get("name", "")
		resolved["identity_publisher"] = config_info.get("publisher", "")
		resolved["identity_version"] = config_info.get("version", "")
		resolved["product_id"] = config_info.get("product_id", "")
		resolved["executable"] = config_info.get("executable", "")

	# Layer 2: .gdk_packaging.cfg.
	if not settings_path.is_empty():
		var store: RefCounted = PackagingSettingsStoreScript.new()
		var state: Dictionary = store.load_state(settings_path)
		resolved["raw_settings"] = state
		_apply_settings_state(resolved, state)

	# Layer 1: CLI overrides (highest precedence).
	for cli_key: String in cli_options:
		var resolved_key: String = _CLI_KEY_REMAP.get(cli_key, cli_key.replace("-", "_"))
		var value: Variant = cli_options[cli_key]
		resolved[resolved_key] = value

	# Derived fallback: content_id defaults to product_id when neither CLI
	# nor settings supplied one.
	if str(resolved["content_id"]).is_empty():
		resolved["content_id"] = resolved["product_id"]

	# Derived fallback: split encrypt=key:<path> into encrypt + encrypt_key.
	var encrypt_str: String = str(resolved["encrypt"])
	if encrypt_str.begins_with("key:"):
		var payload: String = encrypt_str.substr(4)
		if not payload.is_empty() and str(resolved["encrypt_key"]).is_empty():
			resolved["encrypt_key"] = payload
		resolved["encrypt"] = "key"

	return resolved


# ── internal ────────────────────────────────────────────────────────────────

static func _apply_settings_state(resolved: Dictionary, state: Dictionary) -> void:
	var packaging: Dictionary = state.get("packaging", {})
	_set_if_present(resolved, "source_dir", packaging, "source_dir")
	_set_if_present(resolved, "map_file", packaging, "map_file")
	_set_if_present(resolved, "output_dir", packaging, "output_dir")
	_set_if_present(resolved, "content_id", packaging, "content_id")
	_set_if_present(resolved, "product_id", packaging, "product_id")
	_set_if_present(resolved, "encrypt_key", packaging, "encrypt_key")
	# encrypt_option / updcompat_option are integer indexes in the settings
	# store (matching the dock's option-button). Translate to the canonical
	# string / int the service uses.
	if packaging.has("encrypt_option"):
		resolved["encrypt"] = _encrypt_option_to_string(int(packaging["encrypt_option"]))
	if packaging.has("updcompat_option"):
		resolved["updcompat"] = _updcompat_option_to_int(int(packaging["updcompat_option"]))

	var sandbox_state: Dictionary = state.get("sandbox", {})
	_set_if_present(resolved, "sandbox_id", sandbox_state, "sandbox_id")

	var export_state: Dictionary = state.get("export", {})
	_set_if_present(resolved, "preset_name", export_state, "preset_name")


static func _set_if_present(resolved: Dictionary, resolved_key: String,
		source: Dictionary, source_key: String) -> void:
	if not source.has(source_key):
		return
	var value: Variant = source[source_key]
	# Don't let an empty string from the settings file blow away a value
	# we already pulled from MicrosoftGame.config or defaults.
	if typeof(value) == TYPE_STRING and (value as String).is_empty():
		return
	resolved[resolved_key] = value


static func _encrypt_option_to_string(option_index: int) -> String:
	# Matches the dock's OptionButton ordering:
	#   0 = none, 1 = license, 2 = key
	match option_index:
		1: return "license"
		2: return "key"
		_: return "none"


static func _updcompat_option_to_int(option_index: int) -> int:
	# Dock OptionButton index → makepkg /updcompat level (default 3).
	match option_index:
		0: return 3
		1: return 2
		2: return 1
		_: return 3


static func _read_config_info(config_path: String) -> Dictionary:
	# We can't reuse GameConfigManagerScript.parse_config() because it
	# requires a toolchain instance (which we don't need for parsing). The
	# parsing logic is duplicated intentionally minimally below — only the
	# fields the resolver actually needs are extracted. Tests pin both.
	var info: Dictionary = {}
	var parser: XMLParser = XMLParser.new()
	if parser.open(config_path) != OK:
		return info
	while parser.read() == OK:
		if parser.get_node_type() != XMLParser.NODE_ELEMENT:
			continue
		var node_name: String = parser.get_node_name()
		if node_name == "Identity":
			for i: int in parser.get_attribute_count():
				match parser.get_attribute_name(i):
					"Name":
						info["name"] = parser.get_attribute_value(i)
					"Publisher":
						info["publisher"] = parser.get_attribute_value(i)
					"Version":
						info["version"] = parser.get_attribute_value(i)
		elif node_name == "Executable":
			for i: int in parser.get_attribute_count():
				if parser.get_attribute_name(i) == "Name":
					info["executable"] = parser.get_attribute_value(i)
		elif node_name == "MSStore":
			for i: int in parser.get_attribute_count():
				if parser.get_attribute_name(i) == "ProductId":
					info["product_id"] = parser.get_attribute_value(i)
	return info
