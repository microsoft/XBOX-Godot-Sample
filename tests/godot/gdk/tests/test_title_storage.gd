extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_title_storage_surface_and_validation() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var title_storage = gdk.get_title_storage()
	assert_not_null(title_storage, "GDK.title_storage returns service object")
	if title_storage == null:
		return

	for method_name in [
		"get_quota_async",
		"list_blob_metadata_async",
		"get_next_blob_metadata_async",
		"download_blob_async",
		"upload_blob_async",
		"delete_blob_async",
	]:
		assert_has_method_named(title_storage, method_name)

	var metadata = GDKTitleStorageBlobMetadata.new()
	assert_not_null(metadata, "GDKTitleStorageBlobMetadata can be instantiated")
	if metadata != null:
		assert_eq(metadata.blob_path, "", "empty metadata starts with no blob path")
		assert_eq(metadata.length, 0, "empty metadata starts with zero length")

	var metadata_result = GDKTitleStorageBlobMetadataResult.new()
	assert_not_null(metadata_result, "GDKTitleStorageBlobMetadataResult can be instantiated")
	if metadata_result != null:
		assert_eq(metadata_result.items.size(), 0, "empty metadata result starts with no items")
		assert_false(metadata_result.has_next, "empty metadata result has no next page")

	var pre_init_signal = title_storage.get_quota_async(null, "universal")
	await assert_signal_result_error(pre_init_signal, "runtime_unavailable", "get_quota_async() reports unavailable runtime before initialize")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for Title Storage returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Title Storage runtime validation: %s" % init_result.message)
		return

	var invalid_user_signal = title_storage.get_quota_async(null, "universal")
	await assert_signal_result_error(invalid_user_signal, "invalid_user", "get_quota_async() rejects null users after initialize")

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for Title Storage completes - timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed Title Storage sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed Title Storage sign-in exposes an error message")
			pending("Title Storage signed-in validation: %s" % sign_in_result.message)
		else:
			pending("Title Storage signed-in validation: No signed-in user is available on this machine.")
		return

	var invalid_storage_signal = title_storage.get_quota_async(user, "not_storage")
	await assert_signal_result_error(invalid_storage_signal, "invalid_storage_type", "get_quota_async() rejects unknown storage types")

	var invalid_skip_signal = title_storage.list_blob_metadata_async(user, "universal", "", -1, 25)
	await assert_signal_result_error(invalid_skip_signal, "invalid_skip_items", "list_blob_metadata_async() rejects negative skip_items")

	var invalid_next_signal = title_storage.get_next_blob_metadata_async(metadata_result)
	await assert_signal_result_error(invalid_next_signal, "invalid_metadata_result", "get_next_blob_metadata_async() rejects unmanaged results")

	var invalid_download_path = title_storage.download_blob_async(user, "universal", "")
	await assert_signal_result_error(invalid_download_path, "invalid_blob_path", "download_blob_async() rejects empty blob paths")

	var invalid_match_signal = title_storage.upload_blob_async(user, "universal", "path.bin", PackedByteArray(), "", "", "bad_condition")
	await assert_signal_result_error(invalid_match_signal, "invalid_match_condition", "upload_blob_async() rejects unknown match conditions")

	var missing_etag_signal = title_storage.upload_blob_async(user, "universal", "path.bin", PackedByteArray(), "", "", "if_match")
	await assert_signal_result_error(missing_etag_signal, "invalid_e_tag", "upload_blob_async() requires e_tag for if_match")

	var invalid_delete_condition = title_storage.delete_blob_async(user, "universal", "path.bin", "", "if_not_match")
	await assert_signal_result_error(invalid_delete_condition, "invalid_match_condition", "delete_blob_async() rejects if_not_match")
