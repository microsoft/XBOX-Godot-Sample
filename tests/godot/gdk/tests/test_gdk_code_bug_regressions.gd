extends "res://addons/godot_gdk_tests/gdk_test_base.gd"

const SOURCE_ROOT_FROM_TEST_HOST := "../../.."


func _read_repo_source(relative_path: String) -> String:
	var test_root := ProjectSettings.globalize_path("res://")
	var repo_root := test_root.path_join(SOURCE_ROOT_FROM_TEST_HOST).simplify_path()
	var source_path := repo_root.path_join(relative_path).simplify_path()
	assert_true(FileAccess.file_exists(source_path), "source file exists: %s" % relative_path)
	if not FileAccess.file_exists(source_path):
		return ""
	return FileAccess.get_file_as_string(source_path)


func _assert_contains(source: String, needle: String, message: String) -> void:
	assert_true(source.contains(needle), message)


func _assert_not_contains(source: String, needle: String, message: String) -> void:
	assert_false(source.contains(needle), message)


func test_presence_callback_context_uses_retained_weak_token() -> void:
	var header := _read_repo_source("addons/godot_gdk/src/gdk_presence.h")
	var source := _read_repo_source("addons/godot_gdk/src/gdk_presence.cpp")
	if header.is_empty() or source.is_empty():
		return

	_assert_contains(header, "std::shared_ptr<CallbackContext> callback_context", "presence handler owns callback context with shared_ptr")
	_assert_contains(header, "std::weak_ptr<CallbackContext> context", "presence callback token exposes only a weak_ptr to callbacks")
	_assert_contains(header, "m_retired_callback_tokens", "presence retains removed callback tokens beyond unregister")
	_assert_contains(header, "std::mutex mutex", "presence callback context owns its teardown mutex")
	_assert_contains(source, "token->context.lock()", "presence callback locks weak token before using context")
	_assert_contains(source, "std::lock_guard<std::mutex> context_lock(callback_context->mutex)", "presence callback serializes with handler teardown before using the service pointer")
	_assert_contains(source, "active.load(std::memory_order_acquire)", "presence callback checks active flag before queueing events")
	_assert_not_contains(source, "delete p_state.callback_context", "presence no longer deletes callback context immediately after handler removal")


func test_activation_is_single_native_registration_owner() -> void:
	var activation_source := _read_repo_source("addons/godot_gdk/src/gdk_activation.cpp")
	var multiplayer_source := _read_repo_source("addons/godot_gdk/src/gdk_multiplayer_activity.cpp")
	if activation_source.is_empty() or multiplayer_source.is_empty():
		return

	_assert_contains(activation_source, "XGameActivationRegisterForEvent", "GDKActivation owns the native activation registration")
	_assert_contains(activation_source, "add_activation_listener", "GDKActivation exposes internal listener registration")
	_assert_contains(activation_source, "notify_activation_listeners_internal", "GDKActivation fans out native events to internal listeners")
	_assert_contains(multiplayer_source, "activation->add_activation_listener", "GDKMultiplayerActivity subscribes to GDKActivation events")
	_assert_contains(multiplayer_source, "activation->remove_activation_listener", "GDKMultiplayerActivity unsubscribes from GDKActivation events")
	_assert_not_contains(multiplayer_source, "XGameActivationRegisterForEvent", "GDKMultiplayerActivity does not register a second native activation callback")
	_assert_not_contains(multiplayer_source, "XGameActivationUnregisterForEvent", "GDKMultiplayerActivity does not unregister native activation callbacks")


func test_package_mount_finalizer_cancels_before_native_result_work() -> void:
	var source := _read_repo_source("addons/godot_gdk/src/gdk_package.cpp")
	if source.is_empty():
		return

	var finalize_index := source.find("void finalize(XAsyncBlock *p_async_block) override")
	var gate_index := source.find("get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()", finalize_index)
	var result_index := source.find("XPackageMountWithUiResult", finalize_index)
	assert_true(finalize_index >= 0, "PackageMountAsyncContext finalizer exists")
	assert_true(gate_index >= 0, "package mount finalizer checks shutdown/cancel state")
	assert_true(result_index >= 0, "package mount finalizer still reads native mount result")
	assert_true(gate_index >= 0 and result_index >= 0 and gate_index < result_index, "package mount cancel gate runs before native result extraction")
	_assert_contains(source, "GDKResult::cancelled(\"Package mount cancelled.\")", "package mount cancellation completes with cancelled result")


func test_runtime_shutdown_completes_cancelled_pending_signals() -> void:
	var source := _read_repo_source("addons/godot_gdk/src/gdk_runtime.cpp")
	if source.is_empty():
		return

	var cancel_index := source.find("pending_signal->cancel()")
	# Shutdown must use synchronous complete(), not complete_deferred(). When
	# shutdown runs from SceneTree teardown there may be no idle frame left to
	# drain a deferred call, which would strand any awaiters whose signals were
	# already returned to callers.
	var complete_index := source.find("pending_signal->complete(GDKResult::cancelled", cancel_index)
	var terminate_index := source.find("XTaskQueueTerminate", cancel_index)
	assert_true(cancel_index >= 0, "runtime shutdown cancels active pending signals")
	assert_true(complete_index >= 0, "runtime shutdown synchronously completes cancelled pending signals (not deferred)")
	assert_true(terminate_index >= 0, "runtime shutdown terminates the task queue")
	assert_true(complete_index >= 0 and terminate_index >= 0 and complete_index < terminate_index, "pending signals complete before queue termination can strand awaiters")
	# Negative pin: complete_deferred at this site would re-introduce the
	# strand-during-_exit_tree bug Copilot flagged on PR #17. Look for the
	# actual function call (with paren) rather than the word in comments, so
	# explanatory comments mentioning the deprecated path do not break the pin.
	var deferred_call_idx := -1
	if cancel_index >= 0 and terminate_index > cancel_index:
		deferred_call_idx = source.find("pending_signal->complete_deferred(", cancel_index)
		if deferred_call_idx >= terminate_index:
			deferred_call_idx = -1
	assert_true(deferred_call_idx < 0, "shutdown cancel loop does not call pending_signal->complete_deferred() (would strand awaiters during SceneTree teardown)")


func test_presence_retired_callback_tokens_are_bounded() -> void:
	var source := _read_repo_source("addons/godot_gdk/src/gdk_presence.cpp")
	if source.is_empty():
		return

	# Long-running sessions repeatedly call track_presence/stop_tracking_presence,
	# each cycle retiring one callback token. Without a bound the vector grows
	# indefinitely. PR #17 review (Copilot) flagged the unbounded growth path.
	_assert_contains(source, "MAX_RETIRED_CALLBACK_TOKENS", "presence retired-token retention is explicitly bounded")
	_assert_contains(source, "m_retired_callback_tokens.erase(m_retired_callback_tokens.begin())", "presence FIFO-evicts the oldest retired token once the bound is reached")
	_assert_contains(source, "m_retired_callback_tokens.clear()", "presence drops every retired token at shutdown")
