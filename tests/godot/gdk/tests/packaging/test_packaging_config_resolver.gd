extends GutTest
## GUT coverage for `core/packaging_config.gd` — pins the precedence chain
## (CLI > .gdk_packaging.cfg > MicrosoftGame.config > project.godot >
## built-in defaults) and the kebab-case → snake_case key remap.

const PackagingConfig = preload("res://addons/godot_gdk_packaging/core/packaging_config.gd")
const PackagingSettingsStoreScript = preload("res://addons/godot_gdk_packaging/core/packaging_settings_store.gd")

const _FIXTURE_DIR := "user://test_packaging_config_resolver"
const _CONFIG_XML := "MicrosoftGame.config"
const _SETTINGS_FILE := ".gdk_packaging.cfg"


func before_each() -> void:
	DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(_FIXTURE_DIR))


func after_each() -> void:
	# Tidy up so a fresh fixture is written each test.
	var root: String = ProjectSettings.globalize_path(_FIXTURE_DIR)
	if not DirAccess.dir_exists_absolute(root):
		return
	for filename: String in DirAccess.get_files_at(root):
		var path: String = root.path_join(filename)
		if FileAccess.file_exists(path):
			DirAccess.remove_absolute(path)


func _project_dir() -> String:
	return ProjectSettings.globalize_path(_FIXTURE_DIR)


func _write_config_xml(extra_attrs: Dictionary = {}, filename: String = _CONFIG_XML) -> String:
	# Minimal MicrosoftGame.config the resolver knows how to read.
	var product_id: String = extra_attrs.get("product_id", "PROD12345")
	var ident_name: String = extra_attrs.get("name", "MyGame")
	var ident_pub: String = extra_attrs.get("publisher", "CN=Acme")
	var ident_ver: String = extra_attrs.get("version", "1.0.0.0")
	var executable: String = extra_attrs.get("executable", "MyGame.exe")
	var xml: String = ""
	xml += '<?xml version="1.0" encoding="utf-8"?>\n'
	xml += '<Game configVersion="1">\n'
	xml += '  <Identity Name="%s" Publisher="%s" Version="%s" />\n' % [
		ident_name, ident_pub, ident_ver,
	]
	xml += '  <ExecutableList>\n'
	xml += '    <Executable Name="%s" Id="Game" />\n' % executable
	xml += '  </ExecutableList>\n'
	xml += '  <MSStore ProductId="%s" />\n' % product_id
	xml += '</Game>\n'
	var path: String = _project_dir().path_join(filename)
	var f: FileAccess = FileAccess.open(path, FileAccess.WRITE)
	f.store_string(xml)
	f.close()
	return path


func _write_settings_cfg(overrides: Dictionary) -> String:
	var store: RefCounted = PackagingSettingsStoreScript.new()
	var state: Dictionary = store.get_default_state()
	for section: String in overrides:
		for key: String in overrides[section]:
			state[section][key] = overrides[section][key]
	var path: String = _project_dir().path_join(_SETTINGS_FILE)
	store.save_state(path, state)
	return path


# ── Defaults ───────────────────────────────────────────────────────────────

func test_defaults_apply_when_nothing_is_supplied() -> void:
	var resolved: Dictionary = PackagingConfig.resolve({}, _project_dir(), "", "")
	assert_eq(resolved["source_dir"], "", "default source_dir is empty string")
	assert_eq(resolved["updcompat"], 3, "default updcompat is 3")
	assert_eq(resolved["encrypt"], "none", "default encrypt is 'none'")
	assert_eq(resolved["action"], "get", "default sandbox action is 'get'")
	assert_eq(resolved["release"], false, "default release flag is false")


# ── CLI > defaults ─────────────────────────────────────────────────────────

func test_cli_overrides_default() -> void:
	var resolved: Dictionary = PackagingConfig.resolve(
		{"source-dir": "Build", "output-dir": "Out", "updcompat": 1},
		_project_dir(), "", "")
	assert_eq(resolved["source_dir"], "Build",
		"CLI kebab key remapped to snake_case")
	assert_eq(resolved["output_dir"], "Out", "output_dir captured")
	assert_eq(resolved["updcompat"], 1, "CLI updcompat overrides default")


# ── MicrosoftGame.config layer ─────────────────────────────────────────────

func test_microsoftgame_config_supplies_identity_fields() -> void:
	var config_path: String = _write_config_xml({"product_id": "PROD777"})
	var resolved: Dictionary = PackagingConfig.resolve(
		{}, _project_dir(), "", config_path)
	assert_true(resolved["config_exists"], "config_exists flag set")
	assert_eq(resolved["product_id"], "PROD777", "product_id pulled from config")
	assert_eq(resolved["identity_name"], "MyGame", "identity name pulled")
	assert_eq(resolved["identity_publisher"], "CN=Acme", "publisher pulled")
	assert_eq(resolved["executable"], "MyGame.exe", "executable pulled")


