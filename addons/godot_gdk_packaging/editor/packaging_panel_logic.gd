@tool
extends RefCounted
## Pure logic helpers extracted from `packaging_panel.gd`.
##
## Refactor pin: this module hosts the dependency-free pieces of the panel
## (settings-state merging, root-logo discovery, status string formatting)
## so they can be exercised by GUT without instantiating the dock UI. The
## panel script delegates here; behavior must remain identical.

## Files the panel auto-relocates from the project root into `storelogos/`.
const ROOT_LOGO_FILES := [
	"StoreLogo.png",
	"Square44x44Logo.png",
	"Square150x150Logo.png",
	"Square480x480Logo.png",
	"SplashScreenImage.png",
]


## Deep-merges `source` into `target` per top-level section (keys of
## `source`). Each section is treated as a `Dictionary`; the key/value pairs
## from `source[section]` are written over the matching keys in
## `target[section]`. Sections not present in `target` are created from an
## empty dict.
##
## Mutates `target` in-place AND returns it for fluent use.
static func merge_settings_state(target: Dictionary, source: Dictionary) -> Dictionary:
	for section_name: String in source:
		var target_section: Dictionary = target.get(section_name, {})
		var source_section: Dictionary = source[section_name]
		for key: String in source_section:
			target_section[key] = source_section[key]
		target[section_name] = target_section
	return target


## Returns the subset of `root_logo_files` that exist directly at
## `project_dir`. The caller passes a `file_exists` callable so this stays
## pure for tests; in production the panel passes a thin lambda over
## `FileAccess.file_exists`.
static func find_root_logos(project_dir: String, root_logo_files: Array, file_exists: Callable) -> Array:
	var found: Array = []
	if project_dir.is_empty() or root_logo_files.is_empty() or not file_exists.is_valid():
		return found
	for filename: String in root_logo_files:
		var full_path: String = project_dir.path_join(str(filename))
		if bool(file_exists.call(full_path)):
			found.append(str(filename))
	return found


## Formats the GDK status string shown in the panel header. Mirrors the
## branches in `packaging_panel._build_ui` so the exact emoji/spacing stays
## under test.
static func format_status_text(is_available: bool, version_text: String) -> String:
	if is_available:
		if version_text != "":
			return "✅ GDK %s" % version_text
		return "✅ GDK tools found"
	return "❌ GDK not found — install Microsoft GDK"
