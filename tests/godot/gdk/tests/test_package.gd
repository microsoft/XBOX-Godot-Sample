extends "res://addons/godot_gdk_tests/gdk_test_base.gd"

const LIVE_DLC_PACKAGE_ID_ENV := "GDK_TEST_DLC_PACKAGE_ID"
const LIVE_DLC_PACK_PATH_ENV := "GDK_TEST_DLC_PACK_PATH"
const LIVE_DLC_EXPECTED_RESOURCE_ENV := "GDK_TEST_DLC_EXPECTED_RESOURCE"
const LIVE_DLC_LOOSE_PATH_ENV := "GDK_TEST_DLC_LOOSE_PATH"
const LIVE_MOUNT_TIMEOUT_MSEC := 30000


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_package_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var package_service = gdk.get_package()
	assert_not_null(package_service, "GDK.package returns service object")
	if package_service == null:
		return

	for method_name in [
		"enumerate_packages",
		"find_package_by_identifier",
		"get_current_process_package_identifier",
		"mount_package_async",
		"load_resource_pack_async",
		"get_loaded_resource_packs",
		"get_install_progress",
	]:
		assert_has_method_named(package_service, method_name)

	assert_eq(
		get_class_constant("GDKPackage", "PACKAGE_KIND_CONTENT"),
		1,
		"GDKPackage.PACKAGE_KIND_CONTENT constant is registered")
	assert_eq(
		get_class_constant("GDKPackage", "ENUMERATION_SCOPE_THIS_AND_RELATED"),
		1,
		"GDKPackage.ENUMERATION_SCOPE_THIS_AND_RELATED constant is registered")

	var loaded_packs: Array = package_service.get_loaded_resource_packs()
	assert_eq(loaded_packs.size(), 0, "get_loaded_resource_packs() starts empty")

	var mount = instantiate_class("GDKPackageMount")
	assert_object_is(mount, "GDKPackageMount", "GDKPackageMount can be instantiated for reflection")
	if mount != null:
		for method_name in ["get_package_identifier", "get_mount_path", "get_package_details", "is_valid", "resolve_path", "close"]:
			assert_has_method_named(mount, method_name)
		assert_false(mount.is_valid(), "new GDKPackageMount starts invalid")
		assert_result_ok(mount.close(), "GDKPackageMount.close() on an invalid mount")
		assert_result_error(
			mount.resolve_path("content/file.txt"),
			"package_mount_invalid",
			"GDKPackageMount.resolve_path() rejects invalid mounts")

	var resource_pack = instantiate_class("GDKPackageResourcePack")
	assert_object_is(resource_pack, "GDKPackageResourcePack", "GDKPackageResourcePack can be instantiated for reflection")
	if resource_pack != null:
		for method_name in ["get_package_identifier", "get_mount_path", "get_pack_relative_path", "get_pack_path", "get_package_details", "get_replace_files", "get_offset"]:
			assert_has_method_named(resource_pack, method_name)
		assert_eq(resource_pack.get_package_identifier(), "", "new GDKPackageResourcePack has no package identifier")
		assert_eq(resource_pack.get_pack_relative_path(), "", "new GDKPackageResourcePack has no resource-pack path")
		assert_false(resource_pack.get_replace_files(), "new GDKPackageResourcePack defaults replace_files false")
		assert_eq(resource_pack.get_offset(), 0, "new GDKPackageResourcePack defaults offset 0")

	for invalid_path in [
		" ",
		"C:\\temp\\dlc.pck",
		"res://dlc.pck",
		"user://dlc.pck",
		"/content/dlc.pck",
		"../dlc.pck",
		"content/../dlc.pck",
	]:
		await assert_signal_result_error(
			package_service.load_resource_pack_async("sample.package", invalid_path),
			"invalid_package_path",
			"load_resource_pack_async() rejects unsafe path '%s'" % invalid_path)

	await assert_signal_result_error(
		package_service.load_resource_pack_async("sample.package", "content/file.txt"),
		"invalid_resource_pack",
		"load_resource_pack_async() only accepts .pck/.zip packs")
	await assert_signal_result_error(
		package_service.load_resource_pack_async("sample.package", "content/dlc.pck", false, -1),
		"invalid_package_offset",
		"load_resource_pack_async() rejects negative offsets")


