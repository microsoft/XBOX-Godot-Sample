extends RefCounted
## Test suite for the GDK packaging addon editor scripts.
## Tests XML parsing, argument construction, settings persistence,
## path normalization, and config template generation.
## Run: godot --headless --script res://tests/run_tests.gd

const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/editor/game_config_manager.gd")
const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/editor/gdk_toolchain.gd")
const MakePkgExecutorScript = preload("res://addons/godot_gdk_packaging/editor/makepkg_executor.gd")
const PackagingSettingsStoreScript = preload("res://addons/godot_gdk_packaging/editor/packaging_settings_store.gd")
const ExportPresetCatalogScript = preload("res://addons/godot_gdk_packaging/editor/export_preset_catalog.gd")
const PackagingContentPreparerScript = preload("res://addons/godot_gdk_packaging/editor/packaging_content_preparer.gd")
const WdappManagerScript = preload("res://addons/godot_gdk_packaging/editor/wdapp_manager.gd")

func run(context) -> void:
	_test_toolchain_detection(context)
	_test_config_parsing(context)
	_test_config_template_creation(context)
	_test_makepkg_argument_construction(context)
	_test_settings_persistence(context)
	_test_export_preset_parsing(context)
	_test_content_preparer_helpers(context)
	_test_wdapp_output_parsing(context)
	_test_xml_escaping(context)


# ── Toolchain Detection ─────────────────────────────────────────────────────

func _test_toolchain_detection(context) -> void:
	context.log_section("Toolchain Detection")

	var toolchain = GDKToolchainScript.new()

	# GDK should be available on dev machines with GDK installed
	if toolchain.is_gdk_available():
		context.assert_true(toolchain.get_makepkg_path() != "", "makepkg path not empty")
		context.assert_true(toolchain.get_game_config_editor_path() != "", "GameConfigEditor path not empty")
		context.assert_true(toolchain.get_bin_dir() != "", "bin dir not empty")
		context.assert_true(
			FileAccess.file_exists(toolchain.get_makepkg_path()),
			"makepkg.exe exists at detected path")

		var version = toolchain.get_gdk_version()
		if version != "":
			context.assert_true(version.length() == 6, "GDK version is 6 digits: %s" % version)
			context.assert_true(version.is_valid_int(), "GDK version is numeric: %s" % version)
		else:
			context.log_skip("GDK version", "GameDKCoreLatest env var not set")
	else:
		context.log_skip("toolchain paths", "GDK not installed")


# ── Config Parsing ──────────────────────────────────────────────────────────

func _test_config_parsing(context) -> void:
	context.log_section("Config Parsing")

	var toolchain = GDKToolchainScript.new()
	var config_mgr = GameConfigManagerScript.new(toolchain)

	# Write a test config file
	var test_xml := '<?xml version="1.0" encoding="utf-8"?>\n'
	test_xml += '<Game configVersion="1">\n'
	test_xml += '  <Identity Name="TestGame" Publisher="CN=TestPub" Version="2.0.0.0" />\n'
	test_xml += '  <TitleId>AABBCCDD</TitleId>\n'
	test_xml += '  <MSAAppId>00000000DEADBEEF</MSAAppId>\n'
	test_xml += '  <StoreId>9TESTSTOREID</StoreId>\n'
	test_xml += '  <ExecutableList>\n'
	test_xml += '    <Executable Name="TestGame.exe" Id="Game" />\n'
	test_xml += '  </ExecutableList>\n'
	test_xml += '  <ShellVisuals DefaultDisplayName="Test Game"\n'
	test_xml += '                Description="A test game"\n'
	test_xml += '                BackgroundColor="#112233"\n'
	test_xml += '                ForegroundText="dark"\n'
	test_xml += '                StoreLogo="storelogos\\StoreLogo.png"\n'
	test_xml += '                Square480x480Logo="storelogos\\Square480x480Logo.png" />\n'
	test_xml += '  <MSStore ProductId="test-product-id" />\n'
	test_xml += '</Game>\n'

	var config_path = ProjectSettings.globalize_path("res://MicrosoftGame.config")
	var had_original_config := FileAccess.file_exists(config_path)
	var original_config_contents := ""
	if had_original_config:
		var original_file = FileAccess.open(config_path, FileAccess.READ)
		if original_file == null:
			context.log_fail("backup original config", "cannot open existing MicrosoftGame.config")
			return
		original_config_contents = original_file.get_as_text()
		original_file.close()

	var f = FileAccess.open(config_path, FileAccess.WRITE)
	if f == null:
		context.log_fail("write test config", "cannot open MicrosoftGame.config")
		return
	f.store_string(test_xml)
	f.close()

	var result = config_mgr.parse_config()

	if had_original_config:
		var restore_file = FileAccess.open(config_path, FileAccess.WRITE)
		if restore_file == null:
			context.log_fail("restore original config", "cannot restore MicrosoftGame.config")
			return
		restore_file.store_string(original_config_contents)
		restore_file.close()
	else:
		var remove_err = DirAccess.remove_absolute(config_path)
		context.assert_eq(remove_err, OK, "remove temporary MicrosoftGame.config")

	context.assert_eq(result.get("config_version"), "1", "parsed configVersion")
	context.assert_eq(result.get("name"), "TestGame", "parsed Identity.Name")
	context.assert_eq(result.get("publisher"), "CN=TestPub", "parsed Identity.Publisher")
	context.assert_eq(result.get("version"), "2.0.0.0", "parsed Identity.Version")
	context.assert_eq(result.get("title_id"), "AABBCCDD", "parsed TitleId")
	context.assert_eq(result.get("executable"), "TestGame.exe", "parsed Executable.Name")
	context.assert_eq(result.get("display_name"), "Test Game", "parsed ShellVisuals.DefaultDisplayName")
	context.assert_eq(result.get("description"), "A test game", "parsed ShellVisuals.Description")
	context.assert_eq(result.get("background_color"), "#112233", "parsed ShellVisuals.BackgroundColor")
	context.assert_eq(result.get("product_id"), "test-product-id", "parsed MSStore.ProductId")

