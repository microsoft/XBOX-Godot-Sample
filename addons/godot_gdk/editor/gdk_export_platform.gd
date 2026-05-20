@tool
extends EditorExportPlatformExtension
## Xbox/GDK (PC) export platform — packages Godot projects as MSIXVC for Xbox app.

const PLATFORM_NAME := "Xbox GDK (PC)"
const OS_NAME := "Windows"

# GDK tool paths (resolved on init)
var _gdk_root := ""
var _makepkg := ""
var _wdapp := ""
var _gdk_found := false

func _initialize() -> void:
	_detect_gdk()

func _get_name() -> String:
	return PLATFORM_NAME

func _get_os_name() -> String:
	return OS_NAME

func _get_logo() -> Texture2D:
	# Create a simple green placeholder icon (16x16)
	var img := Image.create(16, 16, false, Image.FORMAT_RGBA8)
	img.fill(Color(0.0, 0.5, 0.0))  # Xbox green
	return ImageTexture.create_from_image(img)

func _get_binary_extensions(p_preset: EditorExportPreset) -> PackedStringArray:
	return PackedStringArray(["msixvc"])

func _get_platform_features() -> PackedStringArray:
	return PackedStringArray(["pc", "windows", "gdk", "xbox", "d3d12", "x86_64"])

func _get_preset_features(p_preset: EditorExportPreset) -> PackedStringArray:
	return PackedStringArray(["windows", "gdk", "x86_64"])

func _get_export_options() -> Array[Dictionary]:
	return [
		# ── Packaging ──
		_opt("packaging/ekb_file", TYPE_STRING, "",
			PROPERTY_HINT_GLOBAL_FILE, "*.ekb",
			false),

		# ── Dev Iteration ──
		_opt("dev/register_loose", TYPE_BOOL, false,
			PROPERTY_HINT_NONE,
			"Skip MSIXVC packaging. Instead, register the loose staging folder via wdapp for fast dev iteration. Enable for inner-loop testing; leave off to produce a real .msixvc package."),
		_opt("dev/sandbox_id", TYPE_STRING, "RETAIL",
			PROPERTY_HINT_NONE, "Xbox Live sandbox ID (used by wdapp register)"),
	]

func _get_export_option_visibility(p_preset: EditorExportPreset, p_option: String) -> bool:
	# Hide packaging options when using loose registration; hide sandbox when
	# producing an MSIXVC (sandbox only applies to `wdapp register`).
	var register_loose: bool = false
	if p_preset.has("dev/register_loose"):
		register_loose = bool(p_preset.get("dev/register_loose"))
	if p_option == "packaging/ekb_file":
		return not register_loose
	if p_option == "dev/sandbox_id":
		return register_loose
	return true

func _get_export_option_warning(p_preset: EditorExportPreset, p_option: StringName) -> String:
	return ""

func _has_valid_export_configuration(p_preset: EditorExportPreset, p_debug: bool) -> bool:
	if not _gdk_found:
		return false
	# MicrosoftGame.config is the source of truth for identity / shell visuals
	# and is authored via the godot_gdk_packaging addon's "Create Game Config".
	# If it's missing the export pipeline cannot proceed.
	if not FileAccess.file_exists("res://MicrosoftGame.config"):
		return false
	return true

func _has_valid_project_configuration(p_preset: EditorExportPreset) -> bool:
	return _gdk_found

func _can_export(p_preset: EditorExportPreset, p_debug: bool) -> bool:
	return _has_valid_export_configuration(p_preset, p_debug)

