## C4 sign-in smoke. Verifies the test client can load the PlayFab addon,
## initialize the runtime, and sign in with a custom_id. Live-gated.
##
## Skip conditions:
##   * LIVE_TESTS != "1"
##   * PLAYFAB_TITLE_ID env unset (cannot exercise PlayFab without a title)
##
## Custom id: PLAYFAB_CUSTOM_ID env, defaulting to "godot-gdk-ext-live-smoke"
## (matches tools/configure_playfab_test_title.ps1 provisioning).
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "_smoke.signin"
const SCENARIO_NAME: String = "C4 bootstrap: PlayFab sign_in_with_custom_id"
const CATEGORY: String = "functional"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available"]
const TIMEOUT_SEC: int = 60


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	# Custom id is derived in the test client from --role + env (see
	# tests/godot/mp_test_client/scripts/test_client.gd::_derive_custom_id_for_role).
	# We deliberately do NOT pass an explicit custom_id here so the harness
	# stays aligned with tools/configure_playfab_test_title.ps1's provisioning.
	var c = orch.client("host")
	var response: Dictionary = await c.send("sign_in", {}, 60_000)

	var err: Variant = assert_ok(response, "sign_in failed")
	if err != null:
		return err

	var result: Dictionary = response.get("result", {})
	var entity_key: Dictionary = result.get("entity_key", {})
	if String(entity_key.get("id", "")).is_empty():
		return fail("sign_in returned empty entity_key.id", { "result": result })

	return ok({
		"custom_id": result.get("custom_id", ""),
		"entity_id": entity_key.get("id", ""),
		"multiplayer_initialized": result.get("multiplayer_initialized", false),
	})
