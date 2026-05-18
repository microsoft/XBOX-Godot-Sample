extends GutTest
## GUT coverage for `core/packaging_cli.gd` — the pure argv parser used by
## the headless runner. Tests pin the verb-flag matrix and the parser
## contract that the runner and docs both depend on.

const PackagingCli = preload("res://addons/godot_gdk_packaging/core/packaging_cli.gd")
const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")


# ── Verb matrix is non-empty and consistent ────────────────────────────────

func test_verbs_table_covers_every_expected_verb() -> void:
	# Pins the verb list. If a future PR adds or renames a verb, this test
	# is the gate that forces docs + tests + runner to be updated together.
	var expected: PackedStringArray = PackedStringArray([
		"pack", "genmap", "validate", "prepare_content", "export",
		"register_loose", "install", "uninstall", "launch", "terminate",
		"sandbox", "config_template", "config_editor", "store_wizard",
	])
	for verb: String in expected:
		assert_true(PackagingCli.VERBS.has(verb), "verb '%s' is defined" % verb)
	assert_eq(PackagingCli.VERBS.size(), expected.size(),
		"no extra verbs leaked in (update the expected list when adding one)")


func test_every_verb_entry_has_doc_and_flags() -> void:
	for verb: String in PackagingCli.VERBS:
		var entry: Dictionary = PackagingCli.VERBS[verb]
		assert_true(entry.has("doc"), "verb '%s' has a doc string" % verb)
		assert_true(entry.has("flags"), "verb '%s' has a flags dict" % verb)
		assert_eq(typeof(entry["flags"]), TYPE_DICTIONARY,
			"verb '%s' flags is a Dictionary" % verb)


# ── Empty argv ─────────────────────────────────────────────────────────────

func test_empty_argv_requests_help() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray())
	assert_true(parsed["ok"], "empty argv is not an error")
	assert_true(parsed["help"], "empty argv requests help")
	assert_eq(parsed["verb"], "", "no verb resolved")


func test_help_before_verb_sets_help_flag() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray(["--help"]))
	assert_true(parsed["help"], "--help is recognized")


func test_help_after_verb_sets_help_flag() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray(["pack", "--help"]))
	assert_true(parsed["help"], "--help after verb is recognized")
	assert_eq(parsed["verb"], "pack", "verb still resolved")


# ── Unknown verb / unknown flag ────────────────────────────────────────────

func test_unknown_verb_fails_with_usage_exit_code() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray(["bogus"]))
	assert_false(parsed["ok"], "unknown verb fails")
	assert_eq(parsed["error_code"], PackagingResult.EXIT_USAGE,
		"unknown verb is a usage error")
	assert_string_contains(parsed["error"], "bogus",
		"error mentions the bad verb")


func test_unknown_flag_for_verb_fails() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out", "--bogus",
	]))
	assert_false(parsed["ok"], "unknown flag fails")
	assert_eq(parsed["error_code"], PackagingResult.EXIT_USAGE,
		"unknown flag is a usage error")


# ── Required-flag enforcement ──────────────────────────────────────────────

func test_pack_requires_source_dir_and_output_dir() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray(["pack"]))
	assert_false(parsed["ok"], "missing required flags fails")
	assert_eq(parsed["error_code"], PackagingResult.EXIT_USAGE,
		"missing required is a usage error")
	assert_string_contains(parsed["error"].to_lower(), "required",
		"error mentions the requirement")


func test_pack_with_minimum_flags_passes() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
	]))
	assert_true(parsed["ok"], "minimum required flags pass")
	assert_eq(parsed["verb"], "pack", "verb resolved")
	assert_eq(parsed["options"]["source-dir"], "Build", "source-dir captured")
	assert_eq(parsed["options"]["output-dir"], "Out", "output-dir captured")


# ── Flag value coercion ────────────────────────────────────────────────────