func test_cli_config_flag_redirects_config_parsing() -> void:
	_write_config_xml({"product_id": "DEFAULT", "name": "DefaultGame"})
	var override_path: String = _write_config_xml({
		"product_id": "OVERRIDE",
		"name": "OverrideGame",
		"executable": "Override.exe",
	}, "AltGame.config")
	var resolved: Dictionary = PackagingConfig.resolve(
		{"config": override_path}, _project_dir(), "", "")
	assert_eq(resolved["config_path"], override_path, "--config becomes config_path")
	assert_eq(resolved["product_id"], "OVERRIDE", "--config supplies product_id")
	assert_eq(resolved["identity_name"], "OverrideGame", "--config supplies identity")
	assert_eq(resolved["executable"], "Override.exe", "--config supplies executable")
	assert_eq(resolved["raw_config_info"].get("product_id", ""), "OVERRIDE",
		"raw_config_info comes from --config")


func test_content_id_defaults_to_product_id_when_unset() -> void:
	var config_path: String = _write_config_xml({"product_id": "PROD777"})
	var resolved: Dictionary = PackagingConfig.resolve(
		{}, _project_dir(), "", config_path)
	assert_eq(resolved["content_id"], "PROD777",
		"content_id falls back to product_id")


func test_cli_content_id_beats_product_id_fallback() -> void:
	var config_path: String = _write_config_xml({"product_id": "PROD777"})
	var resolved: Dictionary = PackagingConfig.resolve(
		{"content-id": "OVERRIDE"}, _project_dir(), "", config_path)
	assert_eq(resolved["content_id"], "OVERRIDE",
		"CLI content_id wins over product_id fallback")


# ── .gdk_packaging.cfg layer ───────────────────────────────────────────────

func test_settings_file_supplies_packaging_fields() -> void:
	var settings_path: String = _write_settings_cfg({
		"packaging": {
			"source_dir": "FromSettings",
			"updcompat_option": 2,  # → makepkg /updcompat 1
			"encrypt_option": 1,    # → license
		},
	})
	var resolved: Dictionary = PackagingConfig.resolve(
		{}, _project_dir(), settings_path, "")
	assert_eq(resolved["source_dir"], "FromSettings",
		"source_dir from settings file")
	assert_eq(resolved["updcompat"], 1,
		"updcompat option translated through")
	assert_eq(resolved["encrypt"], "license",
		"encrypt option translated through")


func test_cli_beats_settings_file() -> void:
	var settings_path: String = _write_settings_cfg({
		"packaging": {"source_dir": "FromSettings"},
	})
	var resolved: Dictionary = PackagingConfig.resolve(
		{"source-dir": "FromCli"}, _project_dir(), settings_path, "")
	assert_eq(resolved["source_dir"], "FromCli",
		"CLI source_dir wins over settings file")


func test_empty_string_in_settings_does_not_clobber_config_value() -> void:
	var config_path: String = _write_config_xml({"product_id": "FROM_CONFIG"})
	var settings_path: String = _write_settings_cfg({
		"packaging": {"product_id": ""},  # default empty in settings file
	})
	var resolved: Dictionary = PackagingConfig.resolve(
		{}, _project_dir(), settings_path, config_path)
	assert_eq(resolved["product_id"], "FROM_CONFIG",
		"empty settings string doesn't blow away config value")


# ── Encrypt key:<path> normalisation ───────────────────────────────────────

func test_cli_encrypt_key_payload_is_split() -> void:
	var resolved: Dictionary = PackagingConfig.resolve(
		{"encrypt": "key:secrets/license.ekb"}, _project_dir(), "", "")
	assert_eq(resolved["encrypt"], "key",
		"encrypt collapses to bare 'key'")
	assert_eq(resolved["encrypt_key"], "secrets/license.ekb",
		"encrypt_key extracted from payload")


func test_explicit_encrypt_key_is_preserved_when_already_set() -> void:
	var resolved: Dictionary = PackagingConfig.resolve(
		{"encrypt": "key:override.ekb", "encrypt-key": "explicit.ekb"},
		_project_dir(), "", "")
	assert_eq(resolved["encrypt"], "key", "encrypt collapsed")
	assert_eq(resolved["encrypt_key"], "explicit.ekb",
		"explicit --encrypt-key wins over key:<payload> split")


# ── Project root + raw_* mirrors ───────────────────────────────────────────

func test_raw_settings_and_raw_config_are_populated() -> void:
	var config_path: String = _write_config_xml({})
	var settings_path: String = _write_settings_cfg({
		"sandbox": {"sandbox_id": "XDKS.1"},
	})
	var resolved: Dictionary = PackagingConfig.resolve(
		{}, _project_dir(), settings_path, config_path)
	assert_true(resolved.has("raw_settings"), "raw_settings present")
	assert_eq(resolved["raw_settings"]["sandbox"]["sandbox_id"], "XDKS.1",
		"raw_settings carries the full state for dock-shaped consumers")
	assert_true(resolved.has("raw_config_info"), "raw_config_info present")
	assert_eq(resolved["raw_config_info"]["product_id"], "PROD12345",
		"raw_config_info carries the full parsed config")
	assert_eq(resolved["project_dir"], _project_dir(),
		"project_dir reflects override")
