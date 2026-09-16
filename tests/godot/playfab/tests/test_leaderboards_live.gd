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


# ── Live setup ────────────────────────────────────────────────────────────

func _begin_live_session(
		write_required: bool = false,
		xbox_backed: bool = false) -> Dictionary:
	var outcome := {
		"playfab_user": null,
		"playfab": null,
	}

	if write_required and not requires_live_write():
		return outcome
	if not write_required and not requires_live():
		return outcome
	if pending_unless_playfab_available():
		return outcome

	var playfab = get_playfab()
	outcome["playfab"] = playfab

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		fail("Live PlayFab Leaderboards require ProjectSettings['playfab/runtime/title_id'].")
		playfab.shutdown()
		return outcome

	reset_playfab_runtime()
	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		fail("PlayFab.initialize() live setup failed: %s" % (
			init_result.message if init_result != null else "null result"))
		playfab.shutdown()
		return outcome

	if xbox_backed:
		var xbox_session = await ensure_gdk_primary_user_for_playfab(
			_DEFAULT_OP_TIMEOUT_MSEC)
		var xbox_user = xbox_session.get("user")
		if xbox_user == null:
			fail("Xbox-backed PlayFab setup failed: %s" % str(
				xbox_session.get("skip_reason", "no Xbox user returned")))
			playfab.shutdown()
			return outcome

		var sign_in_signal = playfab.users.sign_in_with_xuser_async(
			xbox_user, false)
		if typeof(sign_in_signal) != TYPE_SIGNAL:
			fail("PlayFab.users.sign_in_with_xuser_async() did not return a Signal.")
			playfab.shutdown()
			return outcome
		var sign_in_result = await await_completion(
			sign_in_signal, _DEFAULT_OP_TIMEOUT_MSEC)
		if sign_in_result == null:
			fail("Xbox-backed PlayFab sign-in timed out.")
			playfab.shutdown()
			return outcome
		if not sign_in_result.ok:
			fail("Xbox-backed PlayFab sign-in failed: %s" % sign_in_result.message)
			playfab.shutdown()
			return outcome
		if sign_in_result.data == null:
			fail("Xbox-backed PlayFab sign-in returned no PlayFabUser.")
			playfab.shutdown()
			return outcome

		outcome["playfab_user"] = sign_in_result.data
		return outcome

	var custom_id_session = await sign_in_with_configured_custom_id(playfab, "Leaderboards live test")
	var custom_id_result = custom_id_session.get("result")
	var custom_id_user = custom_id_session.get("playfab_user")
	if custom_id_result == null:
		fail("Custom-ID PlayFab sign-in did not return a result: %s" % str(
			custom_id_session.get("skip_reason", "unknown setup failure")))
		playfab.shutdown()
		return outcome
	if not custom_id_result.ok:
		fail("Custom-ID PlayFab sign-in failed: %s" % custom_id_result.message)
		playfab.shutdown()
		return outcome
	if custom_id_user == null:
		fail("Custom-ID PlayFab sign-in returned no PlayFabUser.")
		playfab.shutdown()
		return outcome

	outcome["playfab_user"] = custom_id_user
	return outcome


# ── Read-only leaderboards coverage (live) ────────────────────────────────