func test_package_runtime_metadata_and_missing_packages() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var package_service = gdk.get_package()
	assert_not_null(package_service, "GDK.package returns service object")
	if package_service == null:
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult for package tests")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Package service runtime behavior: %s" % init_result.message)
		return

	var packages_result = package_service.enumerate_packages()
	assert_result_ok(packages_result, "enumerate_packages()")
	if packages_result != null and packages_result.ok:
		assert_true(packages_result.data is Array, "enumerate_packages() returns Array payload")
	assert_result_error(
		package_service.enumerate_packages(99, get_class_constant("GDKPackage", "ENUMERATION_SCOPE_THIS_AND_RELATED")),
		"invalid_package_kind",
		"enumerate_packages() validates package kind")
	assert_result_error(
		package_service.enumerate_packages(get_class_constant("GDKPackage", "PACKAGE_KIND_CONTENT"), 99),
		"invalid_package_scope",
		"enumerate_packages() validates enumeration scope")

	assert_result_error(
		package_service.find_package_by_identifier(" "),
		"invalid_package_identifier",
		"find_package_by_identifier() rejects blank identifiers")
	assert_result_error(
		package_service.get_install_progress(" "),
		"invalid_package_identifier",
		"get_install_progress() rejects blank identifiers")
	var mount_result = await await_completion(package_service.mount_package_async(" "))
	assert_result_error(
		mount_result,
		"invalid_package_identifier",
		"mount_package_async() rejects blank identifiers")
	assert_result_error(
		gdk.get_last_error(),
		"invalid_package_identifier",
		"GDK.get_last_error() tracks mount_package_async() validation failures")

	var load_result = await await_completion(package_service.load_resource_pack_async(" ", "content/dlc.pck"))
	assert_result_error(
		load_result,
		"invalid_package_identifier",
		"load_resource_pack_async() rejects blank identifiers")
	assert_result_error(
		gdk.get_last_error(),
		"invalid_package_identifier",
		"GDK.get_last_error() tracks load_resource_pack_async() validation failures")

	const MISSING_ID := "gdk.tests.missing.id"
	assert_result_error(
		package_service.find_package_by_identifier(MISSING_ID),
		"package_not_found",
		"find_package_by_identifier() reports missing package IDs")
	assert_result_error(
		package_service.get_install_progress(MISSING_ID),
		"package_not_found",
		"get_install_progress() reports missing package IDs")
	await assert_signal_result_error(
		package_service.mount_package_async(MISSING_ID),
		"package_not_found",
		"mount_package_async() reports missing package IDs")
	await assert_signal_result_error(
		package_service.load_resource_pack_async(MISSING_ID, "content/dlc.pck"),
		"package_not_found",
		"load_resource_pack_async() reports missing package IDs")

	var identifier_result = package_service.get_current_process_package_identifier()
	assert_not_null(identifier_result, "get_current_process_package_identifier() returns GDKResult")
	if identifier_result != null:
		if identifier_result.ok:
			assert_true(identifier_result.data is String and String(identifier_result.data).length() > 0, "current process package identifier is non-empty when available")
		else:
			assert_eq(identifier_result.code, "package_identifier_unavailable", "unpackaged/current-process identifier failures use explicit error code")


