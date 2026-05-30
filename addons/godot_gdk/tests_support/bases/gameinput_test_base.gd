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


# ── TestEnv convenience wrappers ─────────────────────────────────────────

func requires_live() -> bool:
	if TestEnv.live_tests_enabled():
		return true
	pending("Skipped without LIVE_TESTS=1")
	return false


func requires_live_write() -> bool:
	if TestEnv.live_write_tests_enabled():
		return true
	pending("Skipped without LIVE_TESTS=1 and LIVE_WRITE_TESTS=1")
	return false


func pending_unless_live() -> bool:
	return not requires_live()


func with_unique_id(prefix: String) -> String:
	return prefix + "-" + TestEnv.unique_run_id()
