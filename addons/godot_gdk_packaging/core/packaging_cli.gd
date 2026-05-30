@tool
extends RefCounted
## argv → {verb, options, help, error} parser for the packaging runner.
##
## This module is pure: no file or process IO. It owns the declarative
## verb-flag matrix that both the runner and the docs use as a single
## source of truth. Tests pin the matrix in `test_packaging_cli.gd`.

const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")

## Supported flag value types. Coercion happens during parse.
##   "string" → returned as-is
##   "int"    → int(value) if value is a digit-only string; else error
##   "bool"   → true if presented as a flag (--key) OR --key=true/1/yes; else false
##   "path"   → string; runner may normalize (caller responsibility)
##   "enum:a|b|c"     → string; must be one of the listed values
##   "multi:string"   → PackedStringArray; flag may repeat
const _TYPE_STRING  := "string"
const _TYPE_INT     := "int"
const _TYPE_BOOL    := "bool"
const _TYPE_PATH    := "path"
const _TYPE_ENUM_PREFIX  := "enum:"
const _TYPE_MULTI_STRING := "multi:string"

## Common runner flags accepted by every verb. The runner consumes these
## before dispatching; verbs do not see them.
const RUNNER_FLAGS := {
	"help":      {"type": _TYPE_BOOL, "alias": "h",
				  "doc": "Print usage for the verb and exit."},
	"no-json":   {"type": _TYPE_BOOL,
				  "doc": "Suppress the PACKAGING_RESULT_JSON line on stdout."},
	"config":    {"type": _TYPE_PATH,
				  "doc": "Override the MicrosoftGame.config lookup path."},
	"verbose":   {"type": _TYPE_BOOL, "alias": "v",
				  "doc": "Print extra diagnostic lines while the verb runs."},
}

