@tool
extends EditorImportPlugin
## Minimal import plugin that makes .config and .cfg files visible in the Godot
## FileSystem dock. Godot normally hides unrecognized file types — this plugin
## registers them so MicrosoftGame.config and sample_config.cfg appear in the dock.
## The files are not transformed; a dummy Resource is saved to satisfy the import pipeline.


## Unique identifier for this importer (used by Godot's import system).
func _get_importer_name() -> String:
	return "gdk_packaging.config_file"


## Human-readable name shown in Godot's import dialog.
func _get_visible_name() -> String:
	return "GDK Config File"


## File extensions this plugin handles.
func _get_recognized_extensions() -> PackedStringArray:
	return PackedStringArray(["config", "cfg"])


## Extension for the imported resource (saved to .godot/imported/).
func _get_save_extension() -> String:
	return "res"


## Godot resource type produced by this importer.
func _get_resource_type() -> String:
	return "Resource"


func _get_preset_count() -> int:
	return 1


func _get_preset_name(_preset_index: int) -> String:
	return "Default"


func _get_import_options(_path: String, _preset_index: int) -> Array[Dictionary]:
	return []


func _get_import_order() -> int:
	return 0


func _get_priority() -> float:
	return 1.0


## Imports the file by saving a dummy Resource. The original file stays untouched.
func _import(source_file: String, save_path: String, _options: Dictionary,
		_platform_variants: Array[String], _gen_files: Array[String]) -> Error:
	var res = Resource.new()
	return ResourceSaver.save(res, save_path + ".res")