func test_optional_live_dlc_resource_pack_and_loose_file() -> void:
	if not TestEnv.live_tests_enabled():
		pending("Set LIVE_TESTS=1 plus DLC package env vars to exercise live XPackage mounts.")
		return
	if pending_unless_runtime_available():
		return

	var package_id := OS.get_environment(LIVE_DLC_PACKAGE_ID_ENV).strip_edges()
	var pack_path := OS.get_environment(LIVE_DLC_PACK_PATH_ENV).strip_edges()
	if package_id.is_empty() or pack_path.is_empty():
		pending("Set %s and %s to exercise live XPackage resource-pack loading." % [LIVE_DLC_PACKAGE_ID_ENV, LIVE_DLC_PACK_PATH_ENV])
		return

	var gdk = get_gdk()
	var package_service = gdk.get_package()
	assert_not_null(package_service, "GDK.package returns service object for live DLC test")
	if package_service == null:
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult for live DLC test")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Live DLC XPackage behavior: %s" % init_result.message)
		return

	var load_result = await await_completion(
		package_service.load_resource_pack_async(package_id, pack_path),
		LIVE_MOUNT_TIMEOUT_MSEC)
	assert_result_ok(load_result, "load_resource_pack_async() live DLC")
	if load_result == null or not load_result.ok:
		return

	assert_true(load_result.data is Dictionary, "load_resource_pack_async() returns Dictionary payload")
	var load_data: Dictionary = load_result.data if load_result.data is Dictionary else {}
	assert_object_is(load_data.get("resource_pack"), "GDKPackageResourcePack", "load_resource_pack_async() returns resource-pack metadata")
	assert_false(bool(load_data.get("already_loaded", true)), "first live resource-pack load reports already_loaded false")

	var expected_resource := OS.get_environment(LIVE_DLC_EXPECTED_RESOURCE_ENV).strip_edges()
	if not expected_resource.is_empty():
		assert_true(FileAccess.file_exists(expected_resource), "loaded DLC resource is visible through res://")
		var expected_text := FileAccess.get_file_as_string(expected_resource)
		assert_true(expected_text.length() > 0, "loaded DLC resource can be read through FileAccess")

	var repeat_result = await await_completion(
		package_service.load_resource_pack_async(package_id, pack_path),
		LIVE_MOUNT_TIMEOUT_MSEC)
	assert_result_ok(repeat_result, "load_resource_pack_async() repeated live DLC")
	if repeat_result != null and repeat_result.ok:
		var repeat_data: Dictionary = repeat_result.data if repeat_result.data is Dictionary else {}
		assert_true(bool(repeat_data.get("already_loaded", false)), "repeated live resource-pack load uses service cache")
		assert_object_is(repeat_data.get("resource_pack"), "GDKPackageResourcePack", "repeated live load returns retained metadata")

	assert_true(package_service.get_loaded_resource_packs().size() >= 1, "GDK.package retains loaded resource-pack mounts")

	var loose_path := OS.get_environment(LIVE_DLC_LOOSE_PATH_ENV).strip_edges()
	if loose_path.is_empty():
		return

	var mount_result = await await_completion(
		package_service.mount_package_async(package_id),
		LIVE_MOUNT_TIMEOUT_MSEC)
	assert_result_ok(mount_result, "mount_package_async() live DLC")
	if mount_result == null or not mount_result.ok:
		return

	assert_object_is(mount_result.data, "GDKPackageMount", "mount_package_async() returns raw mount metadata")
	var mount = mount_result.data
	if mount == null:
		return

	var resolved_result = mount.resolve_path(loose_path)
	assert_result_ok(resolved_result, "GDKPackageMount.resolve_path() live loose file")
	if resolved_result != null and resolved_result.ok:
		var absolute_path := String(resolved_result.data)
		assert_true(FileAccess.file_exists(absolute_path), "live loose DLC file exists under the package mount")
		var loose_text := FileAccess.get_file_as_string(absolute_path)
		assert_true(loose_text.length() > 0, "live loose DLC file can be read through FileAccess")

	assert_result_ok(mount.close(), "GDKPackageMount.close() live DLC")
	assert_false(mount.is_valid(), "closed live GDKPackageMount becomes invalid")
