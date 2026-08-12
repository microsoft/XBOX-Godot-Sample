@tool
extends EditorExportPlugin
## Re-adds the [code]gdk[/code] feature tag to a delegated [code]XBOX on PC[/code] export.
##
## The [code]XBOX on PC[/code] platform produces its .exe and .pck by handing
## its own preset to Godot's built-in [code]EditorExportPlatformWindows[/code]
## (see [code]gdk_export_platform.gd[/code]). Export feature tags come from the
## platform that actually performs the export, so during that delegated run the
## GDK platform's own [code]_get_platform_features()[/code] is never consulted
## and [code]gdk[/code] would silently vanish from packaged builds — a
## [code]has_feature("gdk")[/code] check, or a feature-tagged Project Setting
## override, would behave differently in a package than in the editor.
##
## An export plugin's features *are* consulted, whichever platform is running,
## so the tag is restored here for the duration of the delegated export.

const GDKExportPlatform = preload("res://addons/godot_gdk/editor/gdk_export_platform.gd")


func _get_name() -> String:
	return "GDKExportFeatures"


func _get_export_features(_platform: EditorExportPlatform, _debug: bool) -> PackedStringArray:
	# The flag is set only while GDKExportPlatform._export_project() is driving
	# the built-in Windows exporter, so a plain `Windows Desktop` export in the
	# same editor session is unaffected.
	if not GDKExportPlatform.exporting_for_gdk:
		return PackedStringArray()
	return PackedStringArray(["gdk"])
