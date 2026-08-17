@tool
extends EditorExportPlatformExtension
## XBOX on PC export platform — packages Godot projects as MSIXVC for Xbox app.

const PLATFORM_NAME := "XBOX on PC"
const OS_NAME := "Windows"

# Max number of captured tool-output lines to embed in the export dialog error
# summary. The full output is still printed to the editor Output panel.
const _MAX_SUMMARY_OUTPUT_LINES := 12

# Project Setting (registered by the godot_gdk C++ extension) naming the
# directory that holds MicrosoftGame.config and its logo assets. Export reads
# the config from here but always stages it to the package root next to the
# .exe. Defaults to the project root so existing projects keep working.
const SETTING_GAME_CONFIG_DIR := "gdk/packaging/game_config_dir"
const DEFAULT_GAME_CONFIG_DIR := "res://"

# The only architecture this platform packages for. A GDK title on PC is a
# plain x86_64 Win32 application.
const TARGET_ARCH := "x86_64"

# The engine's own Windows export platform. The .exe and .pck are produced by
# this class rather than reimplemented here — see the delegation section below.
const WINDOWS_PLATFORM_CLASS := "EditorExportPlatformWindows"

# Category shown beside every message this platform adds to the export dialog's
# message log.
const MESSAGE_CATEGORY := "GDK Export"

## True only while the delegated Windows export is running.
##
## The delegated export runs as [code]EditorExportPlatformWindows[/code], so
## this platform's [method _get_platform_features] never runs and the
## [code]gdk[/code] feature tag would silently disappear from every packaged
## build. [code]gdk_export_features_plugin.gd[/code] watches this flag and
## re-adds the tag for the duration of the export. Static because the plugin
## has no reference to the platform instance.
static var exporting_for_gdk := false

# GDK tool paths (resolved on init)
var _gdk_root := ""
var _makepkg := ""
var _wdapp := ""
var _gdk_found := false
# Guards one-shot GDK detection. Detection is lazy so the platform works even
# when the engine never calls _initialize(): Godot only began calling
# EditorExportPlatform::initialize() from add_export_platform() in 4.6, so on
# Godot 4.5.x _initialize() never fires and detection must be triggered on
# demand from the export-configuration/validation entry points instead.
var _detected := false

func _initialize() -> void:
	_ensure_detected()

# Runs GDK detection exactly once, regardless of entry point. Called eagerly
# from _initialize() on engines that invoke it (4.6+) and lazily from the
# export/validation callbacks on engines that don't (4.5.x). The guard also
# keeps the export dialog — which polls _has_valid_export_configuration()
# repeatedly — from re-scanning the filesystem and re-emitting warnings.
func _ensure_detected() -> void:
	if _detected:
		return
	_detected = true
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

## Feature tags every export from this platform carries, regardless of preset.
##
## A GDK title on PC is an ordinary Win32 application: it runs the stock Godot
## Windows export template on a Windows desktop. This list therefore mirrors
## [code]EditorExportPlatformPC::get_platform_features()[/code] ("pc" plus the
## lowercased OS name) and adds exactly one tag of its own, [code]gdk[/code], so
## a project can branch on being packaged for the GDK.
##
## [code]xbox[/code] and [code]d3d12[/code] used to be reported here and were
## wrong: Godot defines no such platform tags, so the *same* project packaged
## through [code]gdkpkg[/code] (which drives the built-in Windows Desktop
## preset) resolved [method OS.has_feature] checks, [ProjectSettings]
## [code]setting.<tag>[/code] overrides, and [code].import[/code] remaps
## differently than it did here. That divergence is the same failure mode as
## issue #144 — a feature tag silently changing which resources reach the
## package — so the invented tags are gone. The architecture tag is supplied by
## [method _get_preset_features], matching where Godot puts it.
##
## Note that under delegation the .pck is written by the built-in Windows
## platform, so it is *that* platform's identical [code]pc[/code] /
## [code]windows[/code] list Godot actually consults; [code]gdk[/code] is
## restored on top by [code]gdk_export_features_plugin.gd[/code]. This list
## still governs everything the editor asks this platform directly.
func _get_platform_features() -> PackedStringArray:
	return PackedStringArray(["pc", "windows", "gdk"])

## Feature tags this preset exports with. These are **not** cosmetic: Godot
## matches them against the [code]path.<feature>[/code] keys in every imported
## resource's [code].import[/code] file and **erases** any remap whose feature
## is absent, so a VRAM-compressed texture whose only variant is
## [code]path.s3tc[/code] is dropped from the .pck entirely. The game then dies
## at load with [i]"Can't load dependency: res://…"[/i] and the .pck comes out a
## fraction of its real size (issue #144).
##
## The texture-format tags therefore have to be derived from the preset exactly
## the way [code]EditorExportPlatformPC[/code] derives them, or this platform
## silently ships a package missing every VRAM-compressed texture.
##
## Since the .pck is now written by the built-in Windows platform (which derives
## the very same tags from the very same preset options), this is belt and
## braces — but it keeps what the editor reports for this platform honest, and
## it is what the option table is mirrored against.
func _get_preset_features(p_preset: EditorExportPreset) -> PackedStringArray:
	var features := PackedStringArray(["windows", "gdk", "x86_64"])
	features.append_array(_texture_format_features(
		_preset_bool(p_preset, "texture_format/s3tc_bptc", true),
		_preset_bool(p_preset, "texture_format/etc2_astc", false)))
	return features

## Texture-format feature tags for the two preset toggles, mirroring
## [code]EditorExportPlatformPC::get_preset_features()[/code]. Split out from
## [method _get_preset_features] so it can be exercised without an editor.
static func _texture_format_features(p_s3tc_bptc: bool, p_etc2_astc: bool) -> PackedStringArray:
	var features := PackedStringArray()
	if p_s3tc_bptc:
		features.append("s3tc")
		features.append("bptc")
	if p_etc2_astc:
		features.append("etc2")
		features.append("astc")
	return features

## Reads a boolean export option, falling back to [param p_fallback] when the
## preset predates the option (an [code]export_presets.cfg[/code] written before
## it existed) or has no value for it.
static func _preset_bool(p_preset: EditorExportPreset, p_name: String, p_fallback: bool) -> bool:
	return bool(_preset_value(p_preset, p_name, p_fallback))

