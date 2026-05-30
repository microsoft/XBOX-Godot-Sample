extends GutTest

const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")

const _FIXTURE_DIR := "res://tests/fixtures/packaging"

class _FakeToolchain:
	extends RefCounted

	func get_bin_dir() -> String:
		return ProjectSettings.globalize_path("user://missing_gdk_bin")


func test_parse_config_consumes_valid_full_fixture() -> void:
	var info: Dictionary = _new_manager().parse_config(_fixture_path("valid_full.config"))

	assert_eq(info.get("config_version", ""), "1")
	assert_eq(info.get("name", ""), "FixtureGame")
	assert_eq(info.get("publisher", ""), "CN=FixturePub")
	assert_eq(info.get("version", ""), "1.0.0.0")
	assert_eq(info.get("title_id", ""), "1234ABCD")
	assert_eq(info.get("msa_app_id", ""), "00000000FEEDFACE")
	assert_eq(info.get("store_id", ""), "9FIXTURESTOREID")
	assert_eq(info.get("product_id", ""), "fixture-product-id")
	assert_eq(info.get("executable", ""), "FixtureGame.exe")
	assert_eq(info.get("display_name", ""), "Fixture Game")
	assert_eq(info.get("description", ""), "Valid full config fixture")
	assert_eq(info.get("background_color", ""), "#102030")
	assert_eq(info.get("foreground_text", ""), "light")
	assert_eq(info.get("store_logo", ""), "storelogos\\StoreLogo.png")
	assert_eq(info.get("logo_150", ""), "storelogos\\Square150x150Logo.png")
	assert_eq(info.get("logo_44", ""), "storelogos\\Square44x44Logo.png")
	assert_eq(info.get("logo_480", ""), "storelogos\\Square480x480Logo.png")
	assert_eq(info.get("splash_screen", ""), "storelogos\\SplashScreenImage.png")


func test_parse_config_returns_partial_shape_when_identity_is_missing() -> void:
	var info: Dictionary = _new_manager().parse_config(_fixture_path("missing_identity.config"))

	assert_eq(info.get("config_version", ""), "1")
	assert_eq(info.get("name", ""), "", "missing Identity leaves name empty")
	assert_eq(info.get("publisher", ""), "", "missing Identity leaves publisher empty")
	assert_eq(info.get("version", ""), "", "missing Identity leaves version empty")
	assert_eq(info.get("title_id", ""), "22223333")
	assert_eq(info.get("executable", ""), "MissingIdentity.exe")
	assert_eq(info.get("display_name", ""), "Missing Identity Fixture")
	assert_eq(info.get("product_id", ""), "", "missing MSStore leaves product_id empty")


func test_parse_config_handles_missing_resources_without_dropping_identity() -> void:
	var info: Dictionary = _new_manager().parse_config(_fixture_path("missing_resources.config"))

	assert_eq(info.get("name", ""), "NoResources")
	assert_eq(info.get("publisher", ""), "CN=NoRes")
	assert_eq(info.get("title_id", ""), "44445555")
	assert_eq(info.get("executable", ""), "NoResources.exe")
	assert_eq(info.get("display_name", ""), "No Resources Fixture")
	assert_eq(info.get("store_logo", ""), "", "logo fields default empty when ShellVisuals omits them")
	assert_eq(info.get("logo_150", ""), "", "resource block is optional for parse_config")


func test_parse_config_missing_file_returns_empty_dictionary() -> void:
	var info: Dictionary = _new_manager().parse_config(ProjectSettings.globalize_path("user://missing/MicrosoftGame.config"))

	assert_true(info.is_empty(), "missing config path returns {} rather than a partially-filled shape")


func test_to_filesystem_path_documents_headless_runner_path_rules() -> void:
	var res_path := "res://Configs/Alt.config"
	var user_path := "user://configs/Alt.config"
	var relative_path := "Configs/Relative.config"
	var absolute_path: String = ProjectSettings.globalize_path("user://absolute/Already.config")

	assert_eq(GameConfigManagerScript.to_filesystem_path(""), "", "empty path passes through")
	assert_eq(GameConfigManagerScript.to_filesystem_path(res_path), ProjectSettings.globalize_path(res_path), "res:// paths globalize")
	assert_eq(GameConfigManagerScript.to_filesystem_path(user_path), ProjectSettings.globalize_path(user_path), "user:// paths globalize")
	assert_eq(GameConfigManagerScript.to_filesystem_path(absolute_path), absolute_path, "filesystem-absolute paths pass through")
	assert_eq(GameConfigManagerScript.to_filesystem_path(relative_path), ProjectSettings.globalize_path("res://").path_join(relative_path), "relative paths resolve from project root")


func _new_manager() -> RefCounted:
	return GameConfigManagerScript.new(_FakeToolchain.new())


func _fixture_path(file_name: String) -> String:
	return ProjectSettings.globalize_path(_FIXTURE_DIR.path_join(file_name))