func _export_project(p_preset: EditorExportPreset, p_debug: bool, p_path: String, p_flags: int) -> int:
	if not _gdk_found:
		push_error("GDK not found. Install via: winget install Microsoft.Gaming.GDK")
		return ERR_FILE_NOT_FOUND

	# Resolve output directory. Globalize first so the entire pipeline operates on
	# absolute paths — `DirAccess.open(out_dir).make_dir_recursive(staging_dir)`
	# treats `staging_dir` as relative to `out_dir`, which silently mis-creates the
	# staging folder when the preset uses a relative output path.
	var abs_p_path: String = ProjectSettings.globalize_path(p_path) if p_path.begins_with("res://") else p_path
	if not abs_p_path.is_absolute_path():
		abs_p_path = ProjectSettings.globalize_path("res://").path_join(abs_p_path)
	abs_p_path = abs_p_path.simplify_path()

	var out_dir: String = abs_p_path.get_base_dir()
	var staging_dir: String = out_dir.path_join("_gdk_staging")

	print("[GDK Export] Starting export to: ", abs_p_path)
	print("[GDK Export] Staging directory: ", staging_dir)

	# ── Step 1: Create staging directory ──
	if not DirAccess.dir_exists_absolute(out_dir):
		var mk_err: int = DirAccess.make_dir_recursive_absolute(out_dir)
		if mk_err != OK:
			push_error("GDK Export: Cannot create output directory: %s (err %d)" % [out_dir, mk_err])
			return ERR_FILE_BAD_PATH

	if DirAccess.dir_exists_absolute(staging_dir):
		_rmdir_recursive(staging_dir)
	var stage_err: int = DirAccess.make_dir_recursive_absolute(staging_dir)
	if stage_err != OK:
		push_error("GDK Export: Cannot create staging directory: %s (err %d)" % [staging_dir, stage_err])
		return ERR_FILE_BAD_PATH

	# Drop a .gdignore so Godot's resource importer doesn't try to import the
	# staged .exe / .pck / .config / logo files when the staging directory
	# lives inside the project tree (e.g. `<project>/builds/_gdk_staging/`).
	var gdignore := FileAccess.open(staging_dir.path_join(".gdignore"), FileAccess.WRITE)
	if gdignore != null:
		gdignore.close()

	# ── Step 2: Find Godot export template and export PCK ──
	# Derive the .exe name from the project's MicrosoftGame.config so the
	# staged executable matches what `<Executable Name="...">` declares. The
	# config (authored by the godot_gdk_packaging addon's "Create Game Config"
	# flow) is the single source of truth for identity, shell visuals, and the
	# executable name — no preset duplication.
	var exe_name: String = _read_exe_name_from_project_config()
	if exe_name == "":
		push_error(
			"GDK Export: MicrosoftGame.config not found or missing <Executable Name=...> at project root.\n" +
			"  Open the project in the editor and run GDK ▸ Create Game Config,\n" +
			"  or place a valid MicrosoftGame.config at the project root.")
		return ERR_FILE_NOT_FOUND
	var exe_path: String = staging_dir.path_join(exe_name)
	var pck_path: String = staging_dir.path_join(exe_name.get_basename() + ".pck")

	# Copy Windows export template (or fallback to current Godot exe for dev)
	var template_path: String = _find_windows_template(p_debug)
	if template_path == "":
		# Fallback: use the running Godot executable for dev iteration
		template_path = OS.get_executable_path()
		if template_path == "":
			push_error("GDK Export: No export template found and cannot locate Godot executable. Install export templates via Editor → Manage Export Templates.")
			return ERR_FILE_NOT_FOUND
		push_warning("GDK Export: Using Godot editor binary as template (install export templates for release builds)")

	var copy_err: int = DirAccess.copy_absolute(template_path, exe_path)
	if copy_err != OK:
		push_error("GDK Export: Failed to copy template to %s (err %d)" % [exe_path, copy_err])
		return copy_err
	print("[GDK Export] Template copied: ", exe_path)

	# Export PCK
	var pck_err: int = _export_pck(p_preset, p_debug, pck_path, p_flags)
	if pck_err != OK:
		push_error("GDK Export: PCK export failed")
		return pck_err

	print("[GDK Export] PCK exported: ", pck_path)

	# ── Step 3: Copy addon GDExtension main DLLs + support runtime DLLs ──
	# - Main DLLs (godot_*.windows.<config>.x86_64.dll) go to staging/addons/<name>/bin/
	#   so the .gdextension's res:// path resolves to disk at runtime.
	# - Support DLLs (libHttpClient, PlayFabCore, Microsoft.Xbox.Services.C.Thunks,
	#   Party, …) go to staging root so Windows's default DLL search finds them.
	var dll_err: int = _copy_addon_dlls(staging_dir, p_debug)
	if dll_err != OK:
		return dll_err

	# ── Step 4: Stage MicrosoftGame.config from the project ──
	# The packaging addon's "Create Game Config" flow writes the canonical
	# config + placeholder logos into the project. Don't regenerate them here —
	# just copy whatever the project already has.
	var config_err: int = _stage_microsoft_game_config(staging_dir)
	if config_err != OK:
		return config_err

	# ── Step 4b: Stage the logos referenced by the config ──
	# wdapp register / makepkg pack fail with 0x80070002 if any ShellVisuals
	# image is missing. Read the config we just staged and copy each referenced
	# logo from the project (preserving the relative path).
	_stage_logos(staging_dir)

	# ── Step 5: Package or register ──
	var use_loose: Variant = p_preset.get("dev/register_loose") if p_preset.has("dev/register_loose") else false

	if use_loose:
		return _wdapp_register(staging_dir)
	else:
		return _makepkg_pack(staging_dir, abs_p_path, p_preset)

