extends GutTest

const EditorToolsCli = preload("res://addons/godot_gdk_editortools/core/editortools_cli.gd")
const EditorToolsServiceScript = preload("res://addons/godot_gdk_editortools/core/editortools_service.gd")

class _MinimalToolchain:
	extends RefCounted


func test_helper_suite_inventory_matches_instruction_claim() -> void:
	var helper_files := [
		"test_game_config_manager.gd",
		"test_makepkg_executor.gd",
		"test_wdapp_manager.gd",
		"test_editortools_settings_store.gd",
		"test_export_preset_catalog.gd",
		"test_editortools.gd",
	]
	for file_name: String in helper_files:
		assert_true(
			FileAccess.file_exists(ProjectSettings.globalize_path("res://tests/editortools".path_join(file_name))),
			"editor tools helper suite exists: %s" % file_name)


func test_cli_verb_table_is_the_editortools_service_public_surface() -> void:
	var service = EditorToolsServiceScript.new(_MinimalToolchain.new())
	var expected_verbs := [
		"pack",
		"genmap",
		"validate",
		"prepare_content",
		"export",
		"register_loose",
		"install",
		"uninstall",
		"launch",
		"terminate",
		"sandbox",
		"config_template",
		"config_editor",
		"store_wizard",
	]

	assert_eq(EditorToolsCli.VERBS.size(), expected_verbs.size(), "CLI verb count stays pinned")
	for verb: String in expected_verbs:
		assert_true(EditorToolsCli.VERBS.has(verb), "CLI exposes %s" % verb)
		assert_eq(service.method_for_verb(verb), "run_" + verb, "service method exists for %s" % verb)