## Reads a string export option, trimmed, falling back to [param p_fallback].
static func _preset_string(p_preset: EditorExportPreset, p_name: String, p_fallback: String = "") -> String:
	return str(_preset_value(p_preset, p_name, p_fallback)).strip_edges()

static func _preset_value(p_preset: EditorExportPreset, p_name: String, p_fallback: Variant) -> Variant:
	if p_preset == null or not p_preset.has(p_name):
		return p_fallback
	var value: Variant = p_preset.get(p_name)
	if value == null:
		return p_fallback
	return value

## Every export option the built-in Windows exporter reads, plus this
## platform's own GDK options.
##
## [b]This list is load-bearing, not cosmetic.[/b] [method _export_project]
## hands this preset straight to
## [code]EditorExportPlatformWindows::export_project()[/code], which reads its
## options by name. An option that is missing here reads back as [code]null[/code]
## inside the built-in exporter, and the failure is neither obvious nor local —
## omitting [code]custom_template/debug[/code], for example, sends it down the
## custom-template branch and aborts the export with
## [i]"Mismatching custom export template executable architecture: found
## 'invalid'"[/i].
##
## The names and defaults mirror
## [code]EditorExportPlatformWindows::get_export_options()[/code] and its
## [code]EditorExportPlatformPC[/code] base. They are identical on Godot 4.5 and
## 4.6 (31 Windows options + 7 PC options, verified by diffing both releases);
## [code]test_export_platform_errors.gd[/code] pins the set against a live
## transient preset so a future engine version adding one fails a test instead
## of an export.
func _get_export_options() -> Array[Dictionary]:
	return export_option_list()


## The option table itself. Static so the test suite can pin it without an
## editor — [EditorExportPlatformExtension] cannot be instantiated headlessly.
static func export_option_list() -> Array[Dictionary]:
	return [
		# ── Custom templates (EditorExportPlatformPC) ──
		# A custom engine build is the standard way to ship an engine patch.
		_opt("custom_template/debug", TYPE_STRING, "",
			PROPERTY_HINT_GLOBAL_FILE, "*.exe"),
		_opt("custom_template/release", TYPE_STRING, "",
			PROPERTY_HINT_GLOBAL_FILE, "*.exe"),

		# ── Binary format / console wrapper (EditorExportPlatformPC) ──
		_opt("debug/export_console_wrapper", TYPE_INT, 1,
			PROPERTY_HINT_ENUM, "No,Debug Only,Debug and Release"),
		_opt("binary_format/embed_pck", TYPE_BOOL, false,
			PROPERTY_HINT_NONE,
			"Embed the .pck inside the .exe instead of shipping it alongside."),

		# ── Texture Format (EditorExportPlatformPC) ──
		# These drive _get_preset_features(), which decides which
		# `path.<feature>` remaps survive into the .pck (issue #144).
		_opt("texture_format/s3tc_bptc", TYPE_BOOL, true,
			PROPERTY_HINT_NONE,
			"Export S3TC/BPTC-compressed textures. Required on desktop GPUs; turning this off drops every VRAM-compressed texture from the package."),
		_opt("texture_format/etc2_astc", TYPE_BOOL, false,
			PROPERTY_HINT_NONE,
			"Export ETC2/ASTC-compressed textures. Only needed for projects that also import mobile texture variants."),

		_opt("shader_baker/enabled", TYPE_BOOL, false),

		# ── Architecture ──
		# Godot's Windows exporter offers x86_64/x86_32/arm64; the GDK on PC
		# targets x86_64 only, so the choice is pinned rather than removed —
		# the built-in exporter still reads this name.
		_opt("binary_format/architecture", TYPE_STRING, TARGET_ARCH,
			PROPERTY_HINT_ENUM, TARGET_ARCH),

		# ── Code signing ──
		# Authenticode signing of the staged .exe, handled entirely by the
		# built-in exporter via the `export/windows/signtool` editor setting.
		# Independent of, and applied before, MSIXVC packaging.
		_opt("codesign/enable", TYPE_BOOL, false),
		_opt("codesign/identity_type", TYPE_INT, 0,
			PROPERTY_HINT_ENUM,
			"Select automatically,Use PKCS12 file (specify *.PFX/*.P12 file),Use certificate store (specify SHA-1 hash)"),
		_opt("codesign/identity", TYPE_STRING, "",
			PROPERTY_HINT_GLOBAL_FILE, "*.pfx,*.p12"),
		_opt("codesign/password", TYPE_STRING, "", PROPERTY_HINT_PASSWORD),
		_opt("codesign/timestamp", TYPE_BOOL, true),
		_opt("codesign/timestamp_server_url", TYPE_STRING, ""),
		_opt("codesign/digest_algorithm", TYPE_INT, 1,
			PROPERTY_HINT_ENUM, "SHA1,SHA256"),
		_opt("codesign/description", TYPE_STRING, ""),
		_opt("codesign/custom_options", TYPE_PACKED_STRING_ARRAY, PackedStringArray()),

		# ── Application (executable resources) ──
		# Stamped into the staged .exe by the built-in exporter's own PE
		# resource writer. Godot dropped its rcedit dependency in 4.5, so this
		# needs no external tool on any version this addon supports.
		_opt("application/modify_resources", TYPE_BOOL, true,
			PROPERTY_HINT_NONE,
			"Apply the project icon and version info to the staged .exe."),
		_opt("application/icon", TYPE_STRING, "",
			PROPERTY_HINT_FILE, "*.ico,*.png,*.webp,*.svg"),
		_opt("application/console_wrapper_icon", TYPE_STRING, "",
			PROPERTY_HINT_FILE, "*.ico,*.png,*.webp,*.svg"),
		_opt("application/icon_interpolation", TYPE_INT, 4,
			PROPERTY_HINT_ENUM, "Nearest neighbor,Bilinear,Cubic,Trilinear,Lanczos"),
		_opt("application/file_version", TYPE_STRING, "",
			PROPERTY_HINT_PLACEHOLDER_TEXT, "Leave empty to use project version"),
		_opt("application/product_version", TYPE_STRING, "",
			PROPERTY_HINT_PLACEHOLDER_TEXT, "Leave empty to use project version"),
		_opt("application/company_name", TYPE_STRING, "",
			PROPERTY_HINT_PLACEHOLDER_TEXT, "Company Name"),
		_opt("application/product_name", TYPE_STRING, "",
			PROPERTY_HINT_PLACEHOLDER_TEXT, "Game Name"),
		_opt("application/file_description", TYPE_STRING, ""),
		_opt("application/copyright", TYPE_STRING, ""),
		_opt("application/trademarks", TYPE_STRING, ""),

		# ── Renderer redistributables ──
		# The built-in exporter stages the D3D12 Agility SDK and PIX runtime
		# from beside the export template. Nothing Xbox-specific: this is the
		# plain Win32 Agility SDK redistribution rule.
		_opt("application/export_angle", TYPE_INT, 0,
			PROPERTY_HINT_ENUM, "Auto,Yes,No"),
		_opt("application/export_d3d12", TYPE_INT, 0,
			PROPERTY_HINT_ENUM, "Auto,Yes,No"),
		_opt("application/d3d12_agility_sdk_multiarch", TYPE_BOOL, true),

		# ── SSH remote deploy ──
		# Declared because the built-in exporter reads them. This platform has
		# no remote-run path of its own, so they are inert here — but a missing
		# name reads back as null inside the built-in exporter, and the whole
		# point of mirroring is that "inert" and "absent" are not the same
		# thing. Defaults match Godot's so the fields look identical to a
		# `Windows Desktop` preset.
		_opt("ssh_remote_deploy/enabled", TYPE_BOOL, false),
		_opt("ssh_remote_deploy/host", TYPE_STRING, "user@host_ip"),
		_opt("ssh_remote_deploy/port", TYPE_STRING, "22"),
		_opt("ssh_remote_deploy/extra_args_ssh", TYPE_STRING, "",
			PROPERTY_HINT_MULTILINE_TEXT),
		_opt("ssh_remote_deploy/extra_args_scp", TYPE_STRING, "",
			PROPERTY_HINT_MULTILINE_TEXT),
		_opt("ssh_remote_deploy/run_script", TYPE_STRING, "",
			PROPERTY_HINT_MULTILINE_TEXT),
		_opt("ssh_remote_deploy/cleanup_script", TYPE_STRING, "",
			PROPERTY_HINT_MULTILINE_TEXT),

		# ── Packaging (GDK) ──
		_opt("packaging/ekb_file", TYPE_STRING, "",
			PROPERTY_HINT_GLOBAL_FILE, "*.ekb",
			false),

		# ── Dev Iteration (GDK) ──
		_opt("dev/register_loose", TYPE_BOOL, false,
			PROPERTY_HINT_NONE,
			"Skip MSIXVC packaging. Instead, register the loose staging folder via wdapp for fast dev iteration. Enable for inner-loop testing; leave off to produce a real .msixvc package."),
		_opt("dev/sandbox_id", TYPE_STRING, "RETAIL",
			PROPERTY_HINT_NONE, "Xbox Live sandbox ID (used by wdapp register)"),
	]