# ── MicrosoftGame.config staging ─────────────────────────────────

# Reads `<Executable Name="...">` from the project's MicrosoftGame.config.
# Returns "" if the config is missing or has no Executable element. Centralized
# here so `_export_project` (deriving the staged .exe name) and editor checks
# can share one parse.
func _read_exe_name_from_project_config() -> String:
	var src: String = ProjectSettings.globalize_path("res://").path_join("MicrosoftGame.config")
	if not FileAccess.file_exists(src):
		return ""
	var content: String = FileAccess.get_file_as_string(src)
	if content == "":
		return ""
	var re := RegEx.new()
	re.compile('<Executable\\b[\\s\\S]*?Name="([^"]+)"')
	var m: RegExMatch = re.search(content)
	if m == null:
		return ""
	return m.get_string(1)

# Copies the project's `MicrosoftGame.config` into the staging dir, injecting
# `TargetDeviceFamily="PC"` on the `<Executable>` element if it's missing.
# The config itself is authored by the `godot_gdk_packaging` addon's "Create
# Game Config" flow (or by the developer directly via GameConfigEditor) —
# never generated at export time.
func _stage_microsoft_game_config(staging_dir: String) -> int:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	var src: String = project_dir.path_join("MicrosoftGame.config")
	if not FileAccess.file_exists(src):
		push_error(
			"GDK Export: MicrosoftGame.config not found at project root.\n" +
			"  Open the project in the editor and run GDK ▸ Create Game Config,\n" +
			"  or place a MicrosoftGame.config (and storelogos/) at the project root.")
		return ERR_FILE_NOT_FOUND

	var content: String = FileAccess.get_file_as_string(src)
	if content == "":
		push_error("GDK Export: Failed to read MicrosoftGame.config at %s" % src)
		return ERR_FILE_CANT_READ
	content = _inject_target_device_family(content)

	var dest: String = staging_dir.path_join("MicrosoftGame.config")
	var f := FileAccess.open(dest, FileAccess.WRITE)
	if f == null:
		push_error("GDK Export: Cannot write MicrosoftGame.config to %s" % dest)
		return ERR_FILE_CANT_WRITE
	f.store_string(content)
	f.close()
	print("[GDK Export] MicrosoftGame.config staged from project")
	return OK

# Injects `TargetDeviceFamily="PC"` on the `<Executable>` element if missing.
# makepkg refuses to pack a non-developer executable without it (error
# 0x80070057), and the packaging addon's older `create_template` output did
# not include the attribute.
func _inject_target_device_family(content: String) -> String:
	var tag_re := RegEx.new()
	tag_re.compile("<Executable\\b[\\s\\S]*?/?>")
	var m: RegExMatch = tag_re.search(content)
	if m == null:
		return content
	var tag: String = m.get_string(0)
	if tag.contains("TargetDeviceFamily"):
		return content
	var patched: String = tag
	if patched.ends_with("/>"):
		patched = patched.substr(0, patched.length() - 2) + ' TargetDeviceFamily="PC" />'
	elif patched.ends_with(">"):
		patched = patched.substr(0, patched.length() - 1) + ' TargetDeviceFamily="PC">'
	return content.substr(0, m.get_start()) + patched + content.substr(m.get_end())

