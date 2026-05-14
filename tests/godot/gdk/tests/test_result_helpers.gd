extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 4 GUT coverage for the `godot::String`-returning helpers in
## `addons/godot_gdk/src/gdk_result_codes_internal.{h,cpp}` and the
## public `GDKResult` static constructors that forward to them.
##
## See `spec/testing-strategy.md` ("godot::String constraint"): these
## forwarders are NOT runtime-testable from `gdk_unit_tests.exe` because the
## standalone exe cannot construct `godot::String`s. We pin the contract here
## end-to-end through the public `GDKResult` API from a real Godot process.

const HRESULT_HEX_BUFFER_SIZE := 11
const HRESULT_HEX_LITERAL_LENGTH := 10
const E_FAIL_HEX := "0x80004005"
const E_INVALIDARG_HEX := "0x80070057"
const E_ABORT_HEX := "0x80004004"
const S_OK_HEX := "0x00000000"


func test_format_hresult_string_shape() -> void:
	var format_hresult: Callable = Callable(GDKResult, "format_hresult")
	if not format_hresult.is_valid():
		# `format_hresult` is a `static` C++ helper that isn't bound via
		# ClassDB::bind_static_method, so it is not callable through GDScript.
		# The orchestrator's doctest target covers `format_hresult_hex`
		# directly; this end-to-end pin is only meaningful if/when the
		# static is exposed to script.
		pending("GDKResult.format_hresult is not exposed to GDScript as a callable static.")
		return

	var s_ok_text: String = format_hresult.call(0)
	assert_eq(s_ok_text, S_OK_HEX, "format_hresult(S_OK) renders 0x00000000")
	assert_eq(s_ok_text.length(), HRESULT_HEX_LITERAL_LENGTH, "S_OK hex literal is %d chars" % HRESULT_HEX_LITERAL_LENGTH)
	assert_eq(s_ok_text.length() + 1, HRESULT_HEX_BUFFER_SIZE, "format_hresult length + null terminator == HRESULT_HEX_BUFFER_SIZE")

	var e_fail_text: String = format_hresult.call(-2147467259)
	assert_eq(e_fail_text, E_FAIL_HEX, "format_hresult(E_FAIL) renders 0x80004005 (uppercase hex)")

	var e_invalidarg_text: String = format_hresult.call(-2147024809)
	assert_eq(e_invalidarg_text, E_INVALIDARG_HEX, "format_hresult(E_INVALIDARG) renders 0x80070057 (uppercase hex)")

	# Caller-provided sentinel HRESULT (0xDEADBEEF). Verifies sign-extension
	# round-trip through the (HRESULT)int64 -> unsigned int formatter path.
	var sentinel_text: String = format_hresult.call(-559038737)
	assert_eq(sentinel_text, "0xDEADBEEF", "format_hresult(0xDEADBEEF) renders uppercase hex")
	assert_eq(sentinel_text.to_upper(), sentinel_text, "format_hresult output is upper-case hex")


func test_gdk_result_ok_shape() -> void:
	var ok_result: Variant = null
	var ok_factory: Callable = Callable(GDKResult, "ok_result")
	if ok_factory.is_valid():
		ok_result = ok_factory.call()

	if ok_result == null:
		# `ok_result()` is a static C++ helper that may not be exposed to
		# script as a static. Fall back to a successful runtime call to get a
		# real ok GDKResult.
		var gdk: Object = get_gdk()
		if gdk == null:
			pending("GDKResult.ok shape requires either a script-visible static or a runtime call.")
			return
		# Drive a successful initialize() so we can capture its returned ok
		# GDKResult directly (the result-only refactor removed the
		# get_last_error() poll, so we use the return value as the canonical
		# ok shape).
		var init_result: Variant = gdk.initialize()
		if init_result == null or not init_result.ok:
			pending("GDKResult.ok shape requires a successful runtime init: %s" % (
				init_result.message if init_result != null else "GDK.initialize() returned null"))
			return
		ok_result = init_result

	assert_not_null(ok_result, "constructed GDKResult is non-null")
	if ok_result == null:
		return
	assert_object_is(ok_result, "GDKResult", "constructed value is GDKResult")
	assert_true(ok_result.ok, "ok GDKResult.ok == true")
	assert_eq(ok_result.code, "ok", "ok GDKResult.code == 'ok'")
	assert_eq(ok_result.message, "", "ok GDKResult.message is empty")
	assert_eq(ok_result.hresult, 0, "ok GDKResult.hresult == S_OK (0)")