# ── Config Template ─────────────────────────────────────────────────────────

func _test_config_template_creation(context) -> void:
	context.log_section("Config Template Creation")

	var toolchain = GDKToolchainScript.new()
	var config_mgr = GameConfigManagerScript.new(toolchain)

	# Remove existing config if present
	var config_path = config_mgr.get_config_path()
	if FileAccess.file_exists(config_path):
		DirAccess.remove_absolute(config_path)

	var err = config_mgr.create_template("TestTitle", "CN=TestPub", "Test Title")
	context.assert_eq(err, OK, "create_template returns OK")
	context.assert_true(FileAccess.file_exists(config_path), "config file created on disk")

	# Verify it's valid XML
	var parser := XMLParser.new()
	err = parser.open(config_path)
	context.assert_eq(err, OK, "template is valid XML")

	# Second call should return ERR_ALREADY_EXISTS
	err = config_mgr.create_template()
	context.assert_eq(err, ERR_ALREADY_EXISTS, "create_template rejects duplicate")

	# Clean up
	DirAccess.remove_absolute(config_path)


# ── Makepkg Arguments ───────────────────────────────────────────────────────

func _test_makepkg_argument_construction(context) -> void:
	context.log_section("Makepkg Argument Construction")

	var toolchain = GDKToolchainScript.new()
	var executor = MakePkgExecutorScript.new(toolchain)
	context.assert_not_null(executor, "MakePkgExecutor created")

	var pack_args = executor.build_pack_args("C:/src", "C:/src/layout.xml", "C:/pkg", {
		"content_id": "content-id",
		"product_id": "product-id",
		"encrypt_key": "keys.lekb",
		"updcompat": 2,
	})
	context.assert_eq(pack_args[0], "pack", "pack args start with command")
	context.assert_true(pack_args.has("/contentid"), "pack args include content id")
	context.assert_true(pack_args.has("/productid"), "pack args include product id")
	context.assert_true(pack_args.has("/lk"), "pack args include custom key switch")
	context.assert_true(pack_args.has("keys.lekb"), "pack args include custom key path")
	context.assert_true(pack_args.has("/updcompat"), "pack args include updcompat")

	var genmap_args = executor.build_genmap_args("C:/build", "C:/pkg/layout.xml")
	context.assert_eq(genmap_args, PackedStringArray([
		"genmap",
		"/f", "C:/pkg/layout.xml",
		"/d", "C:/build",
	]), "genmap args match expected layout")

	var validate_args = executor.build_validate_args("C:/build/layout.xml", "C:/build", "C:/pkg")
	context.assert_eq(validate_args, PackedStringArray([
		"validate",
		"/f", "C:/build/layout.xml",
		"/d", "C:/build",
		"/pd", "C:/pkg",
		"/pc",
	]), "validate args match expected layout")


# ── Settings Persistence ────────────────────────────────────────────────────