func test_get_leaderboard_async_live() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()

	var leaderboard_signal = leaderboards.get_leaderboard_async(playfab_user, _LEADERBOARD_NAME, 1, 10, -1)
	assert_eq(typeof(leaderboard_signal), TYPE_SIGNAL,
		"leaderboards.get_leaderboard_async() returns Signal for signed-in user")
	if typeof(leaderboard_signal) != TYPE_SIGNAL:
		playfab.shutdown()
		return

	var result = await await_completion(leaderboard_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	if result == null:
		fail("get_leaderboard_async timed out.")
		playfab.shutdown()
		return
	if not result.ok:
		pending("get_leaderboard_async returned non-ok in this host: %s" % result.message)
		playfab.shutdown()
		return

	assert_true(result.ok, "leaderboards.get_leaderboard_async() result.ok == true")
	if result.data is Dictionary:
		var response: Dictionary = result.data
		assert_true(response.has("rankings"), "get_leaderboard_async response includes rankings array")

	playfab.shutdown()


func test_get_leaderboard_around_user_async_live() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()

	var around_signal = leaderboards.get_leaderboard_around_user_async(playfab_user, _LEADERBOARD_NAME, 5, -1)
	assert_eq(typeof(around_signal), TYPE_SIGNAL,
		"leaderboards.get_leaderboard_around_user_async() returns Signal for signed-in user")
	if typeof(around_signal) != TYPE_SIGNAL:
		playfab.shutdown()
		return

	var result = await await_completion(around_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	if result == null:
		fail("get_leaderboard_around_user_async timed out.")
		playfab.shutdown()
		return
	if not result.ok:
		pending("get_leaderboard_around_user_async returned non-ok in this host: %s" % result.message)
		playfab.shutdown()
		return

	assert_true(result.ok, "leaderboards.get_leaderboard_around_user_async() result.ok == true")
	playfab.shutdown()


func test_get_friend_leaderboard_async_live() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var leaderboards = playfab.get_leaderboards()

	var legacy_result = await _await_required_live_result(
		leaderboards.get_friend_leaderboard_async(
			playfab_user, _LEADERBOARD_NAME, false, -1),
		"legacy PlayFab-only friend leaderboard")
	if legacy_result == null:
		playfab.shutdown()
		return

	var legacy_response := _assert_friend_response_shape(
		legacy_result, "legacy PlayFab-only friend leaderboard")
	if legacy_response.is_empty():
		playfab.shutdown()
		return

	var version := int(legacy_response.get("version", -1))
	var sources_result = await _await_required_live_result(
		leaderboards.get_friend_leaderboard_with_sources_async(
			playfab_user, _LEADERBOARD_NAME, 0, version),
		"explicit FRIEND_SOURCE_NONE leaderboard")
	if sources_result == null:
		playfab.shutdown()
		return

	var sources_response := _assert_friend_response_shape(
		sources_result, "explicit FRIEND_SOURCE_NONE leaderboard")
	if sources_response.is_empty():
		playfab.shutdown()
		return

	assert_eq(
		int(sources_response.get("version", -1)),
		version,
		"explicit FRIEND_SOURCE_NONE preserves the pinned leaderboard version")
	assert_eq(
		int(sources_response.get("entry_count", -1)),
		int(legacy_response.get("entry_count", -1)),
		"legacy false and FRIEND_SOURCE_NONE return the same entry count")
	assert_eq(
		_ranking_identity_scores(sources_response.get("rankings", [])),
		_ranking_identity_scores(legacy_response.get("rankings", [])),
		"legacy false and FRIEND_SOURCE_NONE return the same identities and scores")

	playfab.shutdown()


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

	playfab.shutdown()


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

	playfab.shutdown()


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
	var result = await _await_required_live_result(
		playfab.get_leaderboards().get_friend_leaderboard_with_sources_async(
			playfab_user, _LEADERBOARD_NAME, friend_sources),
		"external friend fixture query")
	if result == null:
		playfab.shutdown()
		return

	var response := _assert_friend_response_shape(
		result, "external friend fixture query")
	if response.is_empty():
		playfab.shutdown()
		return

	var actual_ids: Array[String] = []
	for row in response.get("rankings", []):
		if not (row is Dictionary):
			continue
		var entity = row.get("entity", {})
		if entity is Dictionary:
			actual_ids.append(str(entity.get("id", "")))

	for expected_id in expected_ids:
		assert_true(
			actual_ids.has(expected_id),
			"external friend fixture includes entity %s" % expected_id)

	playfab.shutdown()


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
		playfab.shutdown()
		return

	var submit_result = await await_completion(submit_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	if submit_result == null:
		fail("submit_score_async timed out.")
		playfab.shutdown()
		return
	if not submit_result.ok:
		pending("submit_score_async returned non-ok in this host: %s" % submit_result.message)
		playfab.shutdown()
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
		playfab.shutdown()
		return

	assert_not_null(settled, "submitted score eventually appears in around-user query")
	playfab.shutdown()


func _await_required_live_result(async_signal, label: String):
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns Signal" % label)
	if typeof(async_signal) != TYPE_SIGNAL:
		return null

	var result = await await_completion(async_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	if result == null:
		fail("%s timed out." % label)
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


func _ranking_identity_scores(rankings: Array) -> Array:
	var normalized: Array = []
	for row in rankings:
		if not (row is Dictionary):
			continue
		var entity = row.get("entity", {})
		normalized.append({
			"entity_id": str(entity.get("id", "")) if entity is Dictionary else "",
			"entity_type": str(entity.get("type", "")) if entity is Dictionary else "",
			"scores": row.get("scores", PackedStringArray()),
		})
	return normalized