# Reads the staged MicrosoftGame.config to discover which logo files it
# references, then copies each one from the project into the staging dir at
# the same relative path. Sources are tried in order: <project>/<rel-from-config>,
# <project>/storelogos/<filename>, <project>/<filename>. Missing logos surface
# as warnings — the subsequent wdapp/makepkg step will then fail with a
# specific 0x80070002 pointing at the offending file.
func _stage_logos(staging_dir: String) -> void:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	var config_path: String = staging_dir.path_join("MicrosoftGame.config")
	var content: String = FileAccess.get_file_as_string(config_path)
	if content == "":
		return

	var attrs: PackedStringArray = PackedStringArray([
		"StoreLogo",
		"Square150x150Logo",
		"Square44x44Logo",
		"Square480x480Logo",
		"SplashScreenImage",
	])
	var re := RegEx.new()
	for attr: String in attrs:
		re.compile(attr + '="([^"]+)"')
		var m: RegExMatch = re.search(content)
		if m == null:
			continue
		var rel: String = m.get_string(1).replace("\\", "/")
		var filename: String = rel.get_file()

		var candidates: PackedStringArray = PackedStringArray([
			project_dir.path_join(rel),
			project_dir.path_join("storelogos").path_join(filename),
			project_dir.path_join(filename),
		])
		var src: String = ""
		for c: String in candidates:
			if FileAccess.file_exists(c):
				src = c
				break
		if src == "":
			push_warning(
				"GDK Export: %s logo not found — expected at %s. " % [attr, project_dir.path_join(rel)] +
				"Run GDK ▸ Create Game Config to generate placeholders.")
			continue

		var dest: String = staging_dir.path_join(rel)
		DirAccess.make_dir_recursive_absolute(dest.get_base_dir())
		var err: int = DirAccess.copy_absolute(src, dest)
		if err != OK:
			push_warning("GDK Export: Failed to copy logo %s -> %s (err %d)" % [src, dest, err])
		else:
			print("[GDK Export] Logo staged: %s" % rel)

# ── Tool execution ──────────────────────────────────────────────

func _wdapp_register(staging_dir: String) -> int:
	print("[GDK Export] Registering loose folder via wdapp...")
	var global_path: String = ProjectSettings.globalize_path(staging_dir)
	var output: Array = []
	var exit_code: int = OS.execute(_wdapp, ["register", global_path], output, true)
	for line: Variant in output:
		print("[wdapp] ", line)
	if exit_code != 0:
		push_error("GDK Export: wdapp register failed (exit code %d)" % exit_code)
		return ERR_BUG
	print("[GDK Export] Registered successfully! Launch from Xbox app or Start menu.")
	return OK

func _makepkg_pack(staging_dir: String, output_path: String, p_preset: EditorExportPreset) -> int:
	var global_staging: String = ProjectSettings.globalize_path(staging_dir)
	var global_output: String = ProjectSettings.globalize_path(output_path)

	# Step 1: genmap
	print("[GDK Export] Generating file map...")
	var layout_path: String = global_staging + "\\layout.xml"
	var output1: Array = []
	var exit1: int = OS.execute(_makepkg, [
		"genmap", "/f", layout_path, "/d", global_staging
	], output1, true)
	for line: Variant in output1:
		print("[makepkg] ", line)
	if exit1 != 0:
		push_error("GDK Export: makepkg genmap failed")
		return ERR_BUG

	# Step 2: pack
	print("[GDK Export] Packing MSIXVC...")
	var output2: Array = []
	var pack_args: Array = [
		"pack", "/f", layout_path, "/d", global_staging, "/pd", global_output.get_base_dir()
	]

	# Add EKB file if provided
	var ekb: Variant = p_preset.get("packaging/ekb_file") if p_preset.has("packaging/ekb_file") else ""
	if ekb != "":
		pack_args.append_array(["/lk", ProjectSettings.globalize_path(ekb)])

	var exit2 := OS.execute(_makepkg, pack_args, output2, true)
	for line: Variant in output2:
		print("[makepkg] ", line)
	if exit2 != 0:
		push_error("GDK Export: makepkg pack failed")
		return ERR_BUG

	print("[GDK Export] Package created: ", global_output)
	return OK