func test_int_flag_coerces_value() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
		"--updcompat", "2",
	]))
	assert_true(parsed["ok"], "parse ok")
	assert_eq(parsed["options"]["updcompat"], 2, "int flag coerced from string")
	assert_eq(typeof(parsed["options"]["updcompat"]), TYPE_INT,
		"int flag has int type")


func test_int_flag_with_non_numeric_value_fails() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
		"--updcompat", "high",
	]))
	assert_false(parsed["ok"], "non-numeric int rejected")
	assert_eq(parsed["error_code"], PackagingResult.EXIT_USAGE,
		"coercion failure is a usage error")


func test_bool_flag_as_bare_switch_is_true() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out", "--no-prepare",
	]))
	assert_true(parsed["ok"], "parse ok")
	assert_eq(parsed["options"]["no-prepare"], true,
		"bare bool flag is true")


func test_bool_flag_with_explicit_false_is_false() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
		"--no-prepare=false",
	]))
	assert_true(parsed["ok"], "parse ok")
	assert_eq(parsed["options"]["no-prepare"], false,
		"explicit false is honored")


func test_inline_equals_value_form_is_supported() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir=Build", "--output-dir=Out", "--updcompat=1",
	]))
	assert_true(parsed["ok"], "inline = form parses")
	assert_eq(parsed["options"]["source-dir"], "Build", "string captured")
	assert_eq(parsed["options"]["updcompat"], 1, "int coerced")


func test_enum_flag_accepts_listed_value() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
		"--encrypt", "license",
	]))
	assert_true(parsed["ok"], "parse ok")
	assert_eq(parsed["options"]["encrypt"], "license", "enum value captured")


func test_enum_flag_with_key_payload_is_accepted() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
		"--encrypt", "key:foo.ekb",
	]))
	assert_true(parsed["ok"], "parse ok")
	assert_eq(parsed["options"]["encrypt"], "key:foo.ekb",
		"key:<payload> form preserved for downstream parsing")


func test_enum_flag_rejects_unlisted_value() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
		"--encrypt", "magic",
	]))
	assert_false(parsed["ok"], "unlisted enum value rejected")


# ── Defaults ───────────────────────────────────────────────────────────────

func test_pack_default_updcompat_is_three() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
	]))
	assert_eq(parsed["options"].get("updcompat", -1), 3,
		"default updcompat is 3")


func test_pack_default_encrypt_is_none() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out",
	]))
	assert_eq(parsed["options"].get("encrypt", ""), "none",
		"default encrypt is none")


# ── Runner flags ───────────────────────────────────────────────────────────

func test_no_json_runner_flag_is_recognized() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out", "--no-json",
	]))
	assert_true(parsed["ok"], "parse ok")
	assert_eq(parsed["options"]["no-json"], true,
		"--no-json captured as runner-level flag")


func test_verbose_alias_short_form_is_recognized() -> void:
	var parsed: Dictionary = PackagingCli.parse(PackedStringArray([
		"pack", "--source-dir", "Build", "--output-dir", "Out", "-v",
	]))
	assert_true(parsed["ok"], "parse ok")
	assert_eq(parsed["options"]["verbose"], true,
		"-v resolves to verbose via alias")


# ── Usage text ─────────────────────────────────────────────────────────────

func test_render_usage_lists_every_verb() -> void:
	var text: String = PackagingCli.render_usage()
	for verb: String in PackagingCli.VERBS:
		assert_string_contains(text, verb, "usage mentions verb '%s'" % verb)


func test_render_verb_usage_for_unknown_verb_returns_error() -> void:
	var text: String = PackagingCli.render_verb_usage("nope")
	assert_string_contains(text.to_lower(), "unknown",
		"unknown verb usage is reported")


func test_render_verb_usage_includes_flag_names() -> void:
	var text: String = PackagingCli.render_verb_usage("pack")
	assert_string_contains(text, "--source-dir", "pack usage mentions --source-dir")
	assert_string_contains(text, "--output-dir", "pack usage mentions --output-dir")
