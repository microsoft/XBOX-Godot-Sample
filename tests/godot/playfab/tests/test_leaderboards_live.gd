extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 4 — Live PlayFab Leaderboards contract with eventual-consistency
## settling.
##
## Read checks are gated by `requires_live()`. Only the submit + read-back
## round-trip is gated by `requires_live_write()` because it mutates a backing
## leaderboard in the configured PlayFab title.
##
## Eventual consistency: PlayFab leaderboards do not always reflect a freshly
## submitted score on the next read. We use `TestEnv.poll_until` with the
## `playfab/tests/leaderboard_settle_msec` budget; if that budget expires under
## live coverage, the timeout is a real failure.
##
## Cleanup: client-side leaderboard deletion is not part of the public API,
## so live write tests rely on per-run unique tags (via metadata) and unique
## per-process scores so collisions across CI runs cannot happen.

const _LEADERBOARD_NAME := "wave4_settle_smoke"
const _METADATA_PREFIX := "wave4_settle"
const _DEFAULT_OP_TIMEOUT_MSEC := 60000
const _FRIEND_SOURCES_ENV := "PLAYFAB_TEST_FRIEND_SOURCES"
const _FRIEND_ENTITY_IDS_ENV := "PLAYFAB_TEST_FRIEND_ENTITY_IDS"
const _API_NOT_ENABLED_HRESULT := 0x89235472


# ── Live setup ────────────────────────────────────────────────────────────

func _begin_live_session(
		write_required: bool = false,
		xbox_backed: bool = false) -> Dictionary:
	return await begin_playfab_live_session(
		"Live PlayFab Leaderboards",
		write_required,
		xbox_backed,
		false,
		true,
		_DEFAULT_OP_TIMEOUT_MSEC)


# ── Read-only leaderboards coverage (live) ────────────────────────────────

func test_get_leaderboard_async_live() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()

	var result = await _await_rate_limit_aware_live_result(
		func():
			return leaderboards.get_leaderboard_async(
				playfab_user, _LEADERBOARD_NAME, 1, 10, -1),
		"leaderboards.get_leaderboard_async()")
	if result == null:
		return

	assert_true(result.ok, "leaderboards.get_leaderboard_async() result.ok == true")
	if result.data is Dictionary:
		var response: Dictionary = result.data
		assert_true(response.has("rankings"), "get_leaderboard_async response includes rankings array")


func test_get_leaderboard_around_user_async_live() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()

	var result = await _await_rate_limit_aware_live_result(
		func():
			return leaderboards.get_leaderboard_around_user_async(
				playfab_user, _LEADERBOARD_NAME, 5, -1),
		"leaderboards.get_leaderboard_around_user_async()")
	if result == null:
		return

	assert_true(result.ok, "leaderboards.get_leaderboard_around_user_async() result.ok == true")


func test_get_friend_leaderboard_async_live() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()

	var legacy_result = await _await_rate_limit_aware_live_result(
		func():
			return leaderboards.get_friend_leaderboard_async(
				playfab_user, _LEADERBOARD_NAME, false, -1),
		"legacy PlayFab-only friend leaderboard")
	if legacy_result == null:
		return

	var legacy_response := _assert_friend_response_shape(
		legacy_result, "legacy PlayFab-only friend leaderboard")
	if legacy_response.is_empty():
		return

	var version := int(legacy_response.get("version", -1))
	var sources_result = await _await_rate_limit_aware_live_result(
		func():
			return leaderboards.get_friend_leaderboard_with_sources_async(
				playfab_user, _LEADERBOARD_NAME, 0, version),
		"explicit FRIEND_SOURCE_NONE leaderboard")
	if sources_result == null:
		return

	var sources_response := _assert_friend_response_shape(
		sources_result, "explicit FRIEND_SOURCE_NONE leaderboard")
	if sources_response.is_empty():
		return

	assert_eq(
		int(sources_response.get("version", -1)),
		version,
		"explicit FRIEND_SOURCE_NONE preserves the pinned leaderboard version")


