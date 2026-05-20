extends GutTest
## GUT coverage for the lifecycle contract of `gdk_packaging_plugin.gd`.
##
## The real plugin extends `EditorPlugin`, which is only fully wired up
## inside the Godot editor — `EditorInterface` and the editor MenuBar require
## an editor host. Headless we can't drive the actual
## `_enter_tree` / `_exit_tree` cycle without crashing on missing editor
## singletons. So this suite tests the contract two ways:
##
##   1. **Mock recorder**: a `MockEditorInterface` records every
##      `add_import_plugin` / `remove_import_plugin`,
##      and we drive a tiny harness through enter → exit → enter → exit to
##      assert the pairs balance and a second enter is idempotent w.r.t.
##      net resource state.
##   2. **Source inspection**: read the production script and assert that
##      every `add_*(…)` lifecycle call in `_enter_tree` has a matching
##      `remove_*(…)` in `_exit_tree`. Catches drift from someone adding
##      a register-without-unregister.

const PLUGIN_SCRIPT_PATH := "res://addons/godot_gdk_packaging/editor/gdk_packaging_plugin.gd"


# A stand-in for both EditorInterface and EditorPlugin — records every
# lifecycle call so we can assert balance and idempotency.
class MockEditorInterface extends RefCounted:
	var calls: Array = []
	var registered_imports: Dictionary = {}

	func add_import_plugin(p: Object) -> void:
		calls.append(["add_import_plugin", p])
		registered_imports[p.get_instance_id()] = p

	func remove_import_plugin(p: Object) -> void:
		calls.append(["remove_import_plugin", p])
		registered_imports.erase(p.get_instance_id())

	func reset() -> void:
		calls.clear()
		registered_imports.clear()


# Tiny harness mirroring the structure of `gdk_packaging_plugin._enter_tree`
# / `_exit_tree` minus the parts that depend on the real editor (MenuBar,
# popup wiring, toolchain detection). This keeps the test honest about
# the lifecycle pairing without trying to spin up the editor.
class _LifecycleHarness extends RefCounted:
	var _interface: MockEditorInterface
	var _import_plugin: Object = null

	func _init(iface: MockEditorInterface) -> void:
		_interface = iface

	func enter_tree() -> void:
		_import_plugin = RefCounted.new()
		_interface.add_import_plugin(_import_plugin)

	func exit_tree() -> void:
		if _import_plugin != null:
			_interface.remove_import_plugin(_import_plugin)
			_import_plugin = null


# ── Recorder lifecycle invariants ─────────────────────────────────────────

func test_enter_then_exit_balances_calls() -> void:
	var iface := MockEditorInterface.new()
	var harness := _LifecycleHarness.new(iface)

	harness.enter_tree()
	assert_eq(iface.registered_imports.size(), 1, "one import plugin registered after enter")

	harness.exit_tree()
	assert_eq(iface.registered_imports.size(), 0, "import plugin removed on exit")


func test_call_sequence_pairs_in_lifo_order() -> void:
	# add_import → remove_import. The editor GDK dock tab is intentionally not
	# registered anymore.
	var iface := MockEditorInterface.new()
	_LifecycleHarness.new(iface).enter_tree()
	# Build a fresh harness sequence to avoid leftover state.
	iface.reset()
	var harness := _LifecycleHarness.new(iface)
	harness.enter_tree()
	harness.exit_tree()

	var names: Array = []
	for entry in iface.calls:
		names.append(entry[0])
	assert_eq(
		names,
		[
			"add_import_plugin",
			"remove_import_plugin",
		],
		"calls fire in registration order on enter and reverse order on exit")


func test_repeated_enter_exit_cycles_remain_balanced() -> void:
	# Two full enter/exit cycles: every add must still be matched by a
	# remove and the registry/docked maps must drain to empty between
	# cycles.
	var iface := MockEditorInterface.new()
	for _i in range(2):
		var harness := _LifecycleHarness.new(iface)
		harness.enter_tree()
		assert_eq(iface.registered_imports.size(), 1, "registered after enter (cycle %d)" % _i)
		harness.exit_tree()
		assert_eq(iface.registered_imports.size(), 0, "drained after exit (cycle %d)" % _i)

	var add_count := 0
	var remove_count := 0
	for entry in iface.calls:
		var name: String = entry[0]
		if name.begins_with("add_"):
			add_count += 1
		elif name.begins_with("remove_"):
			remove_count += 1
	assert_eq(add_count, remove_count, "every add has a matching remove across both cycles")
	assert_eq(add_count, 2, "two cycles × one add call per cycle")