## The option names this platform declares, for the drift test.
static func declared_option_names() -> PackedStringArray:
	var names := PackedStringArray()
	for entry: Dictionary in export_option_list():
		names.append(str(entry["name"]))
	return names

## Which options the export dialog shows, mirroring
## [code]EditorExportPlatformWindows::get_export_option_visibility()[/code] for
## the options borrowed from it and adding this platform's own rules.
##
## Options this hides are still *declared* and still read by the built-in
## exporter — hiding only affects the inspector.
func _get_export_option_visibility(p_preset: EditorExportPreset, p_option: String) -> bool:
	# Collapse the executable-resource fields when resource modification is off.
	# These `application/` options describe runtime DLL staging rather than
	# executable resources, so — as in Godot — they stay visible.
	const NON_RESOURCE_APPLICATION_OPTIONS: Array[String] = [
		"application/export_angle",
		"application/export_d3d12",
		"application/d3d12_agility_sdk_multiarch",
	]
	if p_option.begins_with("application/") and p_option not in NON_RESOURCE_APPLICATION_OPTIONS \
			and p_option != "application/modify_resources":
		if not _preset_bool(p_preset, "application/modify_resources", true):
			return false

	# Collapse the code-signing fields behind their own toggle.
	if p_option.begins_with("codesign/") and p_option != "codesign/enable":
		if not _preset_bool(p_preset, "codesign/enable", false):
			return false

	# Same for the SSH remote-deploy fields, which this platform never acts on.
	if p_option.begins_with("ssh_remote_deploy/") and p_option != "ssh_remote_deploy/enabled":
		if not _preset_bool(p_preset, "ssh_remote_deploy/enabled", false):
			return false

	# Hide packaging options when using loose registration; hide sandbox when
	# producing an MSIXVC (sandbox only applies to `wdapp register`).
	var register_loose: bool = _preset_bool(p_preset, "dev/register_loose", false)
	if p_option == "packaging/ekb_file":
		return not register_loose
	if p_option == "dev/sandbox_id":
		return register_loose
	return true

func _get_export_option_warning(p_preset: EditorExportPreset, p_option: StringName) -> String:
	return ""

# ── Validation messages ─────────────────────────────────────────
#
# Validation must never fail silently: Godot greys out "Export Project…" /
# "Export All…" purely on the boolean the callbacks below return, and shows
# nothing at all unless the platform reports *why* through set_config_error().
# Every `return false` therefore records an actionable reason first, so the
# dialog never presents a dead "Export Project" button with no explanation.

static func _gdk_missing_message() -> String:
	return ("Microsoft GDK was not found on this machine. " +
		"Install it with `winget install Microsoft.Gaming.GDK`, then restart the Godot editor.")

static func _missing_game_config_message(config_path: String) -> String:
	return ("MicrosoftGame.config was not found at %s. " % config_path +
		"Run GDK \u25b8 Create Game Config\u2026 from the editor menu, or copy " +
		"MicrosoftGame.config.template next to it and fill in your Partner Center values. " +
		"Set the gdk/packaging/game_config_dir Project Setting to look elsewhere.")

static func _missing_template_message(p_debug: bool, p_dotnet: bool = false) -> String:
	var config_label: String = "debug" if p_debug else "release"
	var godot_ver: String = str(Engine.get_version_info().get("string", ""))
	var flavor: String = ".NET/Mono " if p_dotnet else ""
	return ("No Windows %s %sexport template was found for Godot %s. " % [config_label, flavor, godot_ver] +
		"Install it via Editor \u25b8 Manage Export Templates\u2026 (matching your exact Godot " +
		"version%s). " % (" \u2014 the .NET template package, not the standard one" if p_dotnet else "") +
		"The .exe and .pck are produced by Godot's own Windows exporter, so a real " +
		"template is required even for a loose dev-register build.")

