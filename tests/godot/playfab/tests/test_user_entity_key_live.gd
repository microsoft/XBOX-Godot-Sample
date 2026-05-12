extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 4 — `PlayFabUser.entity_key` live coverage.
##
## Asserts that after a successful custom-ID PlayFab sign-in, the resulting
## `PlayFabUser.entity_key` is populated with a non-empty `id` and `type`,
## and that the values stay consistent across repeated reads and across
## fetching the same user back out of the cache.
##
## Live-only — gated by `pending_unless_live()` +
## `pending_unless_playfab_available()`. No mutation is performed.


func test_entity_key_populated_and_consistent_after_sign_in() -> void:
	if pending_unless_live():
		return
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		pending("Set ProjectSettings['playfab/titleid'] to exercise PlayFabUser.entity_key live coverage.")
		return

	reset_playfab_runtime()
	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		pending("PlayFab.initialize() live setup skipped: %s" % (init_result.message if init_result != null else "null result"))
		return

	var custom_id_session = await sign_in_with_configured_custom_id(playfab, "entity_key live coverage")
	var playfab_user = custom_id_session.get("playfab_user")
	if playfab_user == null:
		playfab.shutdown()
		return

	assert_object_is(playfab_user, "PlayFabUser", "PlayFab sign-in returns PlayFabUser")
	if playfab_user == null:
		playfab.shutdown()
		return

	var first_key: Dictionary = playfab_user.entity_key
	assert_true(first_key.has("id"), "PlayFabUser.entity_key has 'id' key")
	assert_true(first_key.has("type"), "PlayFabUser.entity_key has 'type' key")

	var first_id := str(first_key.get("id", ""))
	var first_type := str(first_key.get("type", ""))
	assert_true(not first_id.is_empty(), "PlayFabUser.entity_key.id is non-empty after sign-in")
	assert_true(not first_type.is_empty(), "PlayFabUser.entity_key.type is non-empty after sign-in")

	# Re-read from the same wrapper — values must be stable across reads.
	var second_key: Dictionary = playfab_user.entity_key
	assert_eq(str(second_key.get("id", "")), first_id, "entity_key.id is stable across repeated reads")
	assert_eq(str(second_key.get("type", "")), first_type, "entity_key.type is stable across repeated reads")

	# Re-fetch from the custom-ID cache and confirm the same key surfaces.
	var users = playfab.get_users()
	assert_eq(users.get_user_by_local_id(0), null, "PlayFab.users.get_user_by_local_id(0) does not return custom-ID sessions")
	var cached_user = users.get_user_by_custom_id(playfab_user.custom_id)
	assert_not_null(cached_user, "PlayFab.users.get_user_by_custom_id() returns the cached signed-in user")
	if cached_user != null:
		var cached_key: Dictionary = cached_user.entity_key
		assert_eq(str(cached_key.get("id", "")), first_id, "cached PlayFabUser.entity_key.id matches original")
		assert_eq(str(cached_key.get("type", "")), first_type, "cached PlayFabUser.entity_key.type matches original")

	playfab.shutdown()
