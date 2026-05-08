extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_system_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var system = gdk.get_system()
	assert_not_null(system, "GDK.system returns service object")
	if system == null:
		return

	for method_name in [
		"get_title_id",
		"get_title_id_hex",
		"get_sandbox_id",
		"get_service_configuration_id",
		"is_xbox_services_initialized",
	]:
		assert_has_method_named(system, method_name)

	var pre_init_scid = system.get_service_configuration_id()
	assert_not_null(pre_init_scid, "get_service_configuration_id() returns GDKResult before initialize")
	if pre_init_scid != null:
		assert_false(pre_init_scid.ok, "get_service_configuration_id() fails before initialize")
		assert_eq(pre_init_scid.code, "xbox_services_uninitialized", "get_service_configuration_id() reports xbox_services_uninitialized before initialize")

	var title_id_result = system.get_title_id()
	assert_not_null(title_id_result, "get_title_id() returns GDKResult")
	if title_id_result != null:
		if title_id_result.ok:
			assert_true(title_id_result.data is int, "get_title_id() success data is int")
		else:
			assert_true(title_id_result.code.length() > 0, "get_title_id() failure exposes an error code")
			assert_true(title_id_result.message.length() > 0, "get_title_id() failure exposes an error message")

	var title_id_hex_result = system.get_title_id_hex()
	assert_not_null(title_id_hex_result, "get_title_id_hex() returns GDKResult")
	if title_id_hex_result != null:
		if title_id_hex_result.ok:
			assert_true(title_id_hex_result.data is String, "get_title_id_hex() success data is String")
			if title_id_hex_result.data is String:
				assert_true(title_id_hex_result.data.begins_with("0x"), "get_title_id_hex() uses 0x prefix")
		else:
			assert_true(title_id_hex_result.code.length() > 0, "get_title_id_hex() failure exposes an error code")
			assert_true(title_id_hex_result.message.length() > 0, "get_title_id_hex() failure exposes an error message")

	var sandbox_result = system.get_sandbox_id()
	assert_not_null(sandbox_result, "get_sandbox_id() returns GDKResult")
	if sandbox_result != null:
		if sandbox_result.ok:
			assert_true(sandbox_result.data is String, "get_sandbox_id() success data is String")
		else:
			assert_true(sandbox_result.code.length() > 0, "get_sandbox_id() failure exposes an error code")
			assert_true(sandbox_result.message.length() > 0, "get_sandbox_id() failure exposes an error message")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for system behavior returns GDKResult")
	if init_result == null:
		return

	var post_init_scid = system.get_service_configuration_id()
	assert_not_null(post_init_scid, "get_service_configuration_id() returns GDKResult after initialize attempt")
	if post_init_scid == null:
		return

	if post_init_scid.ok:
		assert_true(post_init_scid.data is String, "get_service_configuration_id() success data is String")
		if post_init_scid.data is String:
			assert_true(post_init_scid.data.length() > 0, "get_service_configuration_id() success data is non-empty")
		assert_true(system.is_xbox_services_initialized(), "is_xbox_services_initialized() reflects successful services startup")
	else:
		assert_true(post_init_scid.code.length() > 0, "get_service_configuration_id() failure exposes an error code")
		assert_true(post_init_scid.message.length() > 0, "get_service_configuration_id() failure exposes an error message")
