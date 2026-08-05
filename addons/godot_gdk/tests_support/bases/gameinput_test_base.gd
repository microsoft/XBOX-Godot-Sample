extends GutTest
## Shared GUT base for the `godot_gameinput` coverage suite.
##
## DOES NOT extend `GdkTestBase`. The GameInput addon is standalone — no
## build-time or runtime dependency on `godot_gdk` (per
## `.github/instructions/godot-gameinput.instructions.md`). Pulling in the
## GDK base would force every gameinput test host to also resolve the GDK
## addon, which violates that contract.
##
## Wave 3 GameInput tests should
## `extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"`.

const TestEnv = preload("res://addons/godot_gdk_tests/test_env.gd")

const FLOAT_EPSILON := 0.0001


# ── Singleton helpers ────────────────────────────────────────────────────

func get_gameinput():
	return Engine.get_singleton("GameInput") if Engine.has_singleton("GameInput") else null


# Pending the current test if the GameInput singleton is unavailable.
# Returns true when the runtime is missing (caller should `return` after).
func pending_unless_runtime_available() -> bool:
	if get_gameinput() == null:
		pending("GameInput singleton is not available in this host")
		return true
	return false


# ── Float comparison sugar ───────────────────────────────────────────────
# C++ float properties round-trip through 32-bit storage and won't equal
# 64-bit double literals exactly. This is the canonical `assert_eq_approx`
# referenced in `.github/instructions/godot-gameinput.instructions.md`.

func assert_eq_approx(actual: float, expected: float, name: String, eps: float = FLOAT_EPSILON) -> void:
	if absf(actual - expected) <= eps:
		assert_true(true, "%s ≈ %s" % [name, str(expected)])
	else:
		assert_true(false, "%s expected ≈ %s, got %s" % [name, str(expected), str(actual)])


# ── Reflection / class-introspection sugar ───────────────────────────────

func assert_has_method_named(obj: Object, method_name: String, test_name: String = "") -> void:
	var label := test_name if test_name else "%s.%s() exists" % [obj.get_class(), method_name]
	assert_true(obj.has_method(method_name), label)


func assert_has_signal_named(obj: Object, signal_name: String, test_name: String = "") -> void:
	var label := test_name if test_name else "%s.%s signal exists" % [obj.get_class(), signal_name]
	assert_true(obj.has_signal(signal_name), label)


# ── Pending semantics ────────────────────────────────────────────────────

# The live tiers are mandatory (see tools\run_all_tests.ps1), so a test that
# quietly degrades to `pending` is indistinguishable from coverage that never
# ran — which is exactly how the GameInput suite used to report green with no
# controller attached while its assertion count collapsed by 99%.
#
# When the live tier is active, `pending()` therefore FAILS instead. The
# orchestrator's live environment probe (stage 4) checks for an attached
# gamepad before the suite starts; anything that still reaches a `pending()`
# here is a real gap that must be visible.
#
# Direct GUT invocation (`godot -s res://addons/gut/gut_cmdln.gd` from a host
# root, the documented iterate-on-one-host workflow) does not set LIVE_TESTS, so
# `pending()` keeps its stock behavior there.
func pending(text := "") -> void:
	if TestEnv.live_tests_enabled():
		assert_true(false, "Test degraded to pending while the live tier is active — the live tier is mandatory, so this is a coverage gap, not a skip. Reason: " + text)
		return
	super.pending(text)


# Escape hatch for conditions that are legitimately tolerated even on a fully
# configured live machine. Always pends, never fails. Do NOT use this for "the
# environment isn't set up" — that is a failure.
func pending_tolerated(text := "") -> void:
	super.pending(text)


# ── TestEnv convenience wrappers ─────────────────────────────────────────

# Tier=live_read. The live tier is MANDATORY: `tools\run_all_tests.ps1` always
# sets LIVE_TESTS=1 and hard-fails in preflight when live configuration is
# missing. A missing flag means the suite was launched outside the supported
# entry point, so this FAILS the test rather than marking it pending.
#
# The bool return is retained so existing `if not requires_live(): return`
# call sites keep short-circuiting after the failure is recorded.
func requires_live() -> bool:
	if not TestEnv.live_tests_enabled():
		assert_true(false, "Tier=live_read requires LIVE_TESTS=1. The live tier is mandatory — run via `tools\\run_all_tests.ps1` (it sets the flag and validates live config in preflight).")
		return false
	return true


# Tier=live_write. Destructive tier — writes state that persists in the live
# SANDBOX title. Like requires_live(), the tier is mandatory and a missing
# flag is a FAILURE, not a skip.
func requires_live_write() -> bool:
	if not TestEnv.live_tests_enabled():
		assert_true(false, "Tier=live_write requires LIVE_TESTS=1. The live tier is mandatory — run via `tools\\run_all_tests.ps1`.")
		return false
	if not TestEnv.live_write_tests_enabled():
		assert_true(false, "Tier=live_write requires LIVE_WRITE_TESTS=1. The live-write tier is mandatory — run via `tools\\run_all_tests.ps1`, which sets it and validates the sandbox title id in preflight.")
		return false
	return true


# Legacy aliases. These no longer mark the test pending — they fail it, per
# requires_live() / requires_live_write() above.
func pending_unless_live() -> bool:
	return not requires_live()


func pending_unless_live_write() -> bool:
	return not requires_live_write()



func with_unique_id(prefix: String) -> String:
	return prefix + "-" + TestEnv.unique_run_id()
