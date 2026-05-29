## C5a: a single host tracks two lobbies under distinct handles ("alpha",
## "beta") and gets a correct, per-handle snapshot back from each. Leaving
## "alpha" leaves "beta" addressable.
##
## Single-role live-gated scenario. Exercises the handle-keyed lobby tracking
## in PlayFabLobbyOps and proves a single signed-in user can hold two
## concurrent lobby memberships. Maps to
## `lobby.tracking_multiple_per_host` in
## spec/playfab-multiplayer-test-automation/1-test-matrix.md.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after every
## scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.tracking_multiple_per_host"
const SCENARIO_NAME: String = "Single host tracks two distinct lobbies under separate handles"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P1"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available"]
const TIMEOUT_SEC: int = 180


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	# Custom id derived in the test client from --role + env.
	var host = orch.client("host")

	var signed_in: Dictionary = await host.send("sign_in", {}, 60_000)
	var err: Variant = assert_ok(signed_in, "sign_in failed")
	if err != null:
		return err

	var created_alpha: Dictionary = await host.send("create_lobby", {
		"as": "alpha",
		"max_players": 2,
		"lobby_properties": { "scenario": "tracking_multiple_per_host", "tag": "alpha" },
	}, 60_000)
	err = assert_ok(created_alpha, "create_lobby(alpha) failed")
	if err != null:
		return err
	var alpha_id: String = String(created_alpha.get("result", {}).get("lobby", {}).get("lobby_id", ""))
	if alpha_id.is_empty():
		return fail("create_lobby(alpha) returned empty lobby_id", { "result": created_alpha.get("result", {}) })

	var created_beta: Dictionary = await host.send("create_lobby", {
		"as": "beta",
		"max_players": 2,
		"lobby_properties": { "scenario": "tracking_multiple_per_host", "tag": "beta" },
	}, 60_000)
	err = assert_ok(created_beta, "create_lobby(beta) failed")
	if err != null:
		return err
	var beta_id: String = String(created_beta.get("result", {}).get("lobby", {}).get("lobby_id", ""))
	if beta_id.is_empty():
		return fail("create_lobby(beta) returned empty lobby_id", { "result": created_beta.get("result", {}) })
	if alpha_id == beta_id:
		return fail("alpha and beta returned the same lobby_id", { "lobby_id": alpha_id })

	# Snapshots round-trip per handle.
	var alpha_snap: Dictionary = await host.send("get_lobby_snapshot", { "handle": "alpha" }, 15_000)
	err = assert_ok(alpha_snap, "get_lobby_snapshot(alpha) failed")
	if err != null:
		return err
	err = assert_eq(String(alpha_snap.get("result", {}).get("lobby", {}).get("lobby_id", "")), alpha_id, "alpha snapshot returned wrong lobby_id")
	if err != null:
		return err
	err = assert_eq(
		String(alpha_snap.get("result", {}).get("lobby", {}).get("properties", {}).get("tag", "")),
		"alpha",
		"alpha snapshot returned wrong tag property",
	)
	if err != null:
		return err

	var beta_snap: Dictionary = await host.send("get_lobby_snapshot", { "handle": "beta" }, 15_000)
	err = assert_ok(beta_snap, "get_lobby_snapshot(beta) failed")
	if err != null:
		return err
	err = assert_eq(String(beta_snap.get("result", {}).get("lobby", {}).get("lobby_id", "")), beta_id, "beta snapshot returned wrong lobby_id")
	if err != null:
		return err
	err = assert_eq(
		String(beta_snap.get("result", {}).get("lobby", {}).get("properties", {}).get("tag", "")),
		"beta",
		"beta snapshot returned wrong tag property",
	)
	if err != null:
		return err

	# Leave alpha; beta must remain addressable.
	var left_alpha: Dictionary = await host.send("leave_lobby", { "handle": "alpha" }, 30_000)
	err = assert_ok(left_alpha, "leave_lobby(alpha) failed")
	if err != null:
		return err
	err = assert_eq(String(left_alpha.get("result", {}).get("left_lobby_id", "")), alpha_id, "leave_lobby(alpha) returned wrong lobby_id")
	if err != null:
		return err

	var beta_snap_after: Dictionary = await host.send("get_lobby_snapshot", { "handle": "beta" }, 15_000)
	err = assert_ok(beta_snap_after, "get_lobby_snapshot(beta) after alpha leave failed")
	if err != null:
		return err
	err = assert_eq(
		String(beta_snap_after.get("result", {}).get("lobby", {}).get("lobby_id", "")),
		beta_id,
		"beta snapshot drifted after alpha leave",
	)
	if err != null:
		return err

	# alpha must no longer be tracked.
	var alpha_snap_after: Dictionary = await host.send("get_lobby_snapshot", { "handle": "alpha" }, 15_000)
	if bool(alpha_snap_after.get("ok", false)):
		return fail("get_lobby_snapshot(alpha) unexpectedly succeeded after leave", { "response": alpha_snap_after })
	var err_code: String = String(alpha_snap_after.get("error", {}).get("code", ""))
	err = assert_eq(err_code, "unknown_handle", "alpha snapshot after leave returned wrong error code")
	if err != null:
		return err

	return ok({ "alpha_lobby_id": alpha_id, "beta_lobby_id": beta_id })
