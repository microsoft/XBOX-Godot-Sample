extends GutTest
## GUT coverage for `core/packaging_result.gd` — the typed Dictionary
## builder used by every `PackagingService` verb.
##
## Pins:
##   * Exit-code categories are stable integers (the headless runner
##     mirrors them as process exit codes; CI scripts grep for them).
##   * The result shape has every field with the right type — both the
##     ok and fail builders produce the same shape.
##   * `to_json_line` / `from_json_line` round-trip without losing data.
##   * `is_valid_shape` accepts canonical results and rejects malformed
##     ones (catches future drift).

const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")


# ── Exit-code stability ────────────────────────────────────────────────────

func test_exit_code_constants_are_stable() -> void:
	assert_eq(PackagingResult.EXIT_OK, 0, "EXIT_OK is 0")
	assert_eq(PackagingResult.EXIT_FAIL, 1, "EXIT_FAIL is 1")
	assert_eq(PackagingResult.EXIT_USAGE, 2, "EXIT_USAGE is 2")
	assert_eq(PackagingResult.EXIT_CONFIG, 3, "EXIT_CONFIG is 3")
	assert_eq(PackagingResult.EXIT_TOOL, 4, "EXIT_TOOL is 4")
	assert_eq(PackagingResult.EXIT_UNIMPLEMENTED, 5, "EXIT_UNIMPLEMENTED is 5")


func test_json_line_prefix_is_stable() -> void:
	assert_eq(PackagingResult.JSON_LINE_PREFIX, "PACKAGING_RESULT_JSON:",
		"json line prefix is the canonical marker")


# ── make() ─────────────────────────────────────────────────────────────────

func test_make_default_is_ok_with_empty_fields() -> void:
	var r: Dictionary = PackagingResult.make("pack")
	assert_eq(r["verb"], "pack", "verb is set")
	assert_eq(r["exit_code"], 0, "default exit_code is 0")
	assert_true(r["ok"], "ok is true when exit_code is 0")
	assert_eq(r["message"], "", "default message is empty")
	assert_eq(r["details"], {}, "default details is empty")
	assert_eq(r["stdout"], "", "default stdout is empty")
	assert_eq(r["stderr"], "", "default stderr is empty")
	assert_eq(r["duration_ms"], 0, "default duration_ms is 0")


func test_make_with_nonzero_exit_code_flips_ok() -> void:
	var r: Dictionary = PackagingResult.make("pack", PackagingResult.EXIT_TOOL, "boom")
	assert_eq(r["exit_code"], 4, "exit_code carried through")
	assert_false(r["ok"], "ok is false when exit_code is not 0")
	assert_eq(r["message"], "boom", "message carried through")


func test_make_duplicates_details_so_caller_mutations_dont_leak() -> void:
	var details: Dictionary = {"key": "value"}
	var r: Dictionary = PackagingResult.make("pack", 0, "", details)
	details["key"] = "mutated"
	assert_eq(r["details"]["key"], "value", "result holds an independent copy of details")


# ── ok() / fail() ──────────────────────────────────────────────────────────

func test_ok_builds_success_result() -> void:
	var r: Dictionary = PackagingResult.ok("validate", "looks good")
	assert_eq(r["exit_code"], 0, "ok builder forces exit_code 0")
	assert_true(r["ok"], "ok flag set")
	assert_eq(r["message"], "looks good", "message carried")


func test_fail_promotes_zero_exit_code_to_generic_fail() -> void:
	var r: Dictionary = PackagingResult.fail("pack", "no", PackagingResult.EXIT_OK)
	# Calling fail() with EXIT_OK is a programming mistake; the builder
	# defends against it by promoting to EXIT_FAIL so the runner still exits
	# non-zero.
	assert_eq(r["exit_code"], PackagingResult.EXIT_FAIL,
		"fail() refuses to build a zero-exit result")
	assert_false(r["ok"], "ok flag flipped")


func test_fail_carries_stderr_and_details() -> void:
	var r: Dictionary = PackagingResult.fail("install", "nope",
		PackagingResult.EXIT_TOOL, "stderr text", {"package": "Foo"})
	assert_eq(r["exit_code"], PackagingResult.EXIT_TOOL, "exit_code carried")
	assert_eq(r["stderr"], "stderr text", "stderr carried")
	assert_eq(r["details"]["package"], "Foo", "details carried")


# ── JSON round-trip ────────────────────────────────────────────────────────

func test_json_round_trip_preserves_fields() -> void:
	var src: Dictionary = PackagingResult.ok("pack", "done",
		{"map": "/tmp/layout.xml"}, "tool stdout", 17)
	var line: String = PackagingResult.to_json_line(src)
	assert_true(line.begins_with(PackagingResult.JSON_LINE_PREFIX),
		"line carries the canonical prefix")
	var parsed: Dictionary = PackagingResult.from_json_line(line)
	assert_eq(parsed["verb"], src["verb"], "verb survives round trip")
	assert_eq(parsed["exit_code"], src["exit_code"], "exit_code survives round trip")
	assert_eq(parsed["ok"], src["ok"], "ok survives round trip")
	assert_eq(parsed["message"], src["message"], "message survives round trip")
	assert_eq(parsed["details"]["map"], src["details"]["map"],
		"details survives round trip")
	assert_eq(parsed["stdout"], src["stdout"], "stdout survives round trip")
	assert_eq(parsed["duration_ms"], src["duration_ms"],
		"duration_ms survives round trip")


func test_from_json_line_rejects_non_marker_lines() -> void:
	assert_eq(PackagingResult.from_json_line("not a marker"), {},
		"plain text is rejected")
	assert_eq(PackagingResult.from_json_line(""), {}, "empty string is rejected")


func test_from_json_line_rejects_invalid_json() -> void:
	var bad: String = PackagingResult.JSON_LINE_PREFIX + "{not valid json"
	assert_eq(PackagingResult.from_json_line(bad), {},
		"unparseable payload is rejected")


# ── is_valid_shape ─────────────────────────────────────────────────────────

func test_is_valid_shape_accepts_make_output() -> void:
	assert_true(PackagingResult.is_valid_shape(PackagingResult.make("pack")),
		"make() output passes shape check")
	assert_true(PackagingResult.is_valid_shape(
		PackagingResult.fail("pack", "x", PackagingResult.EXIT_TOOL)),
		"fail() output passes shape check")


func test_is_valid_shape_rejects_missing_fields() -> void:
	var partial: Dictionary = {"verb": "pack", "exit_code": 0}
	assert_false(PackagingResult.is_valid_shape(partial),
		"result missing fields is rejected")


func test_is_valid_shape_rejects_inconsistent_ok_flag() -> void:
	var bad: Dictionary = PackagingResult.make("pack")
	bad["ok"] = false  # exit_code is 0 but ok is false — invariant broken
	assert_false(PackagingResult.is_valid_shape(bad),
		"ok flag inconsistent with exit_code is rejected")