func _test_settings_persistence(context) -> void:
	context.log_section("Settings Persistence")

	var test_path = ProjectSettings.globalize_path("res://test_settings.cfg")
	var store = PackagingSettingsStoreScript.new()
	var state = store.get_default_state()
	state["packaging"]["source_dir"] = "/test/source"
	state["packaging"]["output_dir"] = "/test/output"
	state["sandbox"]["sandbox_id"] = "XDKS.1"
	state["sandbox"]["test_account"] = "testuser@xboxtest.com"
	state["export"]["preset_name"] = "Windows Debug"
	state["export"]["clean_build"] = true

	var err = store.save_state(test_path, state)
	context.assert_eq(err, OK, "save settings file")

	var loaded_state = store.load_state(test_path)
	context.assert_eq(loaded_state["packaging"]["source_dir"], "/test/source", "source_dir persisted")
	context.assert_eq(loaded_state["packaging"]["output_dir"], "/test/output", "output_dir persisted")
	context.assert_eq(loaded_state["sandbox"]["sandbox_id"], "XDKS.1", "sandbox_id persisted")
	context.assert_eq(loaded_state["sandbox"]["test_account"], "testuser@xboxtest.com", "test_account persisted")
	context.assert_eq(loaded_state["export"]["preset_name"], "Windows Debug", "preset_name persisted")
	context.assert_true(loaded_state["export"]["clean_build"], "clean_build persisted")

	# Clean up
	DirAccess.remove_absolute(test_path)


func _test_export_preset_parsing(context) -> void:
	context.log_section("Export Preset Parsing")

	var content := '[preset.0]\nname="Windows Debug"\nplatform="Windows Desktop"\n\n'
	content += '[preset.1]\nname="Android"\nplatform="Android"\n\n'
	content += '[preset.2]\nname="Windows Release"\nplatform="Windows Desktop"\n'

	var presets = ExportPresetCatalogScript.parse_presets(content)
	context.assert_eq(presets.size(), 2, "only Windows Desktop presets returned")
	context.assert_eq(presets[0]["name"], "Windows Debug", "first preset name parsed")
	context.assert_eq(presets[0]["preset_index"], 0, "first preset index parsed")
	context.assert_eq(presets[1]["name"], "Windows Release", "second preset name parsed")


func _test_content_preparer_helpers(context) -> void:
	context.log_section("Packaging Content Preparation")

	var xml := '<Game>\n  <ExecutableList>\n    <Executable Name="Old.exe" Id="Game" />\n  </ExecutableList>\n</Game>\n'
	var patched = PackagingContentPreparerScript.patch_executable_name(xml, "NewGame.exe")
	context.assert_true(patched.contains('Executable Name="NewGame.exe"'), "executable name patched")

	var with_dependency = PackagingContentPreparerScript.inject_vc14_dependency("<Game>\n</Game>\n")
	context.assert_true(with_dependency.contains('<KnownDependency Name="VC14"/>'), "VC14 dependency injected")

	var unchanged = PackagingContentPreparerScript.inject_vc14_dependency(with_dependency)
	context.assert_eq(unchanged, with_dependency, "VC14 dependency not duplicated")


func _test_wdapp_output_parsing(context) -> void:
	context.log_section("wdapp Output Parsing")

	var sample_output := "Registered apps:\n"
	sample_output += "ContosoGame_1.0.0.0_x64__8wekyb3d8bbwe\n"
	sample_output += "    ContosoGame_1.0.0.0_x64__8wekyb3d8bbwe!Game\n"
	sample_output += "AnotherGame_1.0.0.0_x64__8wekyb3d8bbwe\n"
	sample_output += "    AnotherGame_1.0.0.0_x64__8wekyb3d8bbwe!App\n"

	var apps = WdappManagerScript.parse_registered_apps(sample_output)
	context.assert_eq(apps.size(), 2, "two registered apps parsed")
	context.assert_eq(apps[0]["pfn"], "ContosoGame_1.0.0.0_x64__8wekyb3d8bbwe", "first PFN parsed")
	context.assert_eq(apps[0]["aumid"], "ContosoGame_1.0.0.0_x64__8wekyb3d8bbwe!Game", "first AUMID parsed")


# ── XML Escaping ────────────────────────────────────────────────────────────

func _test_xml_escaping(context) -> void:
	context.log_section("XML Attribute Escaping")

	# Test the static escape function
	var escaped = GameConfigManagerScript._escape_xml_attr('Test & "Game" <1>')
	context.assert_eq(escaped, 'Test &amp; &quot;Game&quot; &lt;1&gt;', "special chars escaped")

	var plain = GameConfigManagerScript._escape_xml_attr("NormalText")
	context.assert_eq(plain, "NormalText", "plain text unchanged")

	var empty = GameConfigManagerScript._escape_xml_attr("")
	context.assert_eq(empty, "", "empty string unchanged")
