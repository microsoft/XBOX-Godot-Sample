extends GutTest
## GUT coverage for `config_import_plugin.gd` — drives what's possible
## headlessly against the XML fixtures under `tests/fixtures/packaging/`.
##
## Spike-finding pin: `EditorImportPlugin` rejects `new()` outside the
## editor process ("Class 'EditorImportPlugin' can only be instantiated by
## editor."). The orchestrator runs all GUT suites headlessly because
## `--editor` mode ignores `-s gut_cmdln.gd` (per `spike-report.md`). So
## the `_import` callback path can't be driven from GUT — we pin the
## importer's *contract* (recognised extensions, save path layout, no-op
## passthrough) by source inspection plus directly importing the script
## constants, and we still validate every fixture file is parseable /
## present so the related `parse_config` tests have stable inputs.

const ConfigImportPluginScript = preload("res://addons/godot_gdk_editortools/editor/config_import_plugin.gd")

const SCRIPT_PATH := "res://addons/godot_gdk_editortools/editor/config_import_plugin.gd"
const FIXTURE_DIR := "res://tests/fixtures/packaging"
const FIXTURES := [
	"valid_full.config",
	"missing_identity.config",
	"missing_resources.config",
	"malformed.config",
	"empty.config",
]


# ── Source-inspection coverage of the import plugin contract ──────────────

func test_recognized_extensions_are_pinned_in_source() -> void:
	# Pin the recognised extensions without instantiating the EditorImportPlugin.
	var src := _read_script_source()
	assert_string_contains(src, '"config"', "config extension recognised")
	assert_string_contains(src, '"cfg"', "cfg extension recognised")
	assert_string_contains(src, "PackedStringArray([\"config\", \"cfg\"])", "extension list pinned exactly")


func test_importer_metadata_pinned_in_source() -> void:
	var src := _read_script_source()
	assert_string_contains(src, 'return "gdk_packaging.config_file"', "importer name unchanged")
	assert_string_contains(src, 'return "GDK Config File"', "visible name unchanged")
	assert_string_contains(src, 'return "res"', "save extension is res")
	assert_string_contains(src, 'return "Resource"', "resource type is Resource")


func test_import_callback_writes_dummy_resource() -> void:
	# Pin: `_import` saves a fresh `Resource.new()` to `save_path + ".res"`
	# and returns the ResourceSaver.save error code. The plugin intentionally
	# does NOT parse or transform the source XML — its only job is to make
	# the file appear in the FileSystem dock.
	var src := _read_script_source()
	assert_string_contains(src, "Resource.new()", "creates an empty Resource")
	assert_string_contains(src, 'ResourceSaver.save(res, save_path + ".res")', "writes <save_path>.res")


func test_import_does_not_parse_or_modify_source() -> void:
	var src := _read_script_source()
	# The plugin must not pull in XMLParser or write to the source file.
	assert_false(src.contains("XMLParser"), "no XML parsing in the import plugin")
	assert_false(src.contains("FileAccess.open(source_file"), "source file not opened for write")


# ── Fixture inventory ────────────────────────────────────────────────────

func test_fixture_inventory_present() -> void:
	# Every fixture this test suite (and `test_editortools_content_preparer.gd`)
	# expects to exist actually does.
	for fixture in FIXTURES:
		assert_true(
			FileAccess.file_exists(FIXTURE_DIR.path_join(fixture)),
			"fixture file present: %s" % fixture)


func test_valid_fixture_parses_with_xml_parser() -> void:
	# Cross-check: the "valid_full" fixture is real XML. Parsing it via the
	# same XMLParser path that game_config_manager.parse_config() uses
	# should return OK at least once and find an Identity element.
	var parser := XMLParser.new()
	var err := parser.open(FIXTURE_DIR.path_join("valid_full.config"))
	assert_eq(err, OK, "XMLParser.open returned OK")

	var saw_identity := false
	while parser.read() == OK:
		if parser.get_node_type() == XMLParser.NODE_ELEMENT and parser.get_node_name() == "Identity":
			saw_identity = true
			break
	assert_true(saw_identity, "Identity element present in valid_full fixture")


func test_missing_identity_fixture_lacks_identity_element() -> void:
	# Pin the fixture's contract: it has TitleId/Executable but NOT Identity.
	# This is what `parse_config()` exercises against the
	# "missing fields" branch.
	var parser := XMLParser.new()
	var err := parser.open(FIXTURE_DIR.path_join("missing_identity.config"))
	assert_eq(err, OK, "missing_identity fixture opens cleanly")
	var saw_identity := false
	var saw_title_id := false
	while parser.read() == OK:
		if parser.get_node_type() == XMLParser.NODE_ELEMENT:
			match parser.get_node_name():
				"Identity":
					saw_identity = true
				"TitleId":
					saw_title_id = true
	assert_false(saw_identity, "Identity element absent (this is the contract)")
	assert_true(saw_title_id, "TitleId element still present")


func test_missing_resources_fixture_lacks_resources_element() -> void:
	var parser := XMLParser.new()
	var err := parser.open(FIXTURE_DIR.path_join("missing_resources.config"))
	assert_eq(err, OK, "missing_resources fixture opens cleanly")
	var saw_resources := false
	var saw_identity := false
	while parser.read() == OK:
		if parser.get_node_type() == XMLParser.NODE_ELEMENT:
			match parser.get_node_name():
				"Resources":
					saw_resources = true
				"Identity":
					saw_identity = true
	assert_false(saw_resources, "Resources element absent (this is the contract)")
	assert_true(saw_identity, "Identity element still present")


func test_malformed_fixture_eventually_errors_under_xml_parser() -> void:
	# Pin: the malformed fixture either fails to open or returns a non-OK
	# read code at some point. The exact error code is engine-dependent;
	# we only assert "not entirely silent".
	var parser := XMLParser.new()
	var open_err := parser.open(FIXTURE_DIR.path_join("malformed.config"))
	if open_err != OK:
		assert_ne(open_err, OK, "open() rejected malformed file")
		return

	var hit_error := false
	for _i in range(0, 256):
		var step := parser.read()
		if step != OK:
			hit_error = true
			break
	assert_true(hit_error, "XMLParser surfaced a non-OK read for the malformed fixture")


func test_empty_fixture_is_zero_bytes() -> void:
	var path := FIXTURE_DIR.path_join("empty.config")
	assert_true(FileAccess.file_exists(path), "empty fixture present")
	var f := FileAccess.open(path, FileAccess.READ)
	assert_not_null(f, "empty fixture opens")
	if f != null:
		assert_eq(f.get_length(), 0, "empty fixture has zero bytes")
		f.close()


# ── Headless instantiation pending ────────────────────────────────────────

func test_editor_only_instantiation_pending() -> void:
	# Documents WHY we don't directly invoke `_import` here: the underlying
	# `EditorImportPlugin` class refuses to instantiate outside the editor.
	# Per the spike report there is no separate editor stage in the
	# orchestrator, so this scenario is intentionally pending.
	pending("EditorImportPlugin can only be instantiated by editor; --editor mode ignores -s gut_cmdln.gd (see spike-report.md)")


func _read_script_source() -> String:
	var f := FileAccess.open(SCRIPT_PATH, FileAccess.READ)
	if f == null:
		return ""
	var text := f.get_as_text()
	f.close()
	return text