func test_gdk_result_error_message_format() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	# Trigger a known-shape failure: repeated initialize() on an already-
	# initialized runtime. The result.code is "already_initialized" with
	# hresult E_FAIL — this exercises code_or_format_hresult() (provided code
	# wins over the formatted hex fallback).
	var first_init = gdk.initialize()
	if first_init == null or not first_init.ok:
		pending("Could not establish a runtime baseline for result-format coverage.")
		return

	var repeat_init = gdk.initialize()
	assert_not_null(repeat_init, "second initialize() returns GDKResult for format coverage")
	if repeat_init == null:
		gdk.shutdown()
		return

	assert_false(repeat_init.ok, "second initialize() reports failure")
	assert_eq(repeat_init.code, "already_initialized", "code_or_format_hresult: provided code wins over hex fallback")
	assert_true(repeat_init.message.length() > 0, "format_hresult_message(): non-empty message even with provided code")
	assert_true(repeat_init.message.find("(HRESULT ") >= 0, "format_hresult_message() embeds '(HRESULT 0x...)'")
	assert_true(repeat_init.message.find("0x") >= 0, "format_hresult_message() embeds the hex literal")
	assert_true(repeat_init.message.find(E_FAIL_HEX) >= 0, "format_hresult_message() renders the E_FAIL hex literal exactly")

	gdk.shutdown()


func test_format_hresult_message_empty_action() -> void:
	# We can observe the empty-action path indirectly: `cancelled()` is the
	# most stable construction site that uses error_result(...) directly with
	# a caller-supplied message and no action prefix. We don't call cancelled
	# (it's a static C++ helper not necessarily exposed as a static); instead
	# we drive the same code path via a service validation failure that
	# bypasses hresult_error and returns error_result(...) verbatim.
	if pending_unless_runtime_available():
		return

	var gdk: Object = get_gdk()
	# This validation path requires the runtime to be initialized so the
	# `presence` service is available. Drive init explicitly so headless
	# runs without GDK env pending instead of asserting against a
	# `not_initialized` failure code.
	if not gdk.is_initialized():
		var init_result: Variant = gdk.initialize()
		if init_result == null or not init_result.ok:
			pending("Empty-action format coverage requires a working runtime: %s" % (
				init_result.message if init_result != null else "GDK.initialize() returned null"))
			return

	var presence: Object = gdk.get_presence()
	if presence == null:
		pending("Presence service not available for result-format coverage.")
		return

	# `get_presence_async(PackedStringArray())` rejects with code
	# "missing_presence_xuids" and a hand-authored message string (no
	# format_hresult_message prefix). This pins the message-shape contract
	# for non-hresult validation errors.
	var error_signal = presence.get_presence_async(PackedStringArray())
	assert_eq(typeof(error_signal), TYPE_SIGNAL, "get_presence_async([]) returns Signal")
	if typeof(error_signal) != TYPE_SIGNAL:
		return
	var result = await await_completion(error_signal, 4000)
	assert_not_null(result, "validation failure returns a GDKResult")
	if result == null:
		return
	assert_false(result.ok, "validation failure has ok == false")
	assert_eq(result.code, "missing_presence_xuids", "validation failure preserves provided code")
	assert_true(result.message.length() > 0, "validation failure has a non-empty message")
	assert_true(result.message.find("(HRESULT ") < 0, "non-hresult validation message does not embed an HRESULT prefix")


func test_gdk_result_class_surface() -> void:
	for method_name in ["is_ok", "get_hresult", "get_code", "get_message", "get_data"]:
		assert_true(ClassDB.class_has_method("GDKResult", method_name), "GDKResult.%s() bound" % method_name)
	var prop_names := {}
	for prop_info in ClassDB.class_get_property_list("GDKResult"):
		prop_names[prop_info.get("name", "")] = true
	for prop in ["ok", "hresult", "code", "message", "data"]:
		assert_true(
			prop_names.has(prop),
			"GDKResult.%s property bound" % prop)
