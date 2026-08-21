extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Surface + validation coverage for `XboxNetworking` (`GDK.networking`).
## Wraps `XNetworking.h`: preferred local UDP multiplayer port, connectivity
## hints, NSAL security information, and TCP buffer configuration/statistics.

const HINT_KEYS := [
	"connectivity_level",
	"connectivity_level_name",
	"connectivity_cost",
	"connectivity_cost_name",
	"iana_interface_type",
	"network_initialized",
	"approaching_data_limit",
	"over_data_limit",
	"roaming",
]

const STATISTICS_KEYS := [
	"num_bytes_currently_queued",
	"peak_num_bytes_ever_queued",
	"total_num_bytes_queued",
	"num_bytes_dropped_for_exceeding_configured_max",
	"num_bytes_dropped_due_to_any_failure",
]


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func _get_networking():
	var gdk = get_gdk()
	if gdk == null:
		return null
	return gdk.get_networking()


func test_networking_surface_and_constants() -> void:
	if pending_unless_runtime_available():
		return

	var networking = _get_networking()
	assert_not_null(networking, "GDK.networking returns service object")
	if networking == null:
		return

	for method_name in [
		"query_preferred_local_udp_multiplayer_port",
		"query_preferred_local_udp_multiplayer_port_async",
		"get_connectivity_hint",
		"query_security_information_for_url_async",
		"query_configuration_setting",
		"set_configuration_setting",
		"query_statistics",
	]:
		assert_has_method_named(networking, method_name)

	for signal_name in [
		"preferred_local_udp_multiplayer_port_changed",
		"connectivity_hint_changed",
	]:
		assert_has_signal_named(networking, signal_name)

	for constant_name in [
		"CONNECTIVITY_LEVEL_UNKNOWN",
		"CONNECTIVITY_LEVEL_NONE",
		"CONNECTIVITY_LEVEL_LOCAL_ACCESS",
		"CONNECTIVITY_LEVEL_INTERNET_ACCESS",
		"CONNECTIVITY_LEVEL_CONSTRAINED_INTERNET_ACCESS",
		"CONNECTIVITY_COST_UNKNOWN",
		"CONNECTIVITY_COST_UNRESTRICTED",
		"CONNECTIVITY_COST_FIXED",
		"CONNECTIVITY_COST_VARIABLE",
		"THUMBPRINT_TYPE_LEAF",
		"THUMBPRINT_TYPE_ISSUER",
		"THUMBPRINT_TYPE_ROOT",
		"CONFIGURATION_SETTING_MAX_TITLE_TCP_QUEUED_RECEIVE_BUFFER_SIZE",
		"CONFIGURATION_SETTING_MAX_SYSTEM_TCP_QUEUED_RECEIVE_BUFFER_SIZE",
		"CONFIGURATION_SETTING_MAX_TOOLS_TCP_QUEUED_RECEIVE_BUFFER_SIZE",
		"STATISTICS_TYPE_TITLE_TCP_QUEUED_RECEIVED_BUFFER_USAGE",
		"STATISTICS_TYPE_SYSTEM_TCP_QUEUED_RECEIVED_BUFFER_USAGE",
		"STATISTICS_TYPE_TOOLS_TCP_QUEUED_RECEIVED_BUFFER_USAGE",
	]:
		assert_true(
				ClassDB.class_has_integer_constant("XboxNetworking", constant_name),
				"XboxNetworking exposes %s" % constant_name)

	assert_true(
			ClassDB.class_exists("XboxNetworkingSecurityInformation"),
			"XboxNetworkingSecurityInformation is registered")