# A .NET project needs the .NET/Mono template package specifically, not the
# standard one, so call that out rather than sending the user back to the same
# "install export templates" instruction that just failed them.
static func _dotnet_requires_template_message(p_debug: bool) -> String:
	return (_missing_template_message(p_debug, true) + "\n" +
		"This project contains C# code (dotnet/project/assembly_name is set), so the standard " +
		"Windows template will not do \u2014 the game would fail at launch with " +
		"\".NET assemblies not found\".")

# A custom template is an explicit, deliberate choice by the user, so a broken
# path is reported as itself rather than as a missing installed template — and
# it is never substituted with the editor binary, even for a loose build.
static func _missing_custom_template_message(p_debug: bool, p_path: String) -> String:
	var config_label: String = "debug" if p_debug else "release"
	return ("The custom %s template configured on this preset does not exist:\n  %s\n" % [config_label, p_path] +
		"Point custom_template/%s at a valid Godot Windows export template, " % config_label +
		"or clear it to use the templates installed via Editor \u25b8 Manage Export Templates\u2026.")

func _has_valid_export_configuration(p_preset: EditorExportPreset, p_debug: bool) -> bool:
	_ensure_detected()
	set_config_missing_templates(false)
	var errors: PackedStringArray = PackedStringArray()

	if not _gdk_found:
		errors.append(_gdk_missing_message())

	# MicrosoftGame.config is the source of truth for identity / shell visuals
	# and is authored via the godot_gdk_editortools addon's "Create Game Config".
	# If it's missing the export pipeline cannot proceed. Its directory is
	# configurable via the gdk/packaging/game_config_dir Project Setting.
	if not FileAccess.file_exists(_game_config_src()):
		errors.append(_missing_game_config_message(_game_config_src()))

	# The .exe and .pck come from the built-in Windows exporter, which cannot
	# run without a real export template — there is no editor-binary stand-in
	# any more, so this is a hard blocker for loose dev-register builds too
	# (issue #134). Report it as a missing-templates condition so the dialog
	# offers the "Manage Export Templates" shortcut.
	var custom_template: String = _custom_template_path(p_preset, p_debug)
	if custom_template != "" and not FileAccess.file_exists(custom_template):
		errors.append(_missing_custom_template_message(p_debug, custom_template))
	elif _find_windows_template(p_preset, p_debug) == "":
		set_config_missing_templates(true)
		if _is_dotnet_project():
			errors.append(_dotnet_requires_template_message(p_debug))
		else:
			errors.append(_missing_template_message(p_debug))

	set_config_error("\n".join(errors))
	return errors.is_empty()

func _has_valid_project_configuration(p_preset: EditorExportPreset) -> bool:
	_ensure_detected()
	if not _gdk_found:
		set_config_error(_gdk_missing_message())
		return false
	set_config_error("")
	return true

func _can_export(p_preset: EditorExportPreset, p_debug: bool) -> bool:
	return _has_valid_export_configuration(p_preset, p_debug)

func _export_project(p_preset: EditorExportPreset, p_debug: bool, p_path: String, p_flags: int) -> int:
	_ensure_detected()
	# Drop messages from any previous run so the dialog's log reflects only this
	# export, matching how the built-in exporters treat their message list.
	clear_messages()
	if not _gdk_found:
		_report_error("GDK not found. Install via: winget install Microsoft.Gaming.GDK")
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
			_report_error("GDK Export: Cannot create output directory: %s (err %d)" % [out_dir, mk_err])
			return ERR_FILE_BAD_PATH

	if DirAccess.dir_exists_absolute(staging_dir):
		_rmdir_recursive(staging_dir)
	var stage_err: int = DirAccess.make_dir_recursive_absolute(staging_dir)
	if stage_err != OK:
		_report_error("GDK Export: Cannot create staging directory: %s (err %d)" % [staging_dir, stage_err])
		return ERR_FILE_BAD_PATH

	# Drop a .gdignore so Godot's resource importer doesn't try to import the
	# staged .exe / .pck / .config / logo files when the staging directory
	# lives inside the project tree (e.g. `<project>/builds/_gdk_staging/`).
	var gdignore := FileAccess.open(staging_dir.path_join(".gdignore"), FileAccess.WRITE)
	if gdignore != null:
		gdignore.close()

	# ── Step 2: Produce the .exe and .pck via the built-in Windows exporter ──
	# Derive the .exe name from the project's MicrosoftGame.config so the
	# staged executable matches what `<Executable Name="...">` declares. The
	# config (authored by the godot_gdk_editortools addon's "Create Game Config"
	# flow) is the single source of truth for identity, shell visuals, and the
	# executable name — no preset duplication.
	var exe_name: String = _read_exe_name_from_project_config()
	if exe_name == "":
		_report_error(
			"GDK Export: MicrosoftGame.config not found or missing <Executable Name=...> at %s.\n" % _game_config_src() +
			"  Open the project in the editor and run GDK ▸ Create Game Config,\n" +
			"  or place a valid MicrosoftGame.config in the configured game-config\n" +
			"  directory (gdk/packaging/game_config_dir).")
		return ERR_FILE_NOT_FOUND
	var exe_path: String = staging_dir.path_join(exe_name)

	var use_loose: bool = _preset_bool(p_preset, "dev/register_loose", false)
	var delegate_err: int = _run_windows_export(p_preset, p_debug, exe_path, p_flags)
	if delegate_err != OK:
		return delegate_err

	# ── Step 3: Copy addon GDExtension main DLLs + support runtime DLLs ──
	# The built-in exporter already places the GDExtension libraries beside the
	# .exe (Godot's own loader fallback), but the support runtimes that sit in
	# `addons/<name>/bin/` without being declared as shared objects
	# (libHttpClient, Microsoft.Xbox.Services.C.Thunks, Party, …) are invisible
	# to it, and the `res://addons/...` path inside each .gdextension still has
	# to resolve on disk.
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
	if use_loose:
		return _wdapp_register(staging_dir)
	else:
		return _makepkg_pack(staging_dir, abs_p_path, p_preset)