## Verb → flag-schema table. Keys are verb names; values are dictionaries:
##   doc      : String     — one-line summary
##   flags    : Dictionary — flag-name → {type, alias?, doc, default?, required?}
##   positional: Array[String] — optional positional argument names
##
## Public API (consumed by docs + tests + runner). Treat as immutable.
const VERBS := {
	"pack": {
		"doc": "Run makepkg pack to build an MSIXVC package.",
		"flags": {
			"source-dir":  {"type": _TYPE_PATH, "required": true,
							"doc": "Content directory (input)."},
			"map-file":    {"type": _TYPE_PATH, "required": false,
							"doc": "Layout file. Auto-generated if omitted."},
			"output-dir":  {"type": _TYPE_PATH, "required": true,
							"doc": "Output directory for the .msixvc file."},
			"content-id":  {"type": _TYPE_STRING,
							"doc": "Override the /contentid value."},
			"product-id":  {"type": _TYPE_STRING,
							"doc": "Override the /productid value."},
			"encrypt":     {"type": _TYPE_ENUM_PREFIX + "none|license|key",
							"default": "none",
							"doc": "Encryption mode: none, license, or key:<ekb>."},
			"encrypt-key": {"type": _TYPE_PATH,
							"doc": "EKB file path; required when --encrypt=key."},
			"updcompat":   {"type": _TYPE_INT, "default": 3,
							"doc": "/updcompat level (1, 2, or 3)."},
			"no-prepare":  {"type": _TYPE_BOOL,
							"doc": "Skip the content-prep step before pack."},
		},
		"positional": [],
	},
	"genmap": {
		"doc": "Run makepkg genmap to produce a layout file.",
		"flags": {
			"source-dir": {"type": _TYPE_PATH, "required": true,
						   "doc": "Content directory (input)."},
			"map-file":   {"type": _TYPE_PATH, "required": true,
						   "doc": "Output layout file path."},
		},
		"positional": [],
	},
	"validate": {
		"doc": "Run makepkg validate against an existing layout file.",
		"flags": {
			"source-dir": {"type": _TYPE_PATH, "required": true,
						   "doc": "Content directory (input)."},
			"map-file":   {"type": _TYPE_PATH, "required": true,
						   "doc": "Layout file to validate."},
			"output-dir": {"type": _TYPE_PATH, "required": false,
						   "doc": "Destination directory for the validation log (defaults to a sibling 'validate-out')."},
		},
		"positional": [],
	},
	"prepare_content": {
		"doc": "Copy MicrosoftGame.config + logos into a content directory.",
		"flags": {
			"content-dir": {"type": _TYPE_PATH, "required": true,
							"doc": "Content directory to populate."},
		},
		"positional": [],
	},
	"export": {
		"doc": "Run a Godot Windows-Desktop export, then prepare content.",
		"flags": {
			"preset":      {"type": _TYPE_STRING, "required": true,
							"doc": "Export preset name (must match export_presets.cfg)."},
			"output-dir":  {"type": _TYPE_PATH, "required": true,
							"doc": "Output directory for the staged build."},
			"release":     {"type": _TYPE_BOOL,
							"doc": "Use --export-release instead of --export-debug."},
			"no-prepare":  {"type": _TYPE_BOOL,
							"doc": "Skip the post-export prepare_content step."},
		},
		"positional": [],
	},
	"register_loose": {
		"doc": "Register a loose-files content directory with wdapp.",
		"flags": {
			"content-dir": {"type": _TYPE_PATH, "required": true,
							"doc": "Loose-files content directory."},
		},
		"positional": [],
	},
	"install": {
		"doc": "Install a built .msixvc package via wdapp.",
		"flags": {
			"package": {"type": _TYPE_PATH, "required": true,
						"doc": ".msixvc file to install."},
		},
		"positional": [],
	},
	"uninstall": {
		"doc": "Uninstall a registered package by package full name.",
		"flags": {
			"package-name": {"type": _TYPE_STRING, "required": true,
							 "doc": "Package full name (from `wdapp list`)."},
		},
		"positional": [],
	},
	"launch": {
		"doc": "Launch a registered package by package full name.",
		"flags": {
			"package-name": {"type": _TYPE_STRING, "required": true,
							 "doc": "Package full name (from `wdapp list`)."},
			"aumid":        {"type": _TYPE_STRING,
							 "doc": "Override the launched AUMID."},
		},
		"positional": [],
	},
	"terminate": {
		"doc": "Terminate a running package by package full name.",
		"flags": {
			"package-name": {"type": _TYPE_STRING, "required": true,
							 "doc": "Package full name (from `wdapp list`)."},
		},
		"positional": [],
	},
	"sandbox": {
		"doc": "Get, set, or reset the XBL sandbox id.",
		"flags": {
			"action":     {"type": _TYPE_ENUM_PREFIX + "get|set|retail",
						   "default": "get",
						   "doc": "Sub-action: get current id, set <id>, or restore retail."},
			"sandbox-id": {"type": _TYPE_STRING,
						   "doc": "Required when --action=set."},
		},
		"positional": [],
	},
	"config_template": {
		"doc": "Write a starter MicrosoftGame.config.",
		"flags": {
			"output":    {"type": _TYPE_PATH,
						  "doc": "Output path. Defaults to res://MicrosoftGame.config."},
			"overwrite": {"type": _TYPE_BOOL,
						  "doc": "Overwrite if the file already exists."},
		},
		"positional": [],
	},
	"config_editor": {
		"doc": "Launch GameConfigEditor.exe for the current config (detached).",
		"flags": {},
		"positional": [],
	},
	"store_wizard": {
		"doc": "Launch the GDK Store Association wizard (detached).",
		"flags": {},
		"positional": [],
	},
}


