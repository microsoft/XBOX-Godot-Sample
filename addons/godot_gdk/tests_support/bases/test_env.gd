class_name GdkTestEnv
extends RefCounted
## Shared environment + project-setting gating helpers for the GUT-based
## coverage suites. Imported by every `*_test_base.gd` via:
##
##     const TestEnv = preload("res://addons/godot_gdk_tests/test_env.gd")
##
## The mirrored path matches the `DEST_SUBDIR "godot_gdk_tests"` mirror call
## in the root `CMakeLists.txt`. The canonical source of truth lives at
## `addons/godot_gdk/tests_support/bases/test_env.gd`.

const LIVE_TESTS_ENV := "LIVE_TESTS"
const LIVE_WRITE_TESTS_ENV := "LIVE_WRITE_TESTS"
const LEADERBOARD_SETTLE_MSEC_SETTING := "playfab/tests/leaderboard_settle_msec"
const DEFAULT_SETTLE_MSEC := 30000
const DEFAULT_POLL_INTERVAL_MSEC := 250

static var _cached_run_id := ""


# True if LIVE_TESTS=1 in the process environment. Read directly from
# OS.get_environment so the value reflects what the orchestrator actually
# forwarded for this Godot child process. Tests guarded by this should call
# `pending(...)` (not `skip(...)`) when false so the orchestrator's "Pending"
# count stays meaningful.
static func live_tests_enabled() -> bool:
	return OS.get_environment(LIVE_TESTS_ENV) == "1"


# True if LIVE_WRITE_TESTS=1 in the process environment. Use in addition to
# LIVE_TESTS for tests that create or mutate persistent live-service state.
static func live_write_tests_enabled() -> bool:
	return live_tests_enabled() and OS.get_environment(LIVE_WRITE_TESTS_ENV) == "1"


# Generate a unique-per-run id of the form `gdkfleet-YYYYMMDDHHmmss-XXXX`.
# The XXXX suffix is a 4-hex random value. The first call within this Godot
# process caches the result; subsequent calls return the same value so multiple
# live write tests can correlate to one logical run id.
static func unique_run_id() -> String:
	if not _cached_run_id.is_empty():
		return _cached_run_id

	var dt := Time.get_datetime_dict_from_system()
	var stamp := "%04d%02d%02d%02d%02d%02d" % [
		int(dt.get("year", 0)),
		int(dt.get("month", 0)),
		int(dt.get("day", 0)),
		int(dt.get("hour", 0)),
		int(dt.get("minute", 0)),
		int(dt.get("second", 0)),
	]
	var suffix := "%04x" % (randi() & 0xFFFF)
	_cached_run_id = "gdkfleet-" + stamp + "-" + suffix
	return _cached_run_id


# Eventual-consistency polling for read-after-write. Calls `pollable` once,
# returns its result if non-empty / non-null, otherwise sleeps `interval_msec`
# and retries up to `total_msec`. When `total_msec` is negative (the default),
# the budget is sourced from the `playfab/tests/leaderboard_settle_msec`
# project setting, falling back to 30000ms if the setting is missing.
#
# Returns null on timeout — callers should report `pending(...)` (NOT fail)
# on null so eventual-consistency flakes don't churn the orchestrator.
#
# `pollable` may return any type; "non-empty" means: not null, not the bool
# `false`, and (if Array/Dictionary/PackedStringArray/String) not empty. The
# helper stays static by getting the SceneTree via Engine.get_main_loop().
static func poll_until(pollable: Callable, total_msec: int = -1, interval_msec: int = DEFAULT_POLL_INTERVAL_MSEC) -> Variant:
	if not pollable.is_valid():
		return null

	var budget_msec := total_msec
	if budget_msec < 0:
		if ProjectSettings.has_setting(LEADERBOARD_SETTLE_MSEC_SETTING):
			budget_msec = int(ProjectSettings.get_setting(LEADERBOARD_SETTLE_MSEC_SETTING))
		if budget_msec <= 0:
			budget_msec = DEFAULT_SETTLE_MSEC

	var step_msec := interval_msec if interval_msec > 0 else DEFAULT_POLL_INTERVAL_MSEC
	var tree := Engine.get_main_loop() as SceneTree
	var started_msec := Time.get_ticks_msec()

	while true:
		var value: Variant = pollable.call()
		if _poll_value_is_present(value):
			return value
		if Time.get_ticks_msec() - started_msec >= budget_msec:
			return null
		if tree != null:
			await tree.create_timer(float(step_msec) / 1000.0).timeout
		else:
			OS.delay_msec(step_msec)

	return null


static func _poll_value_is_present(value: Variant) -> bool:
	if value == null:
		return false
	match typeof(value):
		TYPE_BOOL:
			return bool(value)
		TYPE_STRING, TYPE_STRING_NAME:
			return not str(value).is_empty()
		TYPE_ARRAY, TYPE_PACKED_STRING_ARRAY, TYPE_PACKED_BYTE_ARRAY, \
		TYPE_PACKED_INT32_ARRAY, TYPE_PACKED_INT64_ARRAY, \
		TYPE_PACKED_FLOAT32_ARRAY, TYPE_PACKED_FLOAT64_ARRAY, \
		TYPE_PACKED_VECTOR2_ARRAY, TYPE_PACKED_VECTOR3_ARRAY, \
		TYPE_PACKED_COLOR_ARRAY:
			return value.size() > 0
		TYPE_DICTIONARY:
			return not value.is_empty()
		_:
			return true