# ── Delegation to the built-in Windows exporter ──────────────────
#
# This platform used to reimplement the desktop export pipeline: copy a
# template, stamp its icon, stage the D3D12 redistributables, write the .pck,
# place the shared objects. None of that is GDK-specific — a GDK title on PC is
# an ordinary Win32 application — and every one of those steps was a chance to
# drift from what a plain `Windows Desktop` export produces. Issues #134 and
# #144 were both instances of that drift.
#
# So the .exe and .pck are produced by `EditorExportPlatformWindows` itself,
# handed this platform's own preset, and the GDK-specific packaging is layered
# on top of the result. Template resolution, PE icon/version stamping, the
# console wrapper, embed_pck, code signing, the D3D12 Agility SDK and PIX
# runtimes, ANGLE, shader baking, and export-plugin shared objects (a C#
# project's assemblies among them) all come from the engine and stay correct on
# their own.
#
# Verified by exporting the same project both ways: the .exe and .pck this
# produces are byte-identical to a built-in `Windows Desktop` export.

## Instantiates the engine's own Windows export platform.
##
## This is an editor-only class, but every caller here already runs inside an
## export session. Returns null if the class is unavailable, which would mean a
## Godot build without the Windows exporter compiled in.
static func _new_windows_platform() -> Object:
	if not ClassDB.class_exists(WINDOWS_PLATFORM_CLASS):
		return null
	return ClassDB.instantiate(WINDOWS_PLATFORM_CLASS)

## Runs the built-in Windows export for [param p_preset] into [param exe_path].
##
## [param p_preset] belongs to *this* platform, not to the Windows one. That is
## deliberate and is what keeps the two in sync: the user's export filters,
## encryption settings, script export mode, custom features and every mirrored
## option are read straight off the preset they actually edited, with no
## copying step to fall out of date. It works because
## [method _get_export_options] declares every option name the built-in
## exporter reads.
func _run_windows_export(p_preset: EditorExportPreset, p_debug: bool, exe_path: String, p_flags: int) -> int:
	var windows: Object = _new_windows_platform()
	if windows == null:
		_report_error("GDK Export: This Godot build has no Windows exporter (%s is unavailable), " % WINDOWS_PLATFORM_CLASS +
			"so the .exe and .pck cannot be produced.")
		return ERR_UNAVAILABLE

	# Tag the export so the companion EditorExportPlugin can re-add the `gdk`
	# feature: the delegated export runs as the Windows platform, so this
	# platform's own _get_platform_features() is never consulted.
	exporting_for_gdk = true
	# `windows` is a bare instance rather than the registered platform, so its
	# message log starts empty and everything in it belongs to this run.
	var err: int = windows.export_project(p_preset, p_debug, exe_path, p_flags)
	exporting_for_gdk = false

	_forward_messages(windows)

	if err != OK:
		_report_error("GDK Export: The built-in Windows export failed (err %d), so no package was produced. " % err +
			"The messages above come from Godot's own exporter.")
		return err
	if not FileAccess.file_exists(exe_path):
		_report_error("GDK Export: The built-in Windows export reported success but produced no executable at %s." % exe_path)
		return ERR_FILE_NOT_FOUND
	print("[GDK Export] Windows export produced: ", exe_path)
	return OK

## Copies the delegated exporter's message log into this platform's own, so the
## export dialog attributes them to a single export and nothing is lost with
## the temporary platform instance.
func _forward_messages(p_platform: Object) -> void:
	for i: int in range(p_platform.get_message_count()):
		add_message(p_platform.get_message_type(i),
			p_platform.get_message_category(i),
			p_platform.get_message_text(i))

# ── MicrosoftGame.config staging ─────────────────────────────────

# Returns the configured game-config directory as a filesystem-absolute path.
# The gdk/packaging/game_config_dir Project Setting names the directory holding
# MicrosoftGame.config + logos and defaults to the project root (res://).
# Bare/relative values are treated as project-relative; trailing slashes are
# trimmed except on root paths (res://, user://, drive roots).
func _game_config_dir() -> String:
	var raw: String = str(ProjectSettings.get_setting(
		SETTING_GAME_CONFIG_DIR, DEFAULT_GAME_CONFIG_DIR)).strip_edges()
	if raw.is_empty():
		raw = DEFAULT_GAME_CONFIG_DIR
	raw = raw.replace("\\", "/")
	if not (raw.begins_with("res://") or raw.begins_with("user://") or raw.is_absolute_path()):
		raw = "res://".path_join(raw)
	var min_len: int = 0
	if raw.begins_with("res://"):
		min_len = "res://".length()
	elif raw.begins_with("user://"):
		min_len = "user://".length()
	elif raw.is_absolute_path():
		min_len = 1
		if raw.length() == 3 and raw.substr(1, 1) == ":":
			min_len = 3
	while raw.length() > min_len and raw.ends_with("/"):
		raw = raw.substr(0, raw.length() - 1)
	if raw.begins_with("res://") or raw.begins_with("user://"):
		return ProjectSettings.globalize_path(raw)
	return raw

# Returns the filesystem path to the source MicrosoftGame.config in the
# configured game-config directory.
func _game_config_src() -> String:
	return _game_config_dir().path_join("MicrosoftGame.config")