func test_exit_without_enter_is_a_noop() -> void:
	# Idempotency boundary: exiting a never-entered harness must not call
	# remove_* on null.
	var iface := MockEditorInterface.new()
	var harness := _LifecycleHarness.new(iface)
	harness.exit_tree()
	assert_eq(iface.calls.size(), 0, "no remove_* calls when nothing was added")


# ── Source-inspection invariant ───────────────────────────────────────────

func test_plugin_source_pairs_add_remove_lifecycle_calls() -> void:
	# Catches future drift in the production plugin: every add_* call in
	# `_enter_tree` must have a matching remove_* counterpart in
	# `_exit_tree`. The check is intentionally syntactic — the
	# refactor/test guards the structure, not the call site.
	var src := _read_plugin_source()
	assert_true(src.length() > 0, "plugin source loaded")

	var enter_block := _extract_func_body(src, "_enter_tree")
	var exit_block := _extract_func_body(src, "_exit_tree")
	assert_true(enter_block.length() > 0, "_enter_tree body extracted")
	assert_true(exit_block.length() > 0, "_exit_tree body extracted")

	var enter_calls := _scan_lifecycle_calls(enter_block, "add_")
	var exit_calls := _scan_lifecycle_calls(exit_block, "remove_")

	assert_true(enter_calls.has("add_import_plugin"), "_enter_tree calls add_import_plugin")
	assert_true(exit_calls.has("remove_import_plugin"), "_exit_tree calls remove_import_plugin")
	assert_false(enter_calls.has("add_control_to_dock"), "_enter_tree does not create a GDK dock tab")
	assert_false(exit_calls.has("remove_control_from_docks"), "_exit_tree has no GDK dock tab to remove")

	# Per-pair assertion (kept narrow — only checks the lifecycle methods
	# this addon actually uses).
	var pairings := {
		"add_import_plugin": "remove_import_plugin",
	}
	for add_name in pairings:
		var remove_name: String = pairings[add_name]
		assert_eq(
			enter_calls.get(add_name, 0),
			exit_calls.get(remove_name, 0),
			"%s in _enter_tree balanced by %s in _exit_tree" % [add_name, remove_name])


func _read_plugin_source() -> String:
	var f := FileAccess.open(PLUGIN_SCRIPT_PATH, FileAccess.READ)
	if f == null:
		return ""
	var text := f.get_as_text()
	f.close()
	return text


# Returns the body of `func <name>` up to the next `func ` declaration or
# end-of-file. Whitespace-tolerant; no full GDScript parse needed.
func _extract_func_body(src: String, fn_name: String) -> String:
	var marker := "func %s(" % fn_name
	var start := src.find(marker)
	if start < 0:
		return ""
	# Walk forward to the next top-level `func ` (column 0). Approximate by
	# searching for "\nfunc " starting after the marker.
	var search_from := start + marker.length()
	var next_func := src.find("\nfunc ", search_from)
	if next_func < 0:
		return src.substr(start)
	return src.substr(start, next_func - start)


# Counts `<prefix>X(…)` call occurrences in `body` and returns a dict of
# {call_name -> count}.
func _scan_lifecycle_calls(body: String, prefix: String) -> Dictionary:
	var counts: Dictionary = {}
	var pos := 0
	while true:
		var hit := body.find(prefix, pos)
		if hit < 0:
			break
		# Skip when prefix is part of a larger identifier (preceded by a
		# letter/digit/underscore).
		var is_word_boundary := true
		if hit > 0:
			var prev := body[hit - 1]
			if prev == "_" or prev.is_valid_identifier() or _is_ascii_letter_or_digit(prev):
				is_word_boundary = false
		if is_word_boundary:
			# Read identifier characters up to '('.
			var paren := body.find("(", hit)
			if paren > hit:
				var ident := body.substr(hit, paren - hit).strip_edges()
				if _looks_like_identifier(ident):
					counts[ident] = int(counts.get(ident, 0)) + 1
		pos = hit + prefix.length()
	return counts


func _is_ascii_letter_or_digit(ch: String) -> bool:
	if ch.length() == 0:
		return false
	var code := ch.unicode_at(0)
	return (code >= 0x30 and code <= 0x39) \
		or (code >= 0x41 and code <= 0x5A) \
		or (code >= 0x61 and code <= 0x7A)


func _looks_like_identifier(s: String) -> bool:
	if s.length() == 0:
		return false
	for i in s.length():
		var ch := s[i]
		if not (ch == "_" or _is_ascii_letter_or_digit(ch)):
			return false
	return true
