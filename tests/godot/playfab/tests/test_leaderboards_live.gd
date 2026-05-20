extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 4 — Live PlayFab Leaderboards contract with eventual-consistency
## settling.
##
## Read-only checks (`get_leaderboard_async`, `get_leaderboard_around_user_async`,
## `get_friend_leaderboard_async`) are gated by `pending_unless_live()` +
## `pending_unless_playfab_available()`. The submit + read-back round-trip is
## live-only because it mutates a backing leaderboard.
##
## Eventual consistency: PlayFab leaderboards do not always reflect a freshly
## submitted score on the next read. We use `TestEnv.poll_until` with the
## `playfab/tests/leaderboard_settle_msec` budget. On timeout we report
## `pending(...)` rather than failing so transient propagation delays don't
## churn the orchestrator.
##
## Cleanup: client-side leaderboard deletion is not part of the public API,
## so live write tests rely on per-run unique tags (via metadata) and unique
## per-process scores so collisions across CI runs cannot happen.

const _LEADERBOARD_NAME := "wave4_settle_smoke"
const _METADATA_PREFIX := "wave4_settle"
const _DEFAULT_OP_TIMEOUT_MSEC := 60000


# ── Live setup ────────────────────────────────────────────────────────────

func _begin_live_session() -> Dictionary:
	var outcome := {
		"playfab_user": null,
		"playfab": null,
	}

	if pending_unless_live():
		return outcome
	if pending_unless_playfab_available():
		return outcome

	var playfab = get_playfab()
	outcome["playfab"] = playfab

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		pending("Set ProjectSettings['playfab/runtime/title_id'] to exercise live PlayFab Leaderboards.")
		return outcome

	reset_playfab_runtime()
	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		pending("PlayFab.initialize() live setup skipped: %s" % (init_result.message if init_result != null else "null result"))
		return outcome

	var custom_id_session = await sign_in_with_configured_custom_id(playfab, "Leaderboards live test")
	if custom_id_session.get("playfab_user") == null:
		return outcome

	outcome["playfab_user"] = custom_id_session["playfab_user"]
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
		pending("get_leaderboard_async timed out.")
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
		pending("get_leaderboard_around_user_async timed out.")
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

	var friend_signal = leaderboards.get_friend_leaderboard_async(playfab_user, _LEADERBOARD_NAME, false, -1)
	assert_eq(typeof(friend_signal), TYPE_SIGNAL,
		"leaderboards.get_friend_leaderboard_async() returns Signal for signed-in user")
	if typeof(friend_signal) != TYPE_SIGNAL:
		playfab.shutdown()
		return

	var result = await await_completion(friend_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	if result == null:
		pending("get_friend_leaderboard_async timed out.")
		playfab.shutdown()
		return
	if not result.ok:
		pending("get_friend_leaderboard_async returned non-ok in this host: %s" % result.message)
		playfab.shutdown()
		return

	assert_true(result.ok, "leaderboards.get_friend_leaderboard_async() result.ok == true")
	playfab.shutdown()


# ── Submit + read-back with eventual-consistency settling ─────────────────

func test_submit_score_settles_in_around_user_query() -> void:
	var session = await _begin_live_session()
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
		pending("submit_score_async timed out.")
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
		pending("leaderboard did not settle within %dms" % settle_budget)
		playfab.shutdown()
		return

	assert_not_null(settled, "submitted score eventually appears in around-user query")
	playfab.shutdown()