# Reads `<Executable Name="...">` from the project's MicrosoftGame.config.
# Returns "" if the config is missing or has no Executable element. Centralized
# here so `_export_project` (deriving the staged .exe name) and editor checks
# can share one parse.
func _read_exe_name_from_project_config() -> String:
	var src: String = _game_config_src()
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
# The config itself is authored by the `godot_gdk_editortools` addon's "Create
# Game Config" flow (or by the developer directly via GameConfigEditor) —
# never generated at export time.
func _stage_microsoft_game_config(staging_dir: String) -> int:
	var src: String = _game_config_src()
	if not FileAccess.file_exists(src):
		_report_error(
			"GDK Export: MicrosoftGame.config not found at %s.\n" % src +
			"  Open the project in the editor and run GDK ▸ Create Game Config,\n" +
			"  or place a MicrosoftGame.config (and its logo PNGs) in the configured\n" +
			"  game-config directory (gdk/packaging/game_config_dir).")
		return ERR_FILE_NOT_FOUND

	var content: String = FileAccess.get_file_as_string(src)
	if content == "":
		_report_error("GDK Export: Failed to read MicrosoftGame.config at %s" % src)
		return ERR_FILE_CANT_READ
	content = _inject_target_device_family(content)

	var dest: String = staging_dir.path_join("MicrosoftGame.config")
	var f := FileAccess.open(dest, FileAccess.WRITE)
	if f == null:
		_report_error("GDK Export: Cannot write MicrosoftGame.config to %s" % dest)
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
# references, then copies each one from the configured game-config directory
# into the staging dir at the same relative path (so they land next to the
# .exe/config at the package root). Sources are tried in order:
# <config-dir>/<rel-from-config>, <config-dir>/storelogos/<filename>,
# <config-dir>/<filename>. Missing logos surface as warnings — the subsequent
# wdapp/makepkg step will then fail with a specific 0x80070002 pointing at the
# offending file.
func _stage_logos(staging_dir: String) -> void:
	var config_dir: String = _game_config_dir()
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
			config_dir.path_join(rel),
			config_dir.path_join("storelogos").path_join(filename),
			config_dir.path_join(filename),
		])
		var src: String = ""
		for c: String in candidates:
			if FileAccess.file_exists(c):
				src = c
				break
		if src == "":
			_report_warning(
				"GDK Export: %s logo not found — expected at %s. " % [attr, config_dir.path_join(rel)] +
				"Run GDK ▸ Create Game Config to generate placeholders.")
			continue

		var dest: String = staging_dir.path_join(rel)
		DirAccess.make_dir_recursive_absolute(dest.get_base_dir())
		var err: int = DirAccess.copy_absolute(src, dest)
		if err != OK:
			_report_warning("GDK Export: Failed to copy logo %s -> %s (err %d)" % [src, dest, err])
		else:
			print("[GDK Export] Logo staged: %s" % rel)


# ── Diagnostics ─────────────────────────────────────────────────
#
# Export failures reported only through push_error() land in the Output panel,
# which the export dialog does not surface — so an export could fail with the
# dialog showing nothing. Routing them through add_message() as well puts them
# in the dialog's own message log with a severity, exactly as the built-in
# exporters do, while keeping the console output for headless runs.

func _report_error(p_text: String) -> void:
	push_error(p_text)
	add_message(EditorExportPlatform.EXPORT_MESSAGE_ERROR, MESSAGE_CATEGORY, _strip_category(p_text))

func _report_warning(p_text: String) -> void:
	push_warning(p_text)
	add_message(EditorExportPlatform.EXPORT_MESSAGE_WARNING, MESSAGE_CATEGORY, _strip_category(p_text))

## The dialog renders the category beside the message, so a message that already
## opens with it would read "GDK Export: GDK Export: …". The console output keeps
## the prefix, which is what makes these lines findable in the Output panel.
static func _strip_category(p_text: String) -> String:
	var prefix: String = MESSAGE_CATEGORY + ": "
	return p_text.substr(prefix.length()) if p_text.begins_with(prefix) else p_text

# ── Tool execution ──────────────────────────────────────────────

func _wdapp_register(staging_dir: String) -> int:
	print("[GDK Export] Registering loose folder via wdapp...")
	var global_path: String = ProjectSettings.globalize_path(staging_dir)
	var result: Dictionary = _run_tool_capture(_wdapp, PackedStringArray(["register", global_path]))
	_print_tool_output("wdapp", result)
	if int(result.get("exit_code", -1)) != 0:
		_report_error(_summarize_tool_failure("wdapp register", result))
		return FAILED
	print("[GDK Export] Registered successfully! Launch from Xbox app or Start menu.")
	return OK

func _makepkg_pack(staging_dir: String, output_path: String, p_preset: EditorExportPreset) -> int:
	var global_staging: String = ProjectSettings.globalize_path(staging_dir)
	var global_output: String = ProjectSettings.globalize_path(output_path)

	# Step 1: genmap
	print("[GDK Export] Generating file map...")
	var layout_path: String = global_staging + "\\layout.xml"
	var genmap_result: Dictionary = _run_tool_capture(_makepkg, PackedStringArray([
		"genmap", "/f", layout_path, "/d", global_staging
	]))
	_print_tool_output("makepkg", genmap_result)
	if int(genmap_result.get("exit_code", -1)) != 0:
		_report_error(_summarize_tool_failure("makepkg genmap", genmap_result))
		return FAILED

	# Step 2: pack
	print("[GDK Export] Packing MSIXVC...")
	var pack_args: PackedStringArray = PackedStringArray([
		"pack", "/f", layout_path, "/d", global_staging, "/pd", global_output.get_base_dir()
	])

	# Add EKB file if provided
	var ekb: Variant = p_preset.get("packaging/ekb_file") if p_preset.has("packaging/ekb_file") else ""
	if ekb != "":
		pack_args.append("/lk")
		pack_args.append(ProjectSettings.globalize_path(ekb))

	var pack_result: Dictionary = _run_tool_capture(_makepkg, pack_args)
	_print_tool_output("makepkg", pack_result)
	if int(pack_result.get("exit_code", -1)) != 0:
		_report_error(_summarize_tool_failure("makepkg pack", pack_result))
		return FAILED

	print("[GDK Export] Package created: ", global_output)
	return OK

# ── GDK tool invocation + failure diagnostics ───────────────────

# Runs a GDK CLI tool (makepkg / wdapp) capturing stdout and stderr on
# SEPARATE pipes. These tools split their failure diagnostics across both
# streams (e.g. makepkg `pack` prints "Failed with error (0x...)" to stdout
# but "Mapfile ... does not exist." to stderr; `genmap` prints the
# "error = 0x8007xxxx" line to stderr), so both must be captured to explain a
# failure. Returns { "exit_code": int, "stdout": String, "stderr": String }.
#
# NOTE: the process exit code from these tools is NOT diagnostic — it is a
# small value (often 2 or 3) that never corresponds to the real cause. The
# actionable signal is the HRESULT embedded in the captured text; see
# `_summarize_tool_failure` / `_extract_hresult`.
func _run_tool_capture(exe_path: String, args: PackedStringArray) -> Dictionary:
	if not FileAccess.file_exists(exe_path):
		return {"exit_code": -1, "stdout": "", "stderr": "Tool not found: " + exe_path}

	var pipes: Dictionary = OS.execute_with_pipe(exe_path, args, false)
	if pipes.is_empty():
		return {"exit_code": -1, "stdout": "", "stderr": "Failed to launch: " + exe_path}

	var pid: int = int(pipes.get("pid", -1))
	var stdout_pipe: FileAccess = pipes.get("stdio", null) as FileAccess
	var stderr_pipe: FileAccess = pipes.get("stderr", null) as FileAccess
	var stdout_text: String = ""
	var stderr_text: String = ""

	while pid >= 0 and OS.is_process_running(pid):
		stdout_text += _drain_pipe_text(stdout_pipe)
		stderr_text += _drain_pipe_text(stderr_pipe)
		OS.delay_msec(10)
	stdout_text += _drain_pipe_text(stdout_pipe)
	stderr_text += _drain_pipe_text(stderr_pipe)

	var exit_code: int = OS.get_process_exit_code(pid) if pid >= 0 else -1
	if stdout_pipe != null:
		stdout_pipe.close()
	if stderr_pipe != null:
		stderr_pipe.close()

	return {"exit_code": exit_code, "stdout": stdout_text, "stderr": stderr_text}

