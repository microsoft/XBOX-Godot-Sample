extends GutTest
## GUT coverage for the static helpers on `packaging_content_preparer.gd`
## (absorbed from the cancelled `cpp-packaging-helpers` workstream — the
## helpers are pure GDScript with no C++ surface, so this is the right
## test layer).
##
## Pinned behaviors:
##   * `patch_executable_name` uses Godot's `RegEx` and matches the FIRST
##     `Executable Name="…"` attribute. The replacement value is NOT XML-
##     escaped; callers are responsible for sanitising untrusted exe names.
##   * `inject_vc14_dependency` is idempotent: a config that already
##     contains `<KnownDependency Name="VC14"/>` is returned unchanged.
##   * Both helpers passthrough on no-match (no `Executable Name=`,
##     no `</Game>` tag).

const PackagingContentPreparerScript = preload("res://addons/godot_gdk_packaging/core/packaging_content_preparer.gd")


# ── patch_executable_name ─────────────────────────────────────────────────

func test_patch_executable_replaces_existing_attribute() -> void:
	var input := '<Executable Name="OldName.exe" Id="Game" />'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "NewName.exe")
	assert_string_contains(patched, 'Executable Name="NewName.exe"', "new exe name written")
	assert_false(patched.contains("OldName.exe"), "old exe name removed")


func test_patch_executable_no_match_passthrough() -> void:
	var input := '<Executable Id="Game" />'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "Anything.exe")
	assert_eq(patched, input, "input returned unchanged when no Executable Name= attribute is present")


func test_patch_executable_replaces_only_first_attribute() -> void:
	# Pin: the underlying RegEx.sub() called without `all` replaces only the
	# first match. Configs in practice have a single Executable, but we
	# document the actual behavior here.
	var input := '<Executable Name="A.exe" /><Executable Name="B.exe" />'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "C.exe")
	assert_true(patched.contains('Name="C.exe"'), "first attribute replaced")
	assert_true(patched.contains('Name="B.exe"'), "second attribute left intact (single replacement)")


func test_patch_executable_does_not_escape_xml_special_chars() -> void:
	# Pin: the implementation does NOT sanitise the replacement value — there
	# is no `xml_escape` helper in the addon. A name containing `&` or `"` is
	# inserted literally and would produce malformed XML downstream. This test
	# documents the current behavior so any future hardening is intentional.
	var input := '<Executable Name="OldName.exe" Id="Game" />'
	var unsafe := 'Game&Name<Special>.exe'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, unsafe)
	assert_string_contains(
		patched,
		'Executable Name="Game&Name<Special>.exe"',
		"unsafe characters inserted verbatim — no XML escaping is performed")


func test_patch_executable_handles_realistic_config_block() -> void:
	var input := ""
	input += '<?xml version="1.0" encoding="utf-8"?>\n'
	input += '<Game configVersion="1">\n'
	input += '  <ExecutableList>\n'
	input += '    <Executable Name="OldExe.exe" Id="Game" />\n'
	input += '  </ExecutableList>\n'
	input += '</Game>\n'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "MyGame.exe")
	assert_string_contains(patched, 'Executable Name="MyGame.exe"', "realistic config patched")
	assert_false(patched.contains("OldExe.exe"), "previous name gone")
	assert_string_contains(patched, "</Game>", "rest of XML preserved")


# ── inject_vc14_dependency ────────────────────────────────────────────────

func test_inject_vc14_inserts_block_before_close_game() -> void:
	var input := "<Game configVersion=\"1\">\n  <Identity Name=\"X\" />\n</Game>\n"
	var injected: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	assert_string_contains(injected, '<KnownDependency Name="VC14"/>', "VC14 dependency injected")
	assert_string_contains(injected, "<DesktopRegistration>", "DesktopRegistration wrapper added")
	assert_string_contains(injected, "</Game>", "</Game> still present")
	assert_lt(
		injected.find('<KnownDependency Name="VC14"/>'),
		injected.find("</Game>"),
		"dependency block sits BEFORE </Game>")


func test_inject_vc14_idempotent_when_already_present() -> void:
	var existing := "<Game>\n  <KnownDependency Name=\"VC14\"/>\n</Game>"
	var injected: String = PackagingContentPreparerScript.inject_vc14_dependency(existing)
	assert_eq(injected, existing, "input with VC14 already present returned unchanged")
	# And explicitly: only one occurrence after the call.
	var occurrences := injected.split('<KnownDependency Name="VC14"/>').size() - 1
	assert_eq(occurrences, 1, "no duplicate VC14 dependency added")


func test_inject_vc14_passthrough_when_no_close_game_tag() -> void:
	var input := "<NotAGameConfig />"
	var injected: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	assert_eq(injected, input, "input without </Game> returned unchanged")


func test_inject_vc14_runs_again_after_dependency_stripped() -> void:
	# Sanity round-trip: inject, strip the marker, re-inject. Verifies the
	# detection only keys on the literal `<KnownDependency Name="VC14"/>`.
	var input := "<Game>\n</Game>\n"
	var first: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	var stripped := first.replace('<KnownDependency Name="VC14"/>', '<KnownDependency Name="OTHER"/>')
	var second: String = PackagingContentPreparerScript.inject_vc14_dependency(stripped)
	assert_string_contains(second, '<KnownDependency Name="VC14"/>', "VC14 re-injected after marker removed")
	assert_string_contains(second, '<KnownDependency Name="OTHER"/>', "unrelated dependency preserved")


# ── pipeline (typical caller order) ───────────────────────────────────────

func test_pipeline_inject_then_patch_matches_caller_order() -> void:
	# `ensure_content_dir_ready` calls inject_vc14_dependency first, then
	# patch_executable_name. Document both transformations on a single
	# canonical config block.
	var input := ""
	input += '<?xml version="1.0" encoding="utf-8"?>\n'
	input += '<Game configVersion="1">\n'
	input += '  <ExecutableList>\n'
	input += '    <Executable Name="Placeholder.exe" Id="Game" />\n'
	input += '  </ExecutableList>\n'
	input += '</Game>\n'
	var step1: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	var step2: String = PackagingContentPreparerScript.patch_executable_name(step1, "RealGame.exe")
	assert_string_contains(step2, 'Executable Name="RealGame.exe"', "exe patched after inject")
	assert_string_contains(step2, '<KnownDependency Name="VC14"/>', "VC14 still present after patch")