func test_custom_id_rejects_xbox_friend_sources_live() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()
	assert_eq(int(playfab_user.local_id), 0, "custom-ID PlayFab session has no local Xbox id")

	for friend_sources in [4, 5, 16]:
		await _assert_required_live_error(
			leaderboards.get_friend_leaderboard_with_sources_async(
				playfab_user, _LEADERBOARD_NAME, friend_sources),
			"friend_leaderboard_xuser_not_found",
			"custom-ID friend source mask %d" % friend_sources)
	await _assert_required_live_error(
		leaderboards.get_friend_leaderboard_async(
			playfab_user, _LEADERBOARD_NAME),
		"friend_leaderboard_xuser_not_found",
		"legacy omitted/default true")
	await _assert_required_live_error(
		leaderboards.get_friend_leaderboard_async(
			playfab_user, _LEADERBOARD_NAME, true),
		"friend_leaderboard_xuser_not_found",
		"legacy explicit true")


func test_friend_leaderboard_name_validation_precedes_xbox_lookup_live() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()
	for friend_sources in [4, 16]:
		await _assert_required_live_error(
			leaderboards.get_friend_leaderboard_with_sources_async(
				playfab_user, "   ", friend_sources),
			"invalid_leaderboard_name",
			"blank leaderboard name with friend source mask %d" % friend_sources)


func test_get_friend_leaderboard_with_sources_external_fixture_live() -> void:
	if not requires_live():
		return

	var raw_sources := OS.get_environment(_FRIEND_SOURCES_ENV).strip_edges()
	if raw_sources.is_empty():
		pending(
			"External friend fixture not selected. Set %s and %s to run it." % [
				_FRIEND_SOURCES_ENV, _FRIEND_ENTITY_IDS_ENV])
		return
	if not raw_sources.is_valid_int():
		fail("%s must be a decimal friend-source mask." % _FRIEND_SOURCES_ENV)
		return

	var friend_sources := raw_sources.to_int()
	if friend_sources <= 0 or not (
		friend_sources <= 15 or friend_sources == 16):
		fail("%s must be a nonzero accepted mask (1..15 or 16)." % _FRIEND_SOURCES_ENV)
		return

	var expected_ids: Array[String] = []
	for raw_id in OS.get_environment(_FRIEND_ENTITY_IDS_ENV).split(",", false):
		var entity_id := String(raw_id).strip_edges()
		if not entity_id.is_empty():
			expected_ids.append(entity_id)
	if expected_ids.is_empty():
		fail("%s must contain at least one expected friend entity id." % _FRIEND_ENTITY_IDS_ENV)
		return

	var requires_xbox := friend_sources == 16 or (friend_sources & 4) != 0
	var session = await _begin_live_session(false, requires_xbox)
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()
	var none_result = await _await_rate_limit_aware_live_result(
		func():
			return leaderboards.get_friend_leaderboard_with_sources_async(
				playfab_user,
				_LEADERBOARD_NAME,
				get_class_constant("PlayFabLeaderboards", "FRIEND_SOURCE_NONE")),
		"external friend fixture NONE baseline")
	if none_result == null:
		return

	var none_response := _assert_friend_response_shape(
		none_result, "external friend fixture NONE baseline")
	if none_response.is_empty():
		return

	var version := int(none_response.get("version", -1))
	var none_ids := _friend_entity_ids(none_response.get("rankings", []))
	for expected_id in expected_ids:
		assert_false(
			none_ids.has(expected_id),
			"external friend fixture entity %s is absent from FRIEND_SOURCE_NONE" % expected_id)

	var result = await _await_rate_limit_aware_live_result(
		func():
			return leaderboards.get_friend_leaderboard_with_sources_async(
				playfab_user, _LEADERBOARD_NAME, friend_sources, version),
		"external friend fixture query")
	if result == null:
		return

	var response := _assert_friend_response_shape(
		result, "external friend fixture query")
	if response.is_empty():
		return

	assert_eq(
		int(response.get("version", -1)),
		version,
		"external friend fixture preserves the FRIEND_SOURCE_NONE baseline version")
	var actual_ids := _friend_entity_ids(response.get("rankings", []))

	for expected_id in expected_ids:
		assert_true(
			actual_ids.has(expected_id),
			"external friend fixture includes entity %s" % expected_id)


# ── Submit + read-back with eventual-consistency settling ─────────────────

