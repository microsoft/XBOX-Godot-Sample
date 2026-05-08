extends SceneTree
## Wave 4 — Bootstrap mini-runner: GameInputBootstrap autoload effectively ABSENT.
##
## Run via:
##   godot --headless --script res://tests/bootstrap/test_bootstrap_autoload_absent.gd
##
## We can't rewrite the project.godot mid-run, so we simulate the "absent"
## case by detaching the autoload from the tree before it can do further work
## and verifying that the rest of the GameInput surface still functions. This
## is the load-bearing test for the addon's opt-in claim:
##
##   * Detach `/root/GameInputBootstrap` from the tree (queue_free + detach).
##   * Confirm the `GameInput` singleton is still reachable and its public
##     methods still soft-fail safely with no autoload around to drive them.
##   * Confirm a `GameInputMapper` can still be instantiated and added/removed
##     from the tree manually — the addon's runtime path is independent of
##     the autoload.
##
## Exits 0 on success with `BOOTSTRAP_AUTOLOAD_ABSENT: PASS` as the final line.
## On any failure prints `BOOTSTRAP_AUTOLOAD_ABSENT: FAIL` and exits 1.

const STATUS_TAG := "BOOTSTRAP_AUTOLOAD_ABSENT"
const AUTOLOAD_NODE_NAME := "GameInputBootstrap"


func _find_autoload() -> Node:
	for child in root.get_children():
		if child.name == AUTOLOAD_NODE_NAME:
			return child
	return null


func _initialize() -> void:
	# Defer one idle frame so autoloads have been added under root.
	await process_frame
	var failures: PackedStringArray = PackedStringArray()

	# Step 1 — verify the autoload was actually present pre-removal so this
	# test cannot pass by accident on a project where the autoload was never
	# installed at all.
	var autoload := _find_autoload()
	if autoload == null:
		failures.append("GameInputBootstrap autoload missing under root before removal — cannot prove 'absent' path is independent")
	else:
		# Step 2 — simulate "no autoload" by removing the bootstrap node.
		root.remove_child(autoload)
		autoload.free()
		if _find_autoload() != null:
			failures.append("Failed to detach GameInputBootstrap autoload")

	# Step 2 — without the bootstrap, the GameInput singleton must still
	# be usable directly (the addon is opt-in, not load-bearing).
	if not Engine.has_singleton("GameInput"):
		failures.append("Engine.has_singleton('GameInput') == false after autoload removal")
	else:
		var gi: Object = Engine.get_singleton("GameInput")
		if gi == null:
			failures.append("Engine.get_singleton('GameInput') returned null")
		else:
			# Soft-fail surface must keep working without the autoload.
			gi.poll()
			var devices = gi.get_devices()
			if not (devices is Array):
				failures.append("get_devices() did not return Array without autoload")

	# Step 3 — runtime classes can still be instantiated and integrated
	# into a fresh sub-tree without the autoload.
	if not ClassDB.class_exists("GameInputMapper"):
		failures.append("GameInputMapper class missing without autoload")
	else:
		var mapper: Node = ClassDB.instantiate("GameInputMapper") as Node
		if mapper == null:
			failures.append("GameInputMapper.new() returned null")
		else:
			var holder := Node.new()
			root.add_child(holder)
			holder.add_child(mapper)
			holder.remove_child(mapper)
			mapper.free()
			root.remove_child(holder)
			holder.free()

	if failures.is_empty():
		print("%s: PASS" % STATUS_TAG)
		quit(0)
	else:
		for line in failures:
			push_error("%s: %s" % [STATUS_TAG, line])
		print("%s: FAIL (%d issue%s)" %
				[STATUS_TAG, failures.size(),
				 ("" if failures.size() == 1 else "s")])
		quit(1)
