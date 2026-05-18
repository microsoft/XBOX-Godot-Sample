@tool
extends RefCounted
## Typed Dictionary builder for `PackagingService` verb results.
##
## Every verb method on `packaging_service.gd` returns a Dictionary matching
## the shape produced by [method make]. The headless runner mirrors the
## `exit_code` field as the process exit code and emits the whole dict as a
## single line of JSON prefixed with `PACKAGING_RESULT_JSON:` so callers can
## grep one canonical marker out of a log.
##
## Result shape:
##   verb        : String   — the verb name (e.g. "pack")
##   exit_code   : int      — 0 on success; non-zero categories defined below
##   ok          : bool     — exit_code == 0
##   message     : String   — single-line human summary
##   details     : Dictionary — verb-specific data (artifact paths, …)
##   stdout      : String   — forwarded from underlying tool; may be ""
##   stderr      : String   — forwarded from underlying tool; may be ""
##   duration_ms : int      — wall time spent inside the verb method

## Exit-code categories. Verb implementations should pick the closest fit.
const EXIT_OK            := 0   ## Success.
const EXIT_FAIL          := 1   ## Generic failure (verb ran, returned bad result).
const EXIT_USAGE         := 2   ## Bad CLI arguments / unknown verb.
const EXIT_CONFIG        := 3   ## Required config / env / file missing.
const EXIT_TOOL          := 4   ## Underlying tool (makepkg, wdapp, …) failed.
const EXIT_UNIMPLEMENTED := 5   ## Verb known but not yet implemented.

const JSON_LINE_PREFIX := "PACKAGING_RESULT_JSON:"


## Builds a result dict. `verb` is required; everything else defaults to a
## safe empty value so callers can mutate the returned dict in-place.
static func make(
		verb: String,
		exit_code: int = EXIT_OK,
		message: String = "",
		details: Dictionary = {},
		stdout: String = "",
		stderr: String = "",
		duration_ms: int = 0
) -> Dictionary:
	return {
		"verb": verb,
		"exit_code": exit_code,
		"ok": exit_code == EXIT_OK,
		"message": message,
		"details": details.duplicate(true),
		"stdout": stdout,
		"stderr": stderr,
		"duration_ms": duration_ms,
	}


## Convenience wrapper for a successful result.
static func ok(verb: String, message: String = "", details: Dictionary = {},
		stdout: String = "", duration_ms: int = 0) -> Dictionary:
	return make(verb, EXIT_OK, message, details, stdout, "", duration_ms)


## Convenience wrapper for a failed result. Defaults to [code]EXIT_FAIL[/code]
## but callers should pick the most specific category from the constants.
static func fail(verb: String, message: String, exit_code: int = EXIT_FAIL,
		stderr: String = "", details: Dictionary = {},
		stdout: String = "", duration_ms: int = 0) -> Dictionary:
	if exit_code == EXIT_OK:
		exit_code = EXIT_FAIL
	return make(verb, exit_code, message, details, stdout, stderr, duration_ms)


## Serializes a result dict to a single-line JSON string suitable for
## printing on stdout. The output has no trailing newline; callers that
## want a full line should append one themselves.
static func to_json_line(result: Dictionary) -> String:
	return JSON_LINE_PREFIX + JSON.stringify(result)


## Inverse of [method to_json_line]. Returns the parsed Dictionary or
## an empty Dictionary if the line is not a valid result marker.
static func from_json_line(line: String) -> Dictionary:
	var trimmed: String = line.strip_edges()
	if not trimmed.begins_with(JSON_LINE_PREFIX):
		return {}
	var payload: String = trimmed.substr(JSON_LINE_PREFIX.length())
	# Use JSON.new().parse() rather than JSON.parse_string() so a malformed
	# payload doesn't surface as an engine push_error (which GUT promotes to
	# a test failure).
	var parser: JSON = JSON.new()
	if parser.parse(payload) != OK:
		return {}
	var parsed: Variant = parser.data
	if typeof(parsed) != TYPE_DICTIONARY:
		return {}
	return parsed


## Returns true if `result` has every required field with the expected
## type. Used by tests to pin the contract.
static func is_valid_shape(result: Dictionary) -> bool:
	if not result.has_all(["verb", "exit_code", "ok", "message", "details",
			"stdout", "stderr", "duration_ms"]):
		return false
	if typeof(result["verb"]) != TYPE_STRING:
		return false
	if typeof(result["exit_code"]) != TYPE_INT:
		return false
	if typeof(result["ok"]) != TYPE_BOOL:
		return false
	if typeof(result["message"]) != TYPE_STRING:
		return false
	if typeof(result["details"]) != TYPE_DICTIONARY:
		return false
	if typeof(result["stdout"]) != TYPE_STRING:
		return false
	if typeof(result["stderr"]) != TYPE_STRING:
		return false
	if typeof(result["duration_ms"]) != TYPE_INT:
		return false
	return result["ok"] == (result["exit_code"] == EXIT_OK)
