extends GutTest

const ExportPresetCatalogScript = preload("res://addons/godot_gdk_packaging/core/export_preset_catalog.gd")

const _FIXTURE_DIR := "user://test_export_preset_catalog"


func before_each() -> void:
	_reset_fixture()
	DirAccess.make_dir_recursive_absolute(_fixture_root())


func after_each() -> void:
	_reset_fixture()


func test_parse_presets_returns_only_windows_desktop_presets_in_file_order() -> void:
	var content := ""
	content += "[preset.0]\n"
	content += "name=\"Web Build\"\n"
	content += "platform=\"Web\"\n\n"
	content += "[preset.1]\n"
	content += "name=\"Windows Debug\"\n"
	content += "platform=\"Windows Desktop\"\n\n"
	content += "[preset.4]\n"
	content += "name=\"Windows Release With Spaces\"\n"
	content += "platform=\"Windows Desktop\"\n\n"
	content += "[preset.5]\n"
	content += "name=\"Linux\"\n"
	content += "platform=\"Linux/X11\"\n"

	var presets: Array[Dictionary] = ExportPresetCatalogScript.parse_presets(content)

	assert_eq(presets.size(), 2)
	assert_eq(presets[0]["preset_index"], 1)
	assert_eq(presets[0]["name"], "Windows Debug")
	assert_eq(presets[0]["platform"], "Windows Desktop")
	assert_eq(presets[1]["preset_index"], 4)
	assert_eq(presets[1]["name"], "Windows Release With Spaces")


func test_parse_presets_accepts_custom_platform_filter() -> void:
	var content := ""
	content += "[preset.0]\nname=\"Windows\"\nplatform=\"Windows Desktop\"\n"
	content += "[preset.1]\nname=\"Web\"\nplatform=\"Web\"\n"

	var presets: Array[Dictionary] = ExportPresetCatalogScript.parse_presets(content, "Web")

	assert_eq(presets.size(), 1)
	assert_eq(presets[0]["preset_index"], 1)
	assert_eq(presets[0]["name"], "Web")
	assert_eq(presets[0]["platform"], "Web")


func test_list_windows_presets_reads_export_presets_cfg() -> void:
	var path: String = _fixture_path("export_presets.cfg")
	_write_text(path, "[preset.2]\nname=\"Windows Shipping\"\nplatform=\"Windows Desktop\"\n")

	var presets: Array[Dictionary] = ExportPresetCatalogScript.new().list_windows_presets(path)

	assert_eq(presets.size(), 1)
	assert_eq(presets[0]["preset_index"], 2)
	assert_eq(presets[0]["name"], "Windows Shipping")


func test_list_windows_presets_missing_file_returns_empty_array() -> void:
	var presets: Array[Dictionary] = ExportPresetCatalogScript.new().list_windows_presets(_fixture_path("missing.cfg"))

	assert_true(presets.is_empty(), "missing export_presets.cfg is not an error for the catalog")


func _fixture_root() -> String:
	return ProjectSettings.globalize_path(_FIXTURE_DIR)


func _fixture_path(relative_path: String) -> String:
	return _fixture_root().path_join(relative_path)


func _write_text(path: String, content: String) -> void:
	DirAccess.make_dir_recursive_absolute(path.get_base_dir())
	var file := FileAccess.open(path, FileAccess.WRITE)
	assert_not_null(file, "opened %s" % path)
	if file == null:
		return
	file.store_string(content)
	file.close()


func _reset_fixture() -> void:
	_remove_tree(_fixture_root())


func _remove_tree(path: String) -> void:
	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(path)
		return
	if not DirAccess.dir_exists_absolute(path):
		return
	for file_name: String in DirAccess.get_files_at(path):
		DirAccess.remove_absolute(path.path_join(file_name))
	for dir_name: String in DirAccess.get_directories_at(path):
		_remove_tree(path.path_join(dir_name))
	DirAccess.remove_absolute(path)