func test_networking_rejects_calls_before_initialize() -> void:
	if pending_unless_runtime_available():
		return

	var networking = _get_networking()
	if networking == null:
		return

	assert_result_error(
			networking.query_preferred_local_udp_multiplayer_port(),
			"not_initialized",
			"query_preferred_local_udp_multiplayer_port() rejects calls before GDK.initialize()")
	assert_result_error(
			networking.get_connectivity_hint(),
			"not_initialized",
			"get_connectivity_hint() rejects calls before GDK.initialize()")
	assert_result_error(
			networking.query_configuration_setting(
					get_class_constant("XboxNetworking", "CONFIGURATION_SETTING_MAX_TITLE_TCP_QUEUED_RECEIVE_BUFFER_SIZE")),
			"not_initialized",
			"query_configuration_setting() rejects calls before GDK.initialize()")
	assert_result_error(
			networking.set_configuration_setting(
					get_class_constant("XboxNetworking", "CONFIGURATION_SETTING_MAX_TITLE_TCP_QUEUED_RECEIVE_BUFFER_SIZE"),
					1024),
			"not_initialized",
			"set_configuration_setting() rejects calls before GDK.initialize()")
	assert_result_error(
			networking.query_statistics(
					get_class_constant("XboxNetworking", "STATISTICS_TYPE_TITLE_TCP_QUEUED_RECEIVED_BUFFER_USAGE")),
			"not_initialized",
			"query_statistics() rejects calls before GDK.initialize()")

	await assert_signal_result_error(
			networking.query_preferred_local_udp_multiplayer_port_async(),
			"not_initialized",
			"query_preferred_local_udp_multiplayer_port_async() rejects calls before GDK.initialize()")
	await assert_signal_result_error(
			networking.query_security_information_for_url_async("https://example.com"),
			"not_initialized",
			"query_security_information_for_url_async() rejects calls before GDK.initialize()")


func test_networking_validates_arguments_after_initialize() -> void:
	if pending_unless_runtime_available():
		return

	var networking = _get_networking()
	if networking == null:
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for networking validation returns XboxResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Networking runtime behavior: %s" % init_result.message)
		return

	assert_result_error(
			networking.query_configuration_setting(999),
			"invalid_configuration_setting",
			"query_configuration_setting() rejects unknown settings")
	assert_result_error(
			networking.set_configuration_setting(999, 1024),
			"invalid_configuration_setting",
			"set_configuration_setting() rejects unknown settings")
	assert_result_error(
			networking.set_configuration_setting(
					get_class_constant("XboxNetworking", "CONFIGURATION_SETTING_MAX_TITLE_TCP_QUEUED_RECEIVE_BUFFER_SIZE"),
					-1),
			"invalid_configuration_value",
			"set_configuration_setting() rejects negative values")
	assert_result_error(
			networking.query_statistics(999),
			"invalid_statistics_type",
			"query_statistics() rejects unknown statistics types")
	await assert_signal_result_error(
			networking.query_security_information_for_url_async("   "),
			"invalid_url",
			"query_security_information_for_url_async() rejects blank URLs")


func test_connectivity_hint_payload_shape() -> void:
	if pending_unless_runtime_available():
		return

	var networking = _get_networking()
	if networking == null:
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Connectivity hint requires an initialized runtime")
		return

	var hint_result = networking.get_connectivity_hint()
	assert_not_null(hint_result, "get_connectivity_hint() returns XboxResult after init")
	if hint_result == null:
		return
	if not hint_result.ok:
		assert_true(hint_result.code.length() > 0, "failed get_connectivity_hint exposes an error code")
		return

	assert_true(hint_result.data is Dictionary, "get_connectivity_hint() success data is Dictionary")
	if not (hint_result.data is Dictionary):
		return

	var hint: Dictionary = hint_result.data
	for key in HINT_KEYS:
		assert_dict_has_key(hint, key, "connectivity hint has %s" % key)

	var valid_levels := [
		get_class_constant("XboxNetworking", "CONNECTIVITY_LEVEL_UNKNOWN"),
		get_class_constant("XboxNetworking", "CONNECTIVITY_LEVEL_NONE"),
		get_class_constant("XboxNetworking", "CONNECTIVITY_LEVEL_LOCAL_ACCESS"),
		get_class_constant("XboxNetworking", "CONNECTIVITY_LEVEL_INTERNET_ACCESS"),
		get_class_constant("XboxNetworking", "CONNECTIVITY_LEVEL_CONSTRAINED_INTERNET_ACCESS"),
	]
	assert_true(
			int(hint.get("connectivity_level", -1)) in valid_levels,
			"connectivity_level is one of CONNECTIVITY_LEVEL_*")

	var valid_costs := [
		get_class_constant("XboxNetworking", "CONNECTIVITY_COST_UNKNOWN"),
		get_class_constant("XboxNetworking", "CONNECTIVITY_COST_UNRESTRICTED"),
		get_class_constant("XboxNetworking", "CONNECTIVITY_COST_FIXED"),
		get_class_constant("XboxNetworking", "CONNECTIVITY_COST_VARIABLE"),
	]
	assert_true(
			int(hint.get("connectivity_cost", -1)) in valid_costs,
			"connectivity_cost is one of CONNECTIVITY_COST_*")

	assert_true(hint.get("network_initialized") is bool, "network_initialized is bool")
	assert_true(String(hint.get("connectivity_level_name", "")).length() > 0, "connectivity_level_name is populated")
	assert_true(String(hint.get("connectivity_cost_name", "")).length() > 0, "connectivity_cost_name is populated")


