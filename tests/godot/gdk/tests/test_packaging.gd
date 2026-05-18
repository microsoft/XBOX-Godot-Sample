extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 3 GUT migration of `suites/packaging_suite.gd`. Behavior parity:
## same per-call assertion count as the pre-GUT harness; `log_skip` mapped to
## `pending(...)`; `log_fail` early-returns on disk I/O errors preserved as
## `assert_true(false, ...)` so failures still fail the suite.

const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")
const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/core/gdk_toolchain.gd")
const MakePkgExecutorScript = preload("res://addons/godot_gdk_packaging/core/makepkg_executor.gd")
const PackagingSettingsStoreScript = preload("res://addons/godot_gdk_packaging/core/packaging_settings_store.gd")
const ExportPresetCatalogScript = preload("res://addons/godot_gdk_packaging/core/export_preset_catalog.gd")
const PackagingContentPreparerScript = preload("res://addons/godot_gdk_packaging/core/packaging_content_preparer.gd")
const WdappManagerScript = preload("res://addons/godot_gdk_packaging/core/wdapp_manager.gd")


func test_toolchain_detection() -> void:
	var toolchain = GDKToolchainScript.new()

	if toolchain.is_gdk_available():
		assert_true(toolchain.get_makepkg_path() != "", "makepkg path not empty")
		assert_true(toolchain.get_game_config_editor_path() != "", "GameConfigEditor path not empty")
		assert_true(toolchain.get_bin_dir() != "", "bin dir not empty")
		assert_true(
			FileAccess.file_exists(toolchain.get_makepkg_path()),
			"makepkg.exe exists at detected path")

		var version = toolchain.get_gdk_version()
		if version != "":
			assert_true(version.length() == 6, "GDK version is 6 digits: %s" % version)
			assert_true(version.is_valid_int(), "GDK version is numeric: %s" % version)
		else:
			pending("GDK version: GameDKCoreLatest env var not set")
	else:
		pending("toolchain paths: GDK not installed")


func test_config_parsing() -> void:
	var toolchain = GDKToolchainScript.new()
	var config_mgr = GameConfigManagerScript.new(toolchain)

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
			assert_true(false, "backup original config — cannot open existing MicrosoftGame.config")
			return
		original_config_contents = original_file.get_as_text()
		original_file.close()

	var f = FileAccess.open(config_path, FileAccess.WRITE)
	if f == null:
		assert_true(false, "write test config — cannot open MicrosoftGame.config")
		return
	f.store_string(test_xml)
	f.close()

	var result = config_mgr.parse_config()

	if had_original_config:
		var restore_file = FileAccess.open(config_path, FileAccess.WRITE)
		if restore_file == null:
			assert_true(false, "restore original config — cannot restore MicrosoftGame.config")
			return
		restore_file.store_string(original_config_contents)
		restore_file.close()
	else:
		var remove_err = DirAccess.remove_absolute(config_path)
		assert_eq(remove_err, OK, "remove temporary MicrosoftGame.config")

	assert_eq(result.get("config_version"), "1", "parsed configVersion")
	assert_eq(result.get("name"), "TestGame", "parsed Identity.Name")
	assert_eq(result.get("publisher"), "CN=TestPub", "parsed Identity.Publisher")
	assert_eq(result.get("version"), "2.0.0.0", "parsed Identity.Version")
	assert_eq(result.get("title_id"), "AABBCCDD", "parsed TitleId")
	assert_eq(result.get("executable"), "TestGame.exe", "parsed Executable.Name")
	assert_eq(result.get("display_name"), "Test Game", "parsed ShellVisuals.DefaultDisplayName")
	assert_eq(result.get("description"), "A test game", "parsed ShellVisuals.Description")
	assert_eq(result.get("background_color"), "#112233", "parsed ShellVisuals.BackgroundColor")
	assert_eq(result.get("product_id"), "test-product-id", "parsed MSStore.ProductId")


func test_config_template_creation() -> void:
	var toolchain = GDKToolchainScript.new()
	var config_mgr = GameConfigManagerScript.new(toolchain)

	var config_path = config_mgr.get_config_path()
	if FileAccess.file_exists(config_path):
		DirAccess.remove_absolute(config_path)

	var err = config_mgr.create_template("TestTitle", "CN=TestPub", "Test Title")
	assert_eq(err, OK, "create_template returns OK")
	assert_true(FileAccess.file_exists(config_path), "config file created on disk")

	var parser := XMLParser.new()
	err = parser.open(config_path)
	assert_eq(err, OK, "template is valid XML")

	err = config_mgr.create_template()
	assert_eq(err, ERR_ALREADY_EXISTS, "create_template rejects duplicate")

	DirAccess.remove_absolute(config_path)