# ── Helpers ─────────────────────────────────────────────────────

func _detect_gdk() -> void:
	var base: String = "C:\\Program Files (x86)\\Microsoft GDK"
	var edition_roots: Array[String] = []

	var env_roots: Array[String] = [OS.get_environment("GameDKCoreLatest"), OS.get_environment("GameDKLatest")]
	for raw_root: String in env_roots:
		if raw_root == "":
			continue
		var normalized_root: String = raw_root.trim_suffix("\\").trim_suffix("/")
		if not edition_roots.has(normalized_root):
			edition_roots.append(normalized_root)

	if DirAccess.dir_exists_absolute(base):
		var da := DirAccess.open(base)
		if da == null:
			_gdk_found = false
			return

		var editions: Array[String] = []
		da.list_dir_begin()
		var entry: String = da.get_next()
		while entry != "":
			if da.current_is_dir() and entry.substr(0, 1).is_valid_int():
				editions.append(entry)
			entry = da.get_next()
		da.list_dir_end()

		editions.sort()
		if not editions.is_empty():
			var latest_root: String = base + "\\" + editions[-1]
			if not edition_roots.has(latest_root):
				edition_roots.append(latest_root)

	if edition_roots.is_empty():
		push_warning("GDK Export: Microsoft GDK not found at ", base)
		_gdk_found = false
		return

	for root: String in edition_roots:
		if DirAccess.dir_exists_absolute(root + "\\windows"):
			_gdk_root = root
			break

	if _gdk_root == "":
		push_warning("GDK Export: No Windows-layout GDK installation was found")
		_gdk_found = false
		return

	var tools_root: String = _gdk_root.get_base_dir()
	_makepkg = tools_root + "\\bin\\makepkg.exe"
	_wdapp = tools_root + "\\bin\\wdapp.exe"

	if FileAccess.file_exists(_makepkg) and FileAccess.file_exists(_wdapp):
		_gdk_found = true
		print("[GDK Export] GDK found: ", _gdk_root)
		print("[GDK Export] Windows layout: ", _gdk_root + "\\windows")
		print("[GDK Export] makepkg: ", _makepkg)
		print("[GDK Export] wdapp: ", _wdapp)
	else:
		push_warning("GDK Export: GDK tools not found")
		_gdk_found = false

func _find_windows_template(p_debug: bool) -> String:
	var app_data: String = OS.get_environment("APPDATA")
	var templates_dir: String = app_data.path_join("Godot").path_join("export_templates")
	var ver: Dictionary = Engine.get_version_info()
	var ver_string: String = "%d.%d.%s" % [ver["major"], ver["minor"], ver["status"]]
	var suffix: String = "debug" if p_debug else "release"
	var template: String = templates_dir.path_join(ver_string).path_join("windows_%s_x86_64.exe" % suffix)
	if FileAccess.file_exists(template):
		return template
	# Fallback: try without status
	var ver_string2: String = "%d.%d" % [ver["major"], ver["minor"]]
	template = templates_dir.path_join(ver_string2).path_join("windows_%s_x86_64.exe" % suffix)
	if FileAccess.file_exists(template):
		return template
	return ""