func test_preferred_local_udp_multiplayer_port_async() -> void:
	if pending_unless_runtime_available():
		return

	var networking = _get_networking()
	if networking == null:
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Preferred port query requires an initialized runtime")
		return

	var result = await await_completion(networking.query_preferred_local_udp_multiplayer_port_async())
	assert_not_null(result, "query_preferred_local_udp_multiplayer_port_async() completes")
	if result == null:
		return

	if not result.ok:
		assert_true(result.code.length() > 0, "failed preferred port query exposes an error code")
		assert_true(result.message.length() > 0, "failed preferred port query exposes an error message")
		return

	assert_true(result.data is Dictionary, "preferred port success data is Dictionary")
	if result.data is Dictionary:
		assert_dict_has_key(result.data, "port", "preferred port data has port")
		var port: int = int(result.data.get("port", -1))
		assert_true(port >= 0 and port <= 65535, "preferred port is a valid UDP port number")


func test_configuration_and_statistics_report_platform_behavior() -> void:
	if pending_unless_runtime_available():
		return

	var networking = _get_networking()
	if networking == null:
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Configuration/statistics queries require an initialized runtime")
		return

	var setting: int = get_class_constant("XboxNetworking", "CONFIGURATION_SETTING_MAX_TITLE_TCP_QUEUED_RECEIVE_BUFFER_SIZE")

	var query_result = networking.query_configuration_setting(setting)
	assert_not_null(query_result, "query_configuration_setting() returns XboxResult after init")
	if query_result != null and query_result.ok:
		assert_true(query_result.data is Dictionary, "query_configuration_setting() success data is Dictionary")
		if query_result.data is Dictionary:
			assert_dict_has_key(query_result.data, "value", "configuration data has value")
			assert_dict_has_key(query_result.data, "unlimited", "configuration data has unlimited")
			assert_true(query_result.data.get("unlimited") is bool, "unlimited is bool")

	# Documented as E_NOTIMPL on Windows; a console-capable host may accept it.
	var set_result = networking.set_configuration_setting(setting, 1024 * 1024)
	assert_not_null(set_result, "set_configuration_setting() returns XboxResult after init")
	if set_result != null and not set_result.ok:
		assert_true(
				set_result.code in ["not_supported_on_platform", "configuration_setting_set_failed"],
				"set_configuration_setting() failure uses a documented code (got %s)" % set_result.code)

	var statistics_result = networking.query_statistics(
			get_class_constant("XboxNetworking", "STATISTICS_TYPE_TITLE_TCP_QUEUED_RECEIVED_BUFFER_USAGE"))
	assert_not_null(statistics_result, "query_statistics() returns XboxResult after init")
	if statistics_result != null and statistics_result.ok:
		assert_true(statistics_result.data is Dictionary, "query_statistics() success data is Dictionary")
		if statistics_result.data is Dictionary:
			for key in STATISTICS_KEYS:
				assert_dict_has_key(statistics_result.data, key, "statistics data has %s" % key)