static func _drain_pipe_text(pipe: FileAccess) -> String:
	if pipe == null or not pipe.is_open():
		return ""
	var bytes: PackedByteArray = PackedByteArray()
	while true:
		var chunk: PackedByteArray = pipe.get_buffer(4096)
		if chunk.is_empty():
			break
		bytes.append_array(chunk)
	return bytes.get_string_from_utf8()

# Echoes a captured tool result to the editor console, tagging the stream so a
# reader can tell stdout diagnostics from the stderr error line. Lines are
# trimmed because the tools emit CRLF and `split("\n")` would otherwise leave a
# stray `\r` on every line.
static func _print_tool_output(tool_name: String, result: Dictionary) -> void:
	for line: String in str(result.get("stdout", "")).split("\n"):
		var trimmed: String = line.strip_edges()
		if trimmed != "":
			print("[%s] %s" % [tool_name, trimmed])
	for line: String in str(result.get("stderr", "")).split("\n"):
		var trimmed: String = line.strip_edges()
		if trimmed != "":
			print("[%s:err] %s" % [tool_name, trimmed])

# Builds a human-readable failure summary from a captured tool result so the
# editor's export dialog explains WHY the step failed instead of surfacing an
# opaque engine error code. Extracts the HRESULT (the real signal) and echoes
# the TAIL of the captured stdout+stderr (last `_MAX_SUMMARY_OUTPUT_LINES`
# non-empty lines) so the dialog stays readable — the full, untruncated output
# is already sent to the editor Output panel by `_print_tool_output`.
static func _summarize_tool_failure(step_label: String, result: Dictionary) -> String:
	var stdout_text: String = str(result.get("stdout", ""))
	var stderr_text: String = str(result.get("stderr", ""))
	var combined: String = stdout_text
	if stderr_text.strip_edges() != "":
		combined += "\n" + stderr_text

	var lines: Array[String] = []
	lines.append("GDK Export: %s failed." % step_label)

	var hresult: String = _extract_hresult(combined)
	if hresult != "":
		lines.append("  Error %s%s" % [hresult, _describe_hresult(hresult)])

	var detail_lines: Array[String] = []
	for line: String in combined.split("\n"):
		var trimmed: String = line.strip_edges()
		if trimmed != "":
			detail_lines.append(trimmed)

	if detail_lines.is_empty():
		lines.append("  (no output captured; process exit code %d)" % int(result.get("exit_code", -1)))
		return "\n".join(lines)

	var total: int = detail_lines.size()
	if total > _MAX_SUMMARY_OUTPUT_LINES:
		detail_lines = detail_lines.slice(total - _MAX_SUMMARY_OUTPUT_LINES)
		lines.append("  Tool output (last %d of %d lines; see Output panel for full log):" % [_MAX_SUMMARY_OUTPUT_LINES, total])
	else:
		lines.append("  Tool output:")
	for line: String in detail_lines:
		lines.append("    " + line)

	return "\n".join(lines)

# Finds the first failure HRESULT (high-bit-set 8-hex value such as
# 0x8007xxxx) in captured tool output. makepkg/wdapp print this as
# "error = 0x8007xxxx" or "Failed with error (0x8007xxxx)". Returns "" when no
# failure HRESULT is present. A successful 0x00000000 token is intentionally
# ignored so it never masks a real failure code appearing later in the text.
static func _extract_hresult(text: String) -> String:
	var re: RegEx = RegEx.new()
	re.compile("0[xX][0-9A-Fa-f]{8}")
	for m: RegExMatch in re.search_all(text):
		var token: String = m.get_string(0)
		var normalized: String = "0x" + token.substr(2).to_upper()
		if normalized.begins_with("0x8") or normalized.begins_with("0xC"):
			return normalized
	return ""

# Decodes the most common FACILITY_WIN32 HRESULTs makepkg/wdapp surface into a
# short actionable hint. Unknown codes return "" (the raw HRESULT + tool output
# already carry the detail).
static func _describe_hresult(hresult: String) -> String:
	# Normalize to a lowercase "0x" prefix + upper-case hex digits so the match
	# labels below are hit regardless of the caller's casing. (`to_upper()` on
	# the whole string would uppercase the "x" and never match.)
	var key: String = hresult.strip_edges()
	if key.length() > 2:
		key = "0x" + key.substr(2).to_upper()
	match key:
		"0x80070002":
			return " (ERROR_FILE_NOT_FOUND — a referenced file is missing from the staging folder)"
		"0x80070003":
			return " (ERROR_PATH_NOT_FOUND — a referenced path is missing)"
		"0x80070005":
			return " (ERROR_ACCESS_DENIED — a file is locked or the tool needs elevation)"
		"0x80070057":
			return " (E_INVALIDARG — check MicrosoftGame.config, e.g. TargetDeviceFamily)"
		_:
			return ""

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

# True when the running editor is a Godot .NET (Mono) build. Only these builds
# expose CSharpScript, and only they can export a project containing C# code.
static func _is_dotnet_editor() -> bool:
	return ClassDB.class_exists("CSharpScript")

# True when this project actually ships C# code, i.e. Godot resolved a .NET
# assembly for it. Such a project must be staged from a .NET export template:
# the editor binary sends the game looking for `GodotSharp/Api/Debug` next to
# the .exe and aborts with ".NET assemblies not found".
static func _is_dotnet_project() -> bool:
	if not _is_dotnet_editor():
		return false
	return str(ProjectSettings.get_setting("dotnet/project/assembly_name", "")) != ""