func test_makepkg_argument_construction() -> void:
	var toolchain = GDKToolchainScript.new()
	var executor = MakePkgExecutorScript.new(toolchain)
	assert_not_null(executor, "MakePkgExecutor created")

	var pack_args = executor.build_pack_args("C:/src", "C:/src/layout.xml", "C:/pkg", {
		"content_id": "content-id",
		"product_id": "product-id",
		"encrypt_key": "keys.lekb",
		"updcompat": 2,
	})
	assert_eq(pack_args[0], "pack", "pack args start with command")
	assert_true(pack_args.has("/contentid"), "pack args include content id")
	assert_true(pack_args.has("/productid"), "pack args include product id")
	assert_true(pack_args.has("/lk"), "pack args include custom key switch")
	assert_true(pack_args.has("keys.lekb"), "pack args include custom key path")
	assert_true(pack_args.has("/updcompat"), "pack args include updcompat")

	var genmap_args = executor.build_genmap_args("C:/build", "C:/pkg/layout.xml")
	assert_eq(genmap_args, PackedStringArray([
		"genmap",
		"/f", "C:/pkg/layout.xml",
		"/d", "C:/build",
	]), "genmap args match expected layout")

	var validate_args = executor.build_validate_args("C:/build/layout.xml", "C:/build", "C:/pkg")
	assert_eq(validate_args, PackedStringArray([
		"validate",
		"/f", "C:/build/layout.xml",
		"/d", "C:/build",
		"/pd", "C:/pkg",
		"/pc",
	]), "validate args match expected layout")


func test_settings_persistence() -> void:
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
	assert_eq(err, OK, "save settings file")

	var loaded_state = store.load_state(test_path)
	assert_eq(loaded_state["packaging"]["source_dir"], "/test/source", "source_dir persisted")
	assert_eq(loaded_state["packaging"]["output_dir"], "/test/output", "output_dir persisted")
	assert_eq(loaded_state["sandbox"]["sandbox_id"], "XDKS.1", "sandbox_id persisted")
	assert_eq(loaded_state["sandbox"]["test_account"], "testuser@xboxtest.com", "test_account persisted")
	assert_eq(loaded_state["export"]["preset_name"], "Windows Debug", "preset_name persisted")
	assert_true(loaded_state["export"]["clean_build"], "clean_build persisted")

	DirAccess.remove_absolute(test_path)


func test_export_preset_parsing() -> void:
	var content := '[preset.0]\nname="Windows Debug"\nplatform="Windows Desktop"\n\n'
	content += '[preset.1]\nname="Android"\nplatform="Android"\n\n'
	content += '[preset.2]\nname="Windows Release"\nplatform="Windows Desktop"\n'

	var presets = ExportPresetCatalogScript.parse_presets(content)
	assert_eq(presets.size(), 2, "only Windows Desktop presets returned")
	assert_eq(presets[0]["name"], "Windows Debug", "first preset name parsed")
	assert_eq(presets[0]["preset_index"], 0, "first preset index parsed")
	assert_eq(presets[1]["name"], "Windows Release", "second preset name parsed")


func test_content_preparer_helpers() -> void:
	var xml := '<Game>\n  <ExecutableList>\n    <Executable Name="Old.exe" Id="Game" />\n  </ExecutableList>\n</Game>\n'
	var patched = PackagingContentPreparerScript.patch_executable_name(xml, "NewGame.exe")
	assert_true(patched.contains('Executable Name="NewGame.exe"'), "executable name patched")

	var with_dependency = PackagingContentPreparerScript.inject_vc14_dependency("<Game>\n</Game>\n")
	assert_true(with_dependency.contains('<KnownDependency Name="VC14"/>'), "VC14 dependency injected")

	var unchanged = PackagingContentPreparerScript.inject_vc14_dependency(with_dependency)
	assert_eq(unchanged, with_dependency, "VC14 dependency not duplicated")


func test_wdapp_output_parsing() -> void:
	var sample_output := "Registered apps:\n"
	sample_output += "ContosoGame_1.0.0.0_x64__8wekyb3d8bbwe\n"
	sample_output += "    ContosoGame_1.0.0.0_x64__8wekyb3d8bbwe!Game\n"
	sample_output += "AnotherGame_1.0.0.0_x64__8wekyb3d8bbwe\n"
	sample_output += "    AnotherGame_1.0.0.0_x64__8wekyb3d8bbwe!App\n"

	var apps = WdappManagerScript.parse_registered_apps(sample_output)
	assert_eq(apps.size(), 2, "two registered apps parsed")
	assert_eq(apps[0]["pfn"], "ContosoGame_1.0.0.0_x64__8wekyb3d8bbwe", "first PFN parsed")
	assert_eq(apps[0]["aumid"], "ContosoGame_1.0.0.0_x64__8wekyb3d8bbwe!Game", "first AUMID parsed")


func test_xml_escaping() -> void:
	var escaped = GameConfigManagerScript._escape_xml_attr('Test & "Game" <1>')
	assert_eq(escaped, 'Test &amp; &quot;Game&quot; &lt;1&gt;', "special chars escaped")

	var plain = GameConfigManagerScript._escape_xml_attr("NormalText")
	assert_eq(plain, "NormalText", "plain text unchanged")

	var empty = GameConfigManagerScript._escape_xml_attr("")
	assert_eq(empty, "", "empty string unchanged")
