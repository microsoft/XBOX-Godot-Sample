@tool
extends RefCounted
## Discovers export presets from export_presets.cfg.

const WINDOWS_DESKTOP_PLATFORM := "Windows Desktop"


func list_windows_presets(export_presets_path: String) -> Array[Dictionary]:
	if not FileAccess.file_exists(export_presets_path):
		return []

	var file = FileAccess.open(export_presets_path, FileAccess.READ)
	if file == null:
		return []

	var content = file.get_as_text()
	file.close()
	return parse_presets(content, WINDOWS_DESKTOP_PLATFORM)


static func parse_presets(content: String, platform_name: String = WINDOWS_DESKTOP_PLATFORM) -> Array[Dictionary]:
	var presets: Array[Dictionary] = []
	var regex = RegEx.new()
	regex.compile('\\[preset\\.(\\d+)\\][\\s\\S]*?name="([^"]*)"[\\s\\S]*?platform="([^"]*)"')
	for result in regex.search_all(content):
		var preset_index = int(result.get_string(1))
		var preset_name = result.get_string(2)
		var preset_platform = result.get_string(3)
		if preset_platform == platform_name:
			presets.append({
				"preset_index": preset_index,
				"name": preset_name,
				"platform": preset_platform,
			})
	return presets
