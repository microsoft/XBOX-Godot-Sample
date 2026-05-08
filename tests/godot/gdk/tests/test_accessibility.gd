extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_accessibility_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var accessibility = gdk.get_accessibility()
	assert_not_null(accessibility, "GDK.accessibility returns service object")
	if accessibility == null:
		return

	for method_name in [
		"query_closed_caption_properties",
		"set_closed_caption_enabled",
		"query_high_contrast_mode",
		"get_high_contrast_mode_name",
	]:
		assert_has_method_named(accessibility, method_name)

	var blank_service = instantiate_class("GDKAccessibility")
	assert_not_null(blank_service, "GDKAccessibility.new() returns service wrapper")
	if blank_service != null:
		var blank_caption_result = blank_service.query_closed_caption_properties()
		assert_result_error(blank_caption_result, "runtime_unavailable", "blank GDKAccessibility query_closed_caption_properties() requires runtime")

		var blank_set_result = blank_service.set_closed_caption_enabled(false)
		assert_result_error(blank_set_result, "runtime_unavailable", "blank GDKAccessibility set_closed_caption_enabled() requires runtime")

		var blank_contrast_result = blank_service.query_high_contrast_mode()
		assert_result_error(blank_contrast_result, "runtime_unavailable", "blank GDKAccessibility query_high_contrast_mode() requires runtime")

	var blank_props = instantiate_class("GDKClosedCaptionProperties")
	assert_not_null(blank_props, "GDKClosedCaptionProperties.new() returns wrapper")
	if blank_props != null:
		assert_eq(blank_props.enabled, false, "blank closed-caption properties start disabled")
		assert_eq(blank_props.font_scale, 1.0, "blank closed-caption properties start with font_scale 1.0")
		assert_eq(
			blank_props.get_font_edge_attribute(),
			get_class_constant("GDKClosedCaptionProperties", "FONT_EDGE_ATTRIBUTE_DEFAULT"),
			"blank closed-caption edge attribute defaults to DEFAULT")
		assert_eq(blank_props.get_font_edge_attribute_name(), "default", "blank closed-caption edge attribute name defaults to default")
		assert_eq(
			blank_props.get_font_style(),
			get_class_constant("GDKClosedCaptionProperties", "FONT_STYLE_DEFAULT"),
			"blank closed-caption font style defaults to DEFAULT")
		assert_eq(blank_props.get_font_style_name(), "default", "blank closed-caption font style name defaults to default")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult for accessibility behavior")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Accessibility runtime behavior: %s" % init_result.message)
		return

	var caption_result = accessibility.query_closed_caption_properties()
	assert_not_null(caption_result, "query_closed_caption_properties() returns GDKResult")
	if caption_result != null:
		if caption_result.ok:
			assert_object_is(caption_result.data, "GDKClosedCaptionProperties", "query_closed_caption_properties() returns GDKClosedCaptionProperties in result.data")
		else:
			assert_true(caption_result.code.length() > 0, "query_closed_caption_properties() failure includes an error code")
			assert_true(caption_result.message.length() > 0, "query_closed_caption_properties() failure includes an error message")

	var high_contrast_result = accessibility.query_high_contrast_mode()
	assert_not_null(high_contrast_result, "query_high_contrast_mode() returns GDKResult")
	if high_contrast_result != null:
		if high_contrast_result.ok:
			assert_true(high_contrast_result.data is Dictionary, "query_high_contrast_mode() success returns Dictionary payload")
			var contrast_data: Dictionary = high_contrast_result.data
			assert_true(contrast_data.has("mode"), "high-contrast payload includes mode")
			assert_true(contrast_data.has("mode_name"), "high-contrast payload includes mode_name")
			if contrast_data.has("mode") and contrast_data.has("mode_name"):
				assert_eq(
					contrast_data["mode_name"],
					accessibility.get_high_contrast_mode_name(contrast_data["mode"]),
					"mode_name matches get_high_contrast_mode_name(mode)")
		else:
			assert_true(high_contrast_result.code.length() > 0, "query_high_contrast_mode() failure includes an error code")
			assert_true(high_contrast_result.message.length() > 0, "query_high_contrast_mode() failure includes an error message")

	var target_enabled := false
	if caption_result != null and caption_result.ok and is_class_instance(caption_result.data, "GDKClosedCaptionProperties"):
		target_enabled = caption_result.data.enabled

	var set_result = accessibility.set_closed_caption_enabled(target_enabled)
	assert_not_null(set_result, "set_closed_caption_enabled() returns GDKResult")
	if set_result != null and not set_result.ok:
		assert_true(set_result.code.length() > 0, "set_closed_caption_enabled() failure includes an error code")
		assert_true(set_result.message.length() > 0, "set_closed_caption_enabled() failure includes an error message")
