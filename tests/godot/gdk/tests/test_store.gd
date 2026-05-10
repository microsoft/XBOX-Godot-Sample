extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_store_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var store = gdk.get_store()
	assert_not_null(store, "GDK.store returns service object")
	if store == null:
		return

	for method_name in [
		"query_license_status_async",
		"refresh_entitlements_async",
		"show_purchase_ui_async",
		"get_cached_license_status",
		"check_cached_license_status",
	]:
		assert_has_method_named(store, method_name)

	var blank_license_status = instantiate_class("GDKStoreLicenseStatus")
	assert_not_null(blank_license_status, "GDKStoreLicenseStatus.new() returns wrapper")
	if blank_license_status != null:
		for method_name in ["get_store_id", "get_licensable_sku", "get_status"]:
			assert_has_method_named(blank_license_status, method_name)
		assert_eq(blank_license_status.get_store_id(), "", "blank GDKStoreLicenseStatus store_id defaults empty")
		assert_eq(blank_license_status.get_licensable_sku(), "", "blank GDKStoreLicenseStatus licensable_sku defaults empty")
		assert_eq(blank_license_status.get_status(), 0, "blank GDKStoreLicenseStatus status defaults 0")

	var blank_user = instantiate_class("GDKUser")

	var pre_init_query = store.query_license_status_async(blank_user, "9NBLGGH4R315")
	await assert_signal_result_error(pre_init_query, "not_initialized", "query_license_status_async() rejects requests before initialize()")

	var pre_init_refresh = store.refresh_entitlements_async(blank_user, "9NBLGGH4R315")
	await assert_signal_result_error(pre_init_refresh, "not_initialized", "refresh_entitlements_async() rejects requests before initialize()")

	var pre_init_purchase = store.show_purchase_ui_async(blank_user, "9NBLGGH4R315")
	await assert_signal_result_error(pre_init_purchase, "not_initialized", "show_purchase_ui_async() rejects requests before initialize()")

	var invalid_cached_check = store.check_cached_license_status("")
	assert_not_null(invalid_cached_check, "check_cached_license_status('') returns GDKResult")
	if invalid_cached_check != null:
		assert_false(invalid_cached_check.ok, "check_cached_license_status('') fails")
		assert_eq(invalid_cached_check.code, "invalid_product_id", "check_cached_license_status('') reports invalid_product_id")

	assert_true(store.get_cached_license_status("9NBLGGH4R315") == null, "get_cached_license_status() returns null when no cache entry exists")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for store behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Store runtime behavior: %s" % init_result.message)
		return

	var invalid_store_id_query = store.query_license_status_async(blank_user, " ")
	await assert_signal_result_error(invalid_store_id_query, "invalid_product_id", "query_license_status_async() rejects blank Store product IDs")

	var invalid_store_id_refresh = store.refresh_entitlements_async(blank_user, "")
	await assert_signal_result_error(invalid_store_id_refresh, "invalid_product_id", "refresh_entitlements_async() rejects blank Store product IDs")

	var invalid_store_id_purchase = store.show_purchase_ui_async(blank_user, " ")
	await assert_signal_result_error(invalid_store_id_purchase, "invalid_product_id", "show_purchase_ui_async() rejects blank Store product IDs")

	var missing_user_query = store.query_license_status_async(null, "9NBLGGH4R315")
	await assert_signal_result_error(missing_user_query, "invalid_user", "query_license_status_async() requires a signed-in user")

	var missing_user_refresh = store.refresh_entitlements_async(null, "9NBLGGH4R315")
	await assert_signal_result_error(missing_user_refresh, "invalid_user", "refresh_entitlements_async() requires a signed-in user")

	var missing_user_purchase = store.show_purchase_ui_async(null, "9NBLGGH4R315")
	await assert_signal_result_error(missing_user_purchase, "invalid_user", "show_purchase_ui_async() requires a signed-in user")

	var uncached_check = store.check_cached_license_status("9NBLGGH4R315")
	assert_not_null(uncached_check, "check_cached_license_status() returns GDKResult for uncached values")
	if uncached_check != null:
		assert_false(uncached_check.ok, "check_cached_license_status() fails when nothing is cached")
		assert_eq(uncached_check.code, "license_status_not_cached", "uncached checks report license_status_not_cached")
