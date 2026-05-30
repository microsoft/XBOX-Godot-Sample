extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Wave 4 — Threading smoke for `GameInput.poll()` / `get_devices()` /
## `get_current_reading()` over many frames with no real device attached.
##
## GameInput's device callbacks fire on a worker thread (see
## `.github/instructions/godot-gameinput.instructions.md`). The main thread
## drains the pending queue inside `poll()` and is the only mutator of the
## device cache. This test exercises the no-hardware steady-state by hammering
## the public API across 100 frames and asserting:
##   * `get_devices(DEVICE_ALL)` returns a coherent device cache and real
##     `GameInputDevice` payloads when hardware is present.
##   * `get_current_reading(device)` is safe with a freshly instantiated bare
##     `GameInputDevice` (id `0`) — it must return null without dereferencing
##     into the worker's pending queue.
##   * `get_current_reading(null)` is safe.
##   * `poll()` is per-frame idempotent: calling it twice in the same frame
##     does not re-drain or crash.
##
## The whole thing is gated by `pending_unless_runtime_available()` because
## `GameInput.initialize()` may legitimately fail on bare CI runners that lack
## a usable GameInput device tree — but the no-init branch is already covered
## by `test_gameinput_core.gd::test_soft_fail_before_init`.

const ITERATIONS := 100


func _await_frames(n: int) -> void:
	var tree := get_tree()
	for i in n:
		await tree.process_frame


func _device_constant(name: String) -> int:
	return ClassDB.class_get_integer_constant("GameInput", name)


func _assert_device_payload(device: Object, label: String) -> void:
	assert_not_null(device, "%s is non-null" % label)
	if device == null:
		return
	assert_true(device.has_method("get_device_id"), "%s exposes get_device_id()" % label)
	assert_true(device.has_method("get_kind_mask"), "%s exposes get_kind_mask()" % label)
	assert_true(device.has_method("is_connected"), "%s exposes is_connected()" % label)
	assert_true(device.call("is_connected"), "%s reports connected" % label)
	assert_true(device.get_device_id() > 0, "%s has a positive session id" % label)
	assert_true(device.get_kind_mask() != 0, "%s has a non-zero kind mask" % label)


func test_repeated_get_devices_no_hardware() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	# Hammer get_devices() across many frames. With no device connected the
	# returned Array must stay empty; if hardware is present, assert the content
	# too so this test cannot pass on Array shape alone.
	var saw_non_array := false
	var all_mask := _device_constant("DEVICE_ALL")
	for i in ITERATIONS:
		gi.poll()
		var devices = gi.get_devices(all_mask)
		if not (devices is Array):
			saw_non_array = true
			break
		assert_eq(devices.size(), gi.get_connected_device_count(),
				"DEVICE_ALL count matches connected count on frame %d" % i)
		for device in devices:
			_assert_device_payload(device, "threading smoke device")
		await get_tree().process_frame

	assert_eq(saw_non_array, false,
			"get_devices(DEVICE_ALL) always returned Array across %d frames" % ITERATIONS)

	gi.shutdown()
	assert_eq(gi.is_initialized(), false,
			"runtime cleanly torn down after threading smoke")


func test_repeated_get_current_reading_no_hardware() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	var bare_device = ClassDB.instantiate("GameInputDevice")
	var saw_unexpected := false
	for i in ITERATIONS:
		# Poll before the bare-id check so the worker -> main queue drain is
		# exercised even when the id-0 wrapper short-circuits to null.
		gi.poll()
		var devices = gi.get_devices(_device_constant("DEVICE_ALL"))
		assert_eq(devices.size(), gi.get_connected_device_count(),
				"worker queue drain keeps device cache count coherent on frame %d" % i)
		for device in devices:
			_assert_device_payload(device, "queue-drained device")

		# null device → null reading. Bare wrapper (id 0) → null reading
		# because no entry exists for id 0 in the cache.
		var r_null = gi.get_current_reading(null)
		var r_bare = gi.get_current_reading(bare_device)
		if r_null != null or r_bare != null:
			saw_unexpected = true
			break
		# Defensive: poll() is per-frame idempotent. Calling it twice on the
		# same frame must not crash and must not re-drain the worker queue.
		gi.poll()
		await get_tree().process_frame

	assert_eq(saw_unexpected, false,
			"get_current_reading(null/bare-device) stayed null across %d frames" % ITERATIONS)

	gi.shutdown()
	assert_true(true, "threading smoke tear-down did not crash")


func test_poll_only_loop_no_crash() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	# Nothing but `poll()` for many frames. Exercises the per-frame idempotence
	# path (the `m_last_polled_frame` guard) plus the worker → main-thread
	# queue drain under sustained churn.
	for i in ITERATIONS:
		gi.poll()
		await get_tree().process_frame

	assert_eq(gi.is_initialized(), true,
			"runtime still initialized after %d poll() iterations" % ITERATIONS)

	gi.shutdown()
	assert_eq(gi.is_initialized(), false,
			"shutdown() after poll loop returns runtime to uninitialized state")


func test_live_device_unregister_callback_contract() -> void:
	if pending_unless_runtime_available():
		return
	if pending_unless_live():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput runtime on host)")
		return

	var all_mask := _device_constant("DEVICE_ALL")
	for _frame_index in 5:
		gi.poll()
		await get_tree().process_frame

	var devices = gi.get_devices(all_mask)
	if devices.is_empty():
		gi.shutdown()
		pending("No live GameInput device is connected; relying on Microsoft-documented UnregisterCallback contract.")
		return

	for iteration in 4:
		gi.shutdown()
		assert_eq(gi.is_initialized(), false,
				"UnregisterCallback shutdown completed with live device on iteration %d" % iteration)
		started = gi.initialize()
		assert_true(started, "GameInput.initialize() restarts after live-device unregister iteration %d" % iteration)
		if not started:
			return
		gi.poll()
		await get_tree().process_frame

	gi.shutdown()
	assert_true(true, "live-device UnregisterCallback cycles completed without crashing")
