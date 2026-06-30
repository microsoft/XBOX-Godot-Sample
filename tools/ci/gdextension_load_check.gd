extends SceneTree

# CI GDExtension load smoke.
#
# Run headlessly against a coverage host project to assert that one or more
# .gdextension libraries are actually loaded by THIS Godot build. The build-once
# / test-across-Godot-versions pipeline reuses a single native addon build across
# every supported engine version, so this gives each per-version test leg a
# fast, deterministic fail with a clear message when a native addon DLL cannot
# load under that engine (ABI / GDExtension-interface incompatibility, missing
# redist DLL, etc.) -- instead of a confusing mid-suite GUT error.
#
# Usage:
#   godot --headless --path <host> -s res://tools/ci/gdextension_load_check.gd \
#         -- res://addons/<addon>/<addon>.gdextension [more.gdextension ...]
#
# The .gdextension res:// paths are passed as user args (after `--`). Exits 0
# when every requested extension is loaded, 1 when any is missing, 2 on misuse.

func _initialize() -> void:
	var requested := OS.get_cmdline_user_args()
	if requested.is_empty():
		push_error("[ci-load-smoke] No .gdextension paths provided (pass them after `--`).")
		quit(2)
		return

	var loaded := GDExtensionManager.get_loaded_extensions()
	print("[ci-load-smoke] Loaded extensions: ", ", ".join(loaded) if not loaded.is_empty() else "(none)")

	var failed := false
	for ext_path in requested:
		if GDExtensionManager.is_extension_loaded(ext_path):
			print("[ci-load-smoke] OK   loaded: ", ext_path)
		else:
			push_error("[ci-load-smoke] FAIL not loaded: " + ext_path)
			failed = true

	quit(1 if failed else 0)