# Walks every `addons/<name>/bin/` directory and copies:
# - GDExtension main DLLs (`godot_*.windows.<config>.x86_64.dll`, matching this
#   build's debug/release config) to `staging/addons/<name>/bin/<dll>` so the
#   .gdextension's `res://` reference resolves on disk at runtime.
# - All other `.dll` files (support runtimes such as libHttpClient.dll,
#   PlayFabCore.dll, Microsoft.Xbox.Services.C.Thunks.dll, Party.dll) to the
#   staging root so Windows's default DLL search finds them next to the .exe.
# GDExtension DLLs from the opposite build config are intentionally skipped so
# a release export does not leak debug binaries (and vice versa).
func _copy_addon_dlls(staging_dir: String, p_debug: bool) -> int:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	var addons_dir: String = project_dir.path_join("addons")
	if not DirAccess.dir_exists_absolute(addons_dir):
		return OK

	var this_config: String = "debug" if p_debug else "release"
	var other_config: String = "release" if p_debug else "debug"

	var main_re: RegEx = RegEx.new()
	main_re.compile("^godot_.*\\.windows\\.(?<cfg>[^.]+)\\.x86_64\\.dll$")

	var addons := DirAccess.open(addons_dir)
	if addons == null:
		push_warning("GDK Export: Cannot open addons directory: %s" % addons_dir)
		return OK

	var main_copied: int = 0
	var support_copied: int = 0

	addons.list_dir_begin()
	var addon_name: String = addons.get_next()
	while addon_name != "":
		if addons.current_is_dir() and not addon_name.begins_with("."):
			var bin_dir: String = addons_dir.path_join(addon_name).path_join("bin")
			if DirAccess.dir_exists_absolute(bin_dir):
				var bin := DirAccess.open(bin_dir)
				if bin != null:
					bin.list_dir_begin()
					var fname: String = bin.get_next()
					while fname != "":
						if not bin.current_is_dir() and fname.ends_with(".dll"):
							var src: String = bin_dir.path_join(fname)
							var m: RegExMatch = main_re.search(fname)
							if m != null:
								# GDExtension main DLL for this addon. Take only
								# the matching build config; skip the other one.
								if m.get_string("cfg") == this_config:
									var dst_dir: String = staging_dir.path_join("addons").path_join(addon_name).path_join("bin")
									var mk_err: int = DirAccess.make_dir_recursive_absolute(dst_dir)
									if mk_err != OK:
										push_error("GDK Export: Failed to create %s (err %d)" % [dst_dir, mk_err])
										return mk_err
									var copy_err: int = DirAccess.copy_absolute(src, dst_dir.path_join(fname))
									if copy_err != OK:
										push_error("GDK Export: Failed to copy %s -> %s (err %d)" % [src, dst_dir, copy_err])
										return copy_err
									main_copied += 1
								# else: opposite config; skip silently
							else:
								# Support DLL — staging root, next to .exe.
								var dst: String = staging_dir.path_join(fname)
								if not FileAccess.file_exists(dst):
									var copy_err2: int = DirAccess.copy_absolute(src, dst)
									if copy_err2 != OK:
										push_warning("GDK Export: Failed to copy support DLL %s (err %d)" % [src, copy_err2])
									else:
										support_copied += 1
						fname = bin.get_next()
					bin.list_dir_end()
		addon_name = addons.get_next()
	addons.list_dir_end()

	print("[GDK Export] Copied %d GDExtension main DLL(s), %d support DLL(s)" % [main_copied, support_copied])
	return OK

func _export_pck(p_preset: EditorExportPreset, p_debug: bool, p_path: String, p_flags: int) -> int:
	return export_pack(p_preset, p_debug, p_path, p_flags)

func _rmdir_recursive(path: String) -> void:
	var da := DirAccess.open(path)
	if da == null:
		return
	da.list_dir_begin()
	var entry := da.get_next()
	while entry != "":
		var full := path.path_join(entry)
		if da.current_is_dir():
			_rmdir_recursive(full)
		else:
			da.remove(entry)
		entry = da.get_next()
	da.list_dir_end()
	DirAccess.remove_absolute(path)

# Export option builder helper — flat dict format for EditorExportPlatformExtension
func _opt(p_name: String, type: int, default_value: Variant = null,
		hint: int = PROPERTY_HINT_NONE, hint_string: String = "",
		required: bool = false) -> Dictionary:
	var d := {
		"name": p_name,
		"type": type,
		"hint": hint,
		"hint_string": hint_string,
		"default_value": default_value,
		"update_visibility": p_name.begins_with("dev/"),
	}
	if required:
		d["required"] = true
	return d
