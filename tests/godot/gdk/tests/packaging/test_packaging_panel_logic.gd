extends GutTest
## GUT coverage for `packaging_panel_logic.gd` — the pure helpers extracted
## from the `packaging_panel.gd` dock script. These tests run headless; the
## real panel is a `Control` and is not instantiated.

const PackagingPanelLogic = preload("res://addons/godot_gdk_packaging/core/packaging_panel_logic.gd")


# ── format_status_text ────────────────────────────────────────────────────

func test_status_text_available_with_version() -> void:
	assert_eq(
		PackagingPanelLogic.format_status_text(true, "260400"),
		"✅ GDK 260400",
		"version string is concatenated when GDK is available")


func test_status_text_available_no_version() -> void:
	assert_eq(
		PackagingPanelLogic.format_status_text(true, ""),
		"✅ GDK tools found",
		"empty version string falls back to generic available text")


func test_status_text_unavailable_ignores_version() -> void:
	assert_eq(
		PackagingPanelLogic.format_status_text(false, "260400"),
		"❌ GDK not found — install Microsoft GDK",
		"unavailable text is shown even if a version somehow leaked through")


# ── merge_settings_state ──────────────────────────────────────────────────

func test_merge_creates_missing_section() -> void:
	var target := {}
	var source := {"export": {"build_dir": "C:/Build"}}
	PackagingPanelLogic.merge_settings_state(target, source)
	assert_true(target.has("export"), "missing target section is created")
	assert_eq(target["export"]["build_dir"], "C:/Build", "value copied")


func test_merge_overwrites_existing_keys() -> void:
	var target := {"export": {"build_dir": "C:/Old", "package_dir": "C:/Pkg"}}
	var source := {"export": {"build_dir": "C:/New"}}
	PackagingPanelLogic.merge_settings_state(target, source)
	assert_eq(target["export"]["build_dir"], "C:/New", "existing key overwritten")
	assert_eq(target["export"]["package_dir"], "C:/Pkg", "untouched key preserved")


func test_merge_preserves_unrelated_sections() -> void:
	var target := {"sandbox": {"id": "XDKS.1"}}
	var source := {"export": {"build_dir": "C:/Build"}}
	PackagingPanelLogic.merge_settings_state(target, source)
	assert_eq(target["sandbox"]["id"], "XDKS.1", "untouched section preserved")
	assert_eq(target["export"]["build_dir"], "C:/Build", "new section added")


func test_merge_returns_target_for_chaining() -> void:
	var target := {}
	var ret = PackagingPanelLogic.merge_settings_state(target, {"a": {"b": 1}})
	assert_true(ret == target, "merge returns the same target dict")


# ── find_root_logos ───────────────────────────────────────────────────────

class _FakeFs:
	var present: Dictionary = {}
	func file_exists(path: String) -> bool:
		return present.get(path, false)


func test_find_root_logos_returns_only_present_files() -> void:
	var fs := _FakeFs.new()
	fs.present = {
		"C:/proj/StoreLogo.png": true,
		"C:/proj/Square480x480Logo.png": true,
	}
	var found: Array = PackagingPanelLogic.find_root_logos(
		"C:/proj",
		PackagingPanelLogic.ROOT_LOGO_FILES,
		Callable(fs, "file_exists"))
	assert_eq(found.size(), 2, "two files found")
	assert_true(found.has("StoreLogo.png"), "StoreLogo.png found")
	assert_true(found.has("Square480x480Logo.png"), "Square480x480Logo.png found")
	assert_false(found.has("Square44x44Logo.png"), "absent file omitted")


func test_find_root_logos_empty_when_none_present() -> void:
	var fs := _FakeFs.new()
	var found: Array = PackagingPanelLogic.find_root_logos(
		"C:/proj",
		PackagingPanelLogic.ROOT_LOGO_FILES,
		Callable(fs, "file_exists"))
	assert_eq(found.size(), 0, "no files found")


func test_find_root_logos_safe_with_empty_inputs() -> void:
	var fs := _FakeFs.new()
	assert_eq(
		PackagingPanelLogic.find_root_logos("", PackagingPanelLogic.ROOT_LOGO_FILES, Callable(fs, "file_exists")).size(),
		0,
		"empty project_dir returns empty")
	assert_eq(
		PackagingPanelLogic.find_root_logos("C:/proj", [], Callable(fs, "file_exists")).size(),
		0,
		"empty list returns empty")
	assert_eq(
		PackagingPanelLogic.find_root_logos("C:/proj", PackagingPanelLogic.ROOT_LOGO_FILES, Callable()).size(),
		0,
		"invalid callable returns empty")


func test_root_logo_files_list_matches_panel_expectations() -> void:
	# Pin the canonical filenames so a future panel change doesn't silently
	# diverge from packaging_content_preparer's logo_keys mapping.
	var expected := [
		"StoreLogo.png",
		"Square44x44Logo.png",
		"Square150x150Logo.png",
		"Square480x480Logo.png",
		"SplashScreenImage.png",
	]
	assert_eq(PackagingPanelLogic.ROOT_LOGO_FILES.size(), expected.size(), "five logo filenames listed")
	for name in expected:
		assert_true(PackagingPanelLogic.ROOT_LOGO_FILES.has(name), "%s in ROOT_LOGO_FILES" % name)