func test_submit_score_settles_in_around_user_query() -> void:
	var session = await _begin_live_session(true)
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()

	# Per-process unique submission so cross-run races never collide. Score
	# is derived from the unique-id hash so we have a stable expected value
	# to look for in the around-user response.
	var run_tag := with_unique_id(_METADATA_PREFIX)
	var submitted_score := 1000 + (hash(run_tag) & 0xFFFF)

	var submit_signal = leaderboards.submit_score_async(
		playfab_user, _LEADERBOARD_NAME, submitted_score, [], run_tag)
	assert_eq(typeof(submit_signal), TYPE_SIGNAL,
		"leaderboards.submit_score_async() returns Signal for signed-in user")
	if typeof(submit_signal) != TYPE_SIGNAL:
		return

	var submit_result = await await_completion(submit_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	if submit_result == null:
		fail("submit_score_async timed out.")
		return
	if not submit_result.ok:
		if is_playfab_rate_limit_result(submit_result):
			pending(
				"submit_score_async hit E_PF_API_CLIENT_REQUEST_RATE_LIMIT_EXCEEDED "
				+ "(HRESULT 0x892354DD). Wait at least 150 seconds before rerunning the live-write tier.")
		elif (int(submit_result.hresult) & 0xFFFFFFFF) == _API_NOT_ENABLED_HRESULT:
			# Real settle coverage needs a statistic-backed fixture, which this
			# direct client endpoint cannot provide.
			pending(
				"submit_score_async is disabled for game-client access; "
				+ "a statistic-backed fixture is required for settle coverage.")
		else:
			fail("submit_score_async failed: [%s] %s" % [
				submit_result.code,
				submit_result.message,
			])
		return

	assert_true(submit_result.ok, "leaderboards.submit_score_async() result.ok == true")

	# Eventual-consistency settle. The pollable returns the matching ranking
	# Dictionary on success, null/false until the score appears.
	var settled = await TestEnv.poll_until(
		func():
			var around_signal = leaderboards.get_leaderboard_around_user_async(playfab_user, _LEADERBOARD_NAME, 5, -1)
			if typeof(around_signal) != TYPE_SIGNAL:
				return null
			var around_result = await await_completion(around_signal, _DEFAULT_OP_TIMEOUT_MSEC)
			if around_result == null or not around_result.ok:
				return null
			if not (around_result.data is Dictionary):
				return null
			var rankings: Array = around_result.data.get("rankings", [])
			for entry in rankings:
				if not (entry is Dictionary):
					continue
				var scores: Array = entry.get("scores", [])
				for s in scores:
					if int(str(s)) == int(submitted_score):
						return entry
			return null,
		-1)

	if settled == null:
		var settle_budget := int(ProjectSettings.get_setting(
			"playfab/tests/leaderboard_settle_msec", 30000))
		fail("leaderboard did not settle within %dms" % settle_budget)
		return

	assert_not_null(settled, "submitted score eventually appears in around-user query")


func _await_rate_limit_aware_live_result(
		operation: Callable,
		label: String):
	var retry = await await_playfab_result_with_rate_limit_retry(
		operation,
		label,
		_DEFAULT_OP_TIMEOUT_MSEC)
	var failure_kind := str(retry.get("failure_kind", ""))
	assert_false(failure_kind == "did_not_start", "%s returns Signal" % label)
	if failure_kind == "did_not_start":
		return null
	if not failure_kind.is_empty():
		fail(str(retry.get("failure_message", "%s failed." % label)))
		return null

	var result = retry.get("result")
	if result == null:
		fail("%s did not return a result." % label)
		return null
	if not result.ok:
		fail("%s failed: [%s] %s" % [label, result.code, result.message])
		return null
	return result


func _assert_required_live_error(
		async_signal,
		expected_code: String,
		label: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns Signal" % label)
	if typeof(async_signal) != TYPE_SIGNAL:
		return

	var result = await await_completion(async_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	if result == null:
		fail("%s timed out." % label)
		return
	assert_playfab_result_error(result, expected_code, label)


func _assert_friend_response_shape(result, label: String) -> Dictionary:
	if result == null or not (result.data is Dictionary):
		fail("%s returned non-dictionary data." % label)
		return {}

	var response: Dictionary = result.data
	for key in ["rankings", "entry_count", "version"]:
		assert_true(response.has(key), "%s response includes %s" % [label, key])
	if not (response.get("rankings") is Array):
		fail("%s rankings value is not an Array." % label)
		return {}
	return response


func _friend_entity_ids(rankings: Array) -> Array[String]:
	var entity_ids: Array[String] = []
	for row in rankings:
		if not (row is Dictionary):
			continue
		var entity = row.get("entity", {})
		if entity is Dictionary:
			entity_ids.append(str(entity.get("id", "")))
	return entity_ids