## Parses a user-args array (post `--`) into a structured result:
##
## Return shape:
##   ok        : bool
##   verb      : String        — empty if [code]ok[/code] is false and no verb was supplied
##   options   : Dictionary    — flag-name → coerced value (no leading dashes)
##   positional: PackedStringArray
##   help      : bool          — true if --help / -h was supplied
##   error     : String        — empty when ok
##   error_code: int           — one of PackagingResult.EXIT_*
static func parse(argv: PackedStringArray) -> Dictionary:
	var result: Dictionary = {
		"ok": true,
		"verb": "",
		"options": {},
		"positional": PackedStringArray(),
		"help": false,
		"error": "",
		"error_code": PackagingResult.EXIT_OK,
	}

	if argv.is_empty():
		result["help"] = true
		return result

	# Allow a leading --help / -h before the verb.
	var i: int = 0
	while i < argv.size() and (argv[i] == "--help" or argv[i] == "-h"):
		result["help"] = true
		i += 1

	if i >= argv.size():
		return result

	var verb: String = argv[i]
	i += 1
	if not VERBS.has(verb):
		result["ok"] = false
		result["error"] = "Unknown verb: %s" % verb
		result["error_code"] = PackagingResult.EXIT_USAGE
		return result
	result["verb"] = verb

	var schema: Dictionary = VERBS[verb]["flags"]
	var alias_map: Dictionary = _build_alias_map(schema)
	var runner_alias_map: Dictionary = _build_alias_map(RUNNER_FLAGS)
	var seen_positional_terminator: bool = false

	# Apply schema defaults up-front so options always have a stable shape.
	for flag_name: String in schema:
		var flag_def: Dictionary = schema[flag_name]
		if flag_def.has("default"):
			result["options"][flag_name] = flag_def["default"]

	while i < argv.size():
		var token: String = argv[i]
		i += 1

		if token == "--":
			seen_positional_terminator = true
			while i < argv.size():
				result["positional"].append(argv[i])
				i += 1
			break

		if seen_positional_terminator or not token.begins_with("-"):
			result["positional"].append(token)
			continue

		var key: String = ""
		var value: String = ""
		var has_inline_value: bool = false

		if token.begins_with("--"):
			var body: String = token.substr(2)
			var eq: int = body.find("=")
			if eq >= 0:
				key = body.substr(0, eq)
				value = body.substr(eq + 1)
				has_inline_value = true
			else:
				key = body
		else:
			# Short flag: -k or -k=value
			var short_body: String = token.substr(1)
			var short_eq: int = short_body.find("=")
			if short_eq >= 0:
				key = short_body.substr(0, short_eq)
				value = short_body.substr(short_eq + 1)
				has_inline_value = true
			else:
				key = short_body
			# Resolve short aliases.
			if alias_map.has(key):
				key = alias_map[key]
			elif runner_alias_map.has(key):
				key = runner_alias_map[key]

		# Empty key (e.g. someone passed bare "--"): treat as terminator.
		if key.is_empty():
			seen_positional_terminator = true
			continue

		var is_runner_flag: bool = RUNNER_FLAGS.has(key)
		var is_verb_flag: bool = schema.has(key)
		if not is_runner_flag and not is_verb_flag:
			result["ok"] = false
			result["error"] = "Unknown flag for verb '%s': --%s" % [verb, key]
			result["error_code"] = PackagingResult.EXIT_USAGE
			return result

		var flag_def_2: Dictionary = (
			RUNNER_FLAGS[key] if is_runner_flag else schema[key]
		)
		var flag_type: String = flag_def_2.get("type", _TYPE_STRING)

		var raw_value: String = ""
		if flag_type == _TYPE_BOOL:
			# Booleans may be presented as a bare flag (--verbose) or with an
			# explicit value (--verbose=false).
			if has_inline_value:
				raw_value = value
			else:
				raw_value = "true"
		else:
			if has_inline_value:
				raw_value = value
			else:
				if i >= argv.size():
					result["ok"] = false
					result["error"] = "Missing value for --%s" % key
					result["error_code"] = PackagingResult.EXIT_USAGE
					return result
				raw_value = argv[i]
				i += 1

		var coerced: Variant = _coerce(raw_value, flag_type)
		if typeof(coerced) == TYPE_DICTIONARY and coerced.has("__error__"):
			result["ok"] = false
			result["error"] = "Invalid value for --%s: %s" % [key, coerced["__error__"]]
			result["error_code"] = PackagingResult.EXIT_USAGE
			return result

		# Runner --help / -h is hoisted onto the top-level result so callers
		# don't have to peek into the options dict to render verb-specific
		# help. We still keep the key out of options for tidiness.
		if is_runner_flag and key == "help":
			if typeof(coerced) == TYPE_BOOL and coerced:
				result["help"] = true
			continue

		if flag_type == _TYPE_MULTI_STRING:
			var existing: PackedStringArray = result["options"].get(key, PackedStringArray())
			existing.append(coerced)
			result["options"][key] = existing
		else:
			result["options"][key] = coerced

	# Required-flag check (only for verb flags; runner flags are always optional).
	for flag_name_2: String in schema:
		var flag_def_3: Dictionary = schema[flag_name_2]
		if flag_def_3.get("required", false) and not result["options"].has(flag_name_2):
			result["ok"] = false
			result["error"] = "Missing required flag --%s for verb '%s'" % [flag_name_2, verb]
			result["error_code"] = PackagingResult.EXIT_USAGE
			return result

	return result


