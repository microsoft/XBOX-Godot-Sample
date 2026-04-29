@tool
extends EditorImportPlugin
## Minimal import plugin that makes .config files visible in the Godot
## FileSystem dock. The file is not transformed — it is kept as-is.


func _get_importer_name() -> String:
	return "gdk_packaging.config_file"


func _get_visible_name() -> String:
	return "GDK Config File"


func _get_recognized_extensions() -> PackedStringArray:
	return PackedStringArray(["config", "cfg"])


func _get_save_extension() -> String:
	return "res"


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


func _import(source_file: String, save_path: String, _options: Dictionary,
		_platform_variants: Array[String], _gen_files: Array[String]) -> Error:
	var res = Resource.new()
	return ResourceSaver.save(res, save_path + ".res")
