## Base class for MP test orchestrator scenarios.
##
## Subclasses declare metadata constants, override `run(orch)`, and return
## `ok()` / `fail()` Dictionaries. See spec/playfab-multiplayer-test-automation/4-scenario-authoring.md.
extends RefCounted

const DEFAULT_TIMEOUT_SEC: int = 60


func ok(details: Dictionary = {}) -> Dictionary:
	return {
		"ok": true,
		"failure_reason": "",
		"details": details,
	}


func fail(reason: String, details: Dictionary = {}) -> Dictionary:
	return {
		"ok": false,
		"failure_reason": reason,
		"details": details,
	}


func skip(reason: String) -> Dictionary:
	return {
		"ok": true,
		"skipped": true,
		"failure_reason": reason,
		"details": {},
	}


func requires_live(orch) -> Variant:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")
	return null


func requires_live_write(orch) -> Variant:
	var live_gate: Variant = requires_live(orch)
	if live_gate != null:
		return live_gate
	if orch.env("LIVE_WRITE_TESTS", "") != "1":
		return skip("LIVE_WRITE_TESTS != 1")
	return null


func assert_eq(actual: Variant, expected: Variant, message: String = "") -> Variant:
	if actual == expected:
		return null
	return fail("assertion_failed", {
		"message": message,
		"expected": expected,
		"actual": actual,
	})


func assert_true(condition: bool, message: String = "") -> Variant:
	if condition:
		return null
	return fail("assertion_failed", { "message": message })


func assert_false(condition: bool, message: String = "") -> Variant:
	if not condition:
		return null
	return fail("assertion_failed", { "message": message })


func assert_ok(response: Dictionary, message: String = "") -> Variant:
	if bool(response.get("ok", false)):
		return null
	return fail("response_not_ok", {
		"message": message,
		"error": response.get("error", {}),
	})


func assert_has(dict: Dictionary, key: String, message: String = "") -> Variant:
	if dict.has(key):
		return null
	return fail("missing_key", { "message": message, "key": key })