## Renders the top-level help text (verb list).
static func render_usage() -> String:
	var lines: PackedStringArray = PackedStringArray()
	lines.append("Usage: <verb> [--flag value] [--flag=value] [-- positional ...]")
	lines.append("")
	lines.append("Common flags (accepted by every verb):")
	for runner_flag: String in RUNNER_FLAGS:
		var info: Dictionary = RUNNER_FLAGS[runner_flag]
		lines.append("  --%-12s %s" % [runner_flag, info.get("doc", "")])
	lines.append("")
	lines.append("Verbs:")
	var verb_names: Array = VERBS.keys()
	verb_names.sort()
	for verb_name: String in verb_names:
		lines.append("  %-18s %s" % [verb_name, VERBS[verb_name].get("doc", "")])
	lines.append("")
	lines.append("Use `<verb> --help` for verb-specific flags.")
	return "\n".join(lines)


## Renders verb-specific help text.
static func render_verb_usage(verb: String) -> String:
	if not VERBS.has(verb):
		return "Unknown verb: %s" % verb
	var schema: Dictionary = VERBS[verb]
	var lines: PackedStringArray = PackedStringArray()
	lines.append("Verb: %s" % verb)
	lines.append("  %s" % schema.get("doc", ""))
	lines.append("")
	if schema["flags"].is_empty():
		lines.append("  (no verb-specific flags)")
	else:
		lines.append("Flags:")
		var flag_names: Array = schema["flags"].keys()
		flag_names.sort()
		for flag_name: String in flag_names:
			var info: Dictionary = schema["flags"][flag_name]
			var req: String = " (required)" if info.get("required", false) else ""
			var default_part: String = ""
			if info.has("default"):
				default_part = " [default: %s]" % str(info["default"])
			lines.append("  --%-14s %s%s%s" % [
				flag_name,
				info.get("doc", ""),
				default_part,
				req,
			])
	return "\n".join(lines)


# ── internal ────────────────────────────────────────────────────────────────

static func _build_alias_map(schema: Dictionary) -> Dictionary:
	var aliases: Dictionary = {}
	for flag_name: String in schema:
		var info: Dictionary = schema[flag_name]
		if info.has("alias"):
			aliases[info["alias"]] = flag_name
	return aliases


static func _coerce(value: String, flag_type: String) -> Variant:
	if flag_type == _TYPE_STRING or flag_type == _TYPE_PATH or flag_type == _TYPE_MULTI_STRING:
		return value
	if flag_type == _TYPE_INT:
		var trimmed: String = value.strip_edges()
		if trimmed.is_empty() or not trimmed.is_valid_int():
			return {"__error__": "expected integer, got '%s'" % value}
		return int(trimmed)
	if flag_type == _TYPE_BOOL:
		var lower: String = value.to_lower().strip_edges()
		if lower in ["true", "1", "yes", "y", "on"]:
			return true
		if lower in ["false", "0", "no", "n", "off", ""]:
			return false
		return {"__error__": "expected boolean, got '%s'" % value}
	if flag_type.begins_with(_TYPE_ENUM_PREFIX):
		var options: PackedStringArray = flag_type.substr(_TYPE_ENUM_PREFIX.length()).split("|")
		# Allow a "key:<payload>" form so e.g. --encrypt=key:foo.ekb keeps
		# the payload alongside the choice for the service to split.
		var lhs: String = value
		var colon: int = value.find(":")
		if colon >= 0:
			lhs = value.substr(0, colon)
		if not options.has(lhs):
			return {"__error__": "expected one of %s, got '%s'" % [
				", ".join(options), value,
			]}
		return value
	# Unknown type: pass through as string.
	return value