## File name of the stock Godot Windows export template for a build config.
## Mirrors [code]EditorExportPlatformWindows::get_template_file_name()[/code].
static func _template_file_name(p_debug: bool) -> String:
	return "windows_%s_%s.exe" % ["debug" if p_debug else "release", TARGET_ARCH]

## Custom template path configured on the preset, or "" when unset.
static func _custom_template_path(p_preset: EditorExportPreset, p_debug: bool) -> String:
	return _preset_string(p_preset,
		"custom_template/debug" if p_debug else "custom_template/release")

## Resolves the executable this export stages from.
##
## A preset-supplied [code]custom_template/*[/code] wins outright — as in
## [code]EditorExportPlatformPC::export_project()[/code] — and when it is set
## but missing this returns "" rather than quietly falling back, so the caller
## can report the user's own broken path instead of an unrelated
## "install export templates" message.
##
## Otherwise the lookup is delegated to the inherited
## [method EditorExportPlatform.find_export_template]. That is deliberately the
## same call the built-in Windows exporter makes: it resolves
## [code]<templates dir>/<VERSION_FULL_CONFIG>/<file>[/code], which handles the
## patch-qualified directory name and the [code].mono[/code] suffix of a .NET
## editor build, and honours self-contained mode and a relocated editor data
## directory — none of which the previous hand-rolled [code]%APPDATA%[/code]
## probe did. It is also stricter: there is no fallback to a
## near-miss version directory, so a 4.6.2 editor never silently packages a
## 4.6 template (the class of mismatch behind issue #134).
##
## The bound method returns a Dictionary of
## [code]{result, path, error_string}[/code] on both Godot 4.5 and 4.6; only the
## path is needed here, and it is empty when the template is missing.
func _find_windows_template(p_preset: EditorExportPreset, p_debug: bool) -> String:
	var custom: String = _custom_template_path(p_preset, p_debug)
	if custom != "":
		return custom if FileAccess.file_exists(custom) else ""
	var found: Dictionary = find_export_template(_template_file_name(p_debug))
	if int(found.get("result", FAILED)) != OK:
		return ""
	return str(found.get("path", ""))

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
		_report_warning("GDK Export: Cannot open addons directory: %s" % addons_dir)
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
										_report_error("GDK Export: Failed to create %s (err %d)" % [dst_dir, mk_err])
										return mk_err
									var copy_err: int = DirAccess.copy_absolute(src, dst_dir.path_join(fname))
									if copy_err != OK:
										_report_error("GDK Export: Failed to copy %s -> %s (err %d)" % [src, dst_dir, copy_err])
										return copy_err
									main_copied += 1
								# else: opposite config; skip silently
							else:
								# Support DLL — staging root, next to .exe.
								var dst: String = staging_dir.path_join(fname)
								if not FileAccess.file_exists(dst):
									var copy_err2: int = DirAccess.copy_absolute(src, dst)
									if copy_err2 != OK:
										_report_warning("GDK Export: Failed to copy support DLL %s (err %d)" % [src, copy_err2])
									else:
										support_copied += 1
						fname = bin.get_next()
					bin.list_dir_end()
		addon_name = addons.get_next()
	addons.list_dir_end()

	print("[GDK Export] Copied %d GDExtension main DLL(s), %d support DLL(s)" % [main_copied, support_copied])

	# A build with zero GDExtension main DLLs staged loads with "GDExtension
	# dynamic library not found" at launch. This happens when the requested
	# config's DLL was never built/synced into addons/*/bin — e.g. exporting
	# release after only a debug build. Fail loudly at export time with the
	# exact fix instead of shipping a broken package (issue #134).
	if main_copied == 0:
		_report_error(_missing_main_dll_message(this_config))
		return ERR_FILE_NOT_FOUND

	return OK

# Actionable diagnostic for the zero-GDExtension-DLL failure mode (issue #134).
static func _missing_main_dll_message(p_config: String) -> String:
	var is_debug: bool = p_config == "debug"
	var configure_preset: String = "default" if is_debug else "default-release"
	var build_preset: String = "debug" if is_debug else "release"
	return (
		"GDK Export: no GDExtension main DLL for the '%s' configuration was found in any addons/*/bin.\n" % p_config +
		"  A '%s' export must stage godot_*.windows.%s.x86_64.dll next to its .gdextension, or the\n" % [p_config, p_config] +
		"  packaged game fails at launch with \"GDExtension dynamic library not found\".\n" +
		"  Build the native addons in this configuration first (each configuration has its own\n" +
		"  configure preset and build tree):\n" +
		"      cmake --preset %s\n" % configure_preset +
		"      cmake --build --preset %s\n" % build_preset +
		"  then re-export. (In the Godot export dialog, \"Export With Debug\" unchecked selects release.)")

static func _copy_dir_recursive(src_dir: String, dest_dir: String) -> int:
	var mk_err: int = DirAccess.make_dir_recursive_absolute(dest_dir)
	if mk_err != OK:
		return mk_err
	var dir: DirAccess = DirAccess.open(src_dir)
	if dir == null:
		return ERR_CANT_OPEN
	dir.list_dir_begin()
	var entry: String = dir.get_next()
	while entry != "":
		var src: String = src_dir.path_join(entry)
		var dest: String = dest_dir.path_join(entry)
		var err: int = OK
		if dir.current_is_dir():
			err = _copy_dir_recursive(src, dest)
		else:
			err = DirAccess.copy_absolute(src, dest)
		if err != OK:
			dir.list_dir_end()
			return err
		entry = dir.get_next()
	dir.list_dir_end()
	return OK

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
static func _opt(p_name: String, type: int, default_value: Variant = null,
		hint: int = PROPERTY_HINT_NONE, hint_string: String = "",
		required: bool = false) -> Dictionary:
	var d := {
		"name": p_name,
		"type": type,
		"hint": hint,
		"hint_string": hint_string,
		"default_value": default_value,
		# Options whose value changes which *other* options are shown must ask
		# the dialog to re-query _get_export_option_visibility().
		"update_visibility": p_name.begins_with("dev/") or p_name == "application/modify_resources",
	}
	if required:
		d["required"] = true
	return d
