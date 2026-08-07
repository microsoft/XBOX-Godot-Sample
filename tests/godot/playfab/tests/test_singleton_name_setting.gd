extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Coverage for the configurable engine singleton name
## (`playfab/runtime/singleton_name`).
##
## The C++ extension reads this setting once at load time, so the actual rename
## is proven end-to-end by `tests/bootstrap/run_singleton_name_override.gd`
## (which relaunches Godot behind an `override.cfg`). This suite pins the
## setting's registration contract and the GDScript-side resolver shared by the
## bootstrap autoload and this test base.

const SETTING_SINGLETON_NAME := "playfab/runtime/singleton_name"
const DEFAULT_SINGLETON_NAME := "PlayFab"
const BOOTSTRAP_SCRIPT_PATH := "res://addons/godot_playfab/runtime/playfab_bootstrap.gd"

var _original_setting_value: Variant = null


func before_each() -> void:
	_original_setting_value = ProjectSettings.get_setting(
			SETTING_SINGLETON_NAME, DEFAULT_SINGLETON_NAME)


func after_each() -> void:
	ProjectSettings.set_setting(SETTING_SINGLETON_NAME, _original_setting_value)


func test_setting_registered_with_default() -> void:
	assert_true(ProjectSettings.has_setting(SETTING_SINGLETON_NAME),
			"playfab/runtime/singleton_name project setting registered")
	assert_eq(String(get_setting_default(SETTING_SINGLETON_NAME)), DEFAULT_SINGLETON_NAME,
			"playfab/runtime/singleton_name defaults to 'PlayFab'")
	assert_eq(typeof(ProjectSettings.get_setting(SETTING_SINGLETON_NAME, "")), TYPE_STRING,
			"playfab/runtime/singleton_name is a String setting")


func test_singleton_registered_under_configured_name() -> void:
	var configured := playfab_singleton_name()
	assert_true(Engine.has_singleton(configured),
			"singleton registered under the configured name '%s'" % configured)
	assert_not_null(get_playfab(), "get_playfab() resolves the configured singleton")


func test_resolver_falls_back_for_unusable_values() -> void:
	for bad_value in ["", "   ", "9Leading", "has space", "has-dash", "dots.in.name"]:
		ProjectSettings.set_setting(SETTING_SINGLETON_NAME, bad_value)
		assert_eq(playfab_singleton_name(), DEFAULT_SINGLETON_NAME,
				"'%s' falls back to '%s'" % [bad_value, DEFAULT_SINGLETON_NAME])


func test_resolver_honours_valid_custom_name() -> void:
	ProjectSettings.set_setting(SETTING_SINGLETON_NAME, "  TitlePlayFab  ")
	assert_eq(playfab_singleton_name(), "TitlePlayFab", "valid custom name is trimmed and honoured")


func test_resolver_falls_back_to_default_singleton_when_rename_rejected() -> void:
	# The C++ side keeps the default registration when the configured name is
	# unusable at load time. GDScript consumers must still resolve the runtime,
	# so the lookup retries under the default name.
	ProjectSettings.set_setting(SETTING_SINGLETON_NAME, "NeverRegisteredName")
	assert_not_null(get_playfab(), "get_playfab() falls back to the default singleton registration")


func test_resolver_rejects_singleton_of_a_different_class() -> void:
	# `Input` is a valid identifier AND an already-registered engine singleton,
	# so the native side rejects it and keeps the default registration. The
	# GDScript resolver must not hand back the colliding `Input` singleton just
	# because `Engine.has_singleton()` succeeds for that name.
	ProjectSettings.set_setting(SETTING_SINGLETON_NAME, "Input")
	assert_eq(playfab_singleton_name(), "Input", "the colliding name is still what the setting resolves to")

	var resolved = get_playfab()
	assert_not_null(resolved, "resolver still returns a singleton under a colliding name")
	assert_ne(resolved, Engine.get_singleton("Input"),
			"resolver does not return the colliding 'Input' singleton")
	if resolved != null:
		assert_true(resolved.is_class(DEFAULT_SINGLETON_NAME),
				"resolver falls back to the real PlayFab singleton")


func test_bootstrap_resolver_agrees_with_test_base() -> void:
	var bootstrap_script: GDScript = load(BOOTSTRAP_SCRIPT_PATH)
	assert_not_null(bootstrap_script, "playfab_bootstrap.gd loads")
	if bootstrap_script == null:
		return

	var constants: Dictionary = bootstrap_script.get_script_constant_map()
	assert_eq(str(constants.get("SETTING_SINGLETON_NAME", "")), SETTING_SINGLETON_NAME,
			"bootstrap reads the same project setting")
	assert_eq(str(constants.get("DEFAULT_SINGLETON_NAME", "")), DEFAULT_SINGLETON_NAME,
			"bootstrap uses the same default")

	ProjectSettings.set_setting(SETTING_SINGLETON_NAME, "TitlePlayFab")
	assert_eq(str(bootstrap_script.get_singleton_name()), playfab_singleton_name(),
			"bootstrap resolver agrees with the test-base resolver")
