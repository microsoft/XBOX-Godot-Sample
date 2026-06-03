@tool
extends ScrollContainer

const ENCRYPT_NONE := 0
const ENCRYPT_LICENSE := 1
const ENCRYPT_CUSTOM_KEY := 2

var _coordinator

var source_dir_edit: LineEdit
var map_file_edit: LineEdit
var auto_genmap_check: CheckBox
var output_dir_edit: LineEdit
var content_id_edit: LineEdit
var product_id_edit: LineEdit
var encrypt_option: OptionButton
var encrypt_key_edit: LineEdit
var updcompat_option: OptionButton
var preset_selector: OptionButton
var clean_build_check: CheckBox
var export_btn: Button
var register_btn: Button
var export_register_btn: Button
var export_package_btn: Button
var export_status_label: Label
var genmap_btn: Button
var validate_btn: Button
var pack_btn: Button


func setup(coordinator) -> void:
	_coordinator = coordinator
	horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	size_flags_vertical = SIZE_EXPAND_FILL

	var root := VBoxContainer.new()
	root.size_flags_horizontal = SIZE_EXPAND_FILL
	add_child(root)

	_coordinator._add_section_header(root, "Export Presets & Actions")

	var preset_row := HBoxContainer.new()
	root.add_child(preset_row)

	var preset_label := Label.new()
	preset_label.text = "Export Preset"
	preset_label.custom_minimum_size.x = 130
	preset_row.add_child(preset_label)

	preset_selector = OptionButton.new()
	preset_selector.size_flags_horizontal = SIZE_EXPAND_FILL
	preset_selector.tooltip_text = "Select a Windows Desktop export preset"
	preset_row.add_child(preset_selector)

	clean_build_check = CheckBox.new()
	clean_build_check.text = "Clean Build/ folder before export"
	clean_build_check.button_pressed = false
	root.add_child(clean_build_check)

	var export_btn_row := HBoxContainer.new()
	root.add_child(export_btn_row)

	export_btn = Button.new()
	export_btn.text = "Export Build"
	export_btn.tooltip_text = "Export to Build/ folder using the selected preset"
	export_btn.pressed.connect(on_export)
	export_btn_row.add_child(export_btn)

	export_register_btn = Button.new()
	export_register_btn.text = "Export + Register"
	export_register_btn.tooltip_text = "Export then register for immediate testing"
	export_register_btn.pressed.connect(on_export_and_register)
	export_btn_row.add_child(export_register_btn)

	register_btn = Button.new()
	register_btn.text = "Register Build"
	register_btn.tooltip_text = "Register the Build/ folder with wdapp for fast dev iteration"
	register_btn.pressed.connect(on_register_loose)
	export_btn_row.add_child(register_btn)

	export_status_label = Label.new()
	export_status_label.text = ""
	root.add_child(export_status_label)

	root.add_child(HSeparator.new())

	_coordinator._add_section_header(root, "Package Source Configuration")

	source_dir_edit = _coordinator._add_path_field(root, "Content Directory", "Directory with exported game files", true)
	_coordinator._add_open_folder_btn(source_dir_edit)
	map_file_edit = _coordinator._add_path_field(root, "Mapping File", "XML mapping file (or auto-generate)", false)

	auto_genmap_check = CheckBox.new()
	auto_genmap_check.text = "Auto-generate mapping file before packaging"
	auto_genmap_check.button_pressed = true
	auto_genmap_check.toggled.connect(_on_auto_genmap_toggled)
	root.add_child(auto_genmap_check)

	output_dir_edit = _coordinator._add_path_field(root, "Output Directory", "Package/ (default)", true)
	_coordinator._add_open_folder_btn(output_dir_edit)

	root.add_child(HSeparator.new())

	_coordinator._add_section_header(root, "Packaging Options")

	var cid_row := HBoxContainer.new()
	root.add_child(cid_row)
	var cid_label := Label.new()
	cid_label.text = "Content ID"
	cid_label.custom_minimum_size.x = 130
	cid_row.add_child(cid_label)
	content_id_edit = LineEdit.new()
	content_id_edit.placeholder_text = "Optional — from MicrosoftGame.config if blank"
	content_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	cid_row.add_child(content_id_edit)

	var pid_row := HBoxContainer.new()
	root.add_child(pid_row)
	var pid_label := Label.new()
	pid_label.text = "Product ID"
	pid_label.custom_minimum_size.x = 130
	pid_row.add_child(pid_label)
	product_id_edit = LineEdit.new()
	product_id_edit.placeholder_text = "Optional — from MicrosoftGame.config if blank"
	product_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	pid_row.add_child(product_id_edit)

	var enc_row := HBoxContainer.new()
	root.add_child(enc_row)
	var enc_label := Label.new()
	enc_label.text = "Encryption"
	enc_label.custom_minimum_size.x = 130
	enc_row.add_child(enc_label)
	encrypt_option = OptionButton.new()
	encrypt_option.add_item("None (dev default)")
	encrypt_option.add_item("License encrypt (/l)")
	encrypt_option.add_item("Custom key (/lk)")
	encrypt_option.item_selected.connect(_on_encrypt_changed)
	enc_row.add_child(encrypt_option)

	encrypt_key_edit = _coordinator._add_path_field(root, "EKB Key File", "Path to encryption key bundle file", false)
	encrypt_key_edit.get_parent().visible = false

	var compat_row := HBoxContainer.new()
	root.add_child(compat_row)
	var compat_label := Label.new()
	compat_label.text = "Update Compat"
	compat_label.custom_minimum_size.x = 130
	compat_row.add_child(compat_label)
	updcompat_option = OptionButton.new()
	updcompat_option.add_item("3 — Sub-file granularity (default)")
	updcompat_option.add_item("2 — File-level granularity")
	updcompat_option.add_item("1 — Legacy")
	compat_row.add_child(updcompat_option)

	root.add_child(HSeparator.new())

	_coordinator._add_section_header(root, "Packaging Actions")

	var action_row := HBoxContainer.new()
	root.add_child(action_row)

	export_package_btn = Button.new()
	export_package_btn.text = "Export & Package"
	export_package_btn.tooltip_text = "Export project then create MSIXVC package in one step"
	export_package_btn.pressed.connect(on_export_and_package)
	action_row.add_child(export_package_btn)

	pack_btn = Button.new()
	pack_btn.text = "Create Package Only"
	pack_btn.pressed.connect(on_pack)
	action_row.add_child(pack_btn)

	validate_btn = Button.new()
	validate_btn.text = "Validate Package"
	validate_btn.pressed.connect(on_validate)
	action_row.add_child(validate_btn)

	genmap_btn = Button.new()
	genmap_btn.text = "Generate Map"
	genmap_btn.pressed.connect(on_genmap)
	action_row.add_child(genmap_btn)

	_populate_preset_selector()


func apply_state(state: Dictionary) -> void:
	var packaging_state: Dictionary = state.get("packaging", {})
	var export_state: Dictionary = state.get("export", {})

	source_dir_edit.text = str(packaging_state.get("source_dir", ""))
	map_file_edit.text = str(packaging_state.get("map_file", ""))
	auto_genmap_check.button_pressed = bool(packaging_state.get("auto_genmap", true))
	output_dir_edit.text = str(packaging_state.get("output_dir", ""))
	content_id_edit.text = str(packaging_state.get("content_id", ""))
	product_id_edit.text = str(packaging_state.get("product_id", ""))
	encrypt_option.selected = int(packaging_state.get("encrypt_option", ENCRYPT_NONE))
	encrypt_key_edit.text = str(packaging_state.get("encrypt_key", ""))
	updcompat_option.selected = int(packaging_state.get("updcompat_option", 0))
	clean_build_check.button_pressed = bool(export_state.get("clean_build", false))

	var saved_preset = str(export_state.get("preset_name", ""))
	for i in preset_selector.get_item_count():
		if preset_selector.get_item_text(i) == saved_preset:
			preset_selector.select(i)
			break

	_on_encrypt_changed(encrypt_option.selected)
	_on_auto_genmap_toggled(auto_genmap_check.button_pressed)


func collect_state() -> Dictionary:
	var preset_name := ""
	if preset_selector.selected >= 0 and not preset_selector.get_item_text(preset_selector.selected).begins_with("No "):
		preset_name = preset_selector.get_item_text(preset_selector.selected)

	return {
		"packaging": {
			"source_dir": source_dir_edit.text,
			"map_file": map_file_edit.text,
			"auto_genmap": auto_genmap_check.button_pressed,
			"output_dir": output_dir_edit.text,
			"content_id": content_id_edit.text,
			"product_id": product_id_edit.text,
			"encrypt_option": encrypt_option.selected,
			"encrypt_key": encrypt_key_edit.text,
			"updcompat_option": updcompat_option.selected,
		},
		"export": {
			"preset_name": preset_name,
			"clean_build": clean_build_check.button_pressed,
		},
	}


func connect_autosave(save_callback: Callable) -> void:
	for edit in [source_dir_edit, map_file_edit, output_dir_edit, content_id_edit, product_id_edit, encrypt_key_edit]:
		edit.text_changed.connect(func(_value): save_callback.call())
		edit.focus_exited.connect(save_callback)

	auto_genmap_check.toggled.connect(func(_pressed): save_callback.call())
	encrypt_option.item_selected.connect(func(_index): save_callback.call())
	updcompat_option.item_selected.connect(func(_index): save_callback.call())
	preset_selector.item_selected.connect(func(_index): save_callback.call())
	clean_build_check.toggled.connect(func(_pressed): save_callback.call())


func set_actions_enabled(enabled: bool) -> void:
	genmap_btn.disabled = not enabled
	validate_btn.disabled = not enabled
	pack_btn.disabled = not enabled


func _populate_preset_selector() -> void:
	preset_selector.clear()
	var cfg_path = ProjectSettings.globalize_path("res://export_presets.cfg")
	if not FileAccess.file_exists(cfg_path):
		preset_selector.add_item("No export presets — add one in Project → Export")
		export_btn.disabled = true
		export_register_btn.disabled = true
		export_package_btn.disabled = true
		return

	var presets = _coordinator.get_export_preset_catalog().list_windows_presets(cfg_path)
	for preset in presets:
		preset_selector.add_item(str(preset["name"]), int(preset["preset_index"]))

	if presets.is_empty():
		preset_selector.add_item("No Windows preset — add one in Project → Export")
		export_btn.disabled = true
		export_register_btn.disabled = true
		export_package_btn.disabled = true
	else:
		export_btn.disabled = false
		export_register_btn.disabled = false
		export_package_btn.disabled = false


func _on_encrypt_changed(index: int) -> void:
	encrypt_key_edit.get_parent().visible = (index == ENCRYPT_CUSTOM_KEY)


func _on_auto_genmap_toggled(pressed: bool) -> void:
	map_file_edit.editable = not pressed
	if pressed:
		map_file_edit.placeholder_text = "Will be auto-generated in output directory"
	else:
		map_file_edit.placeholder_text = "XML mapping file path"


func on_genmap() -> void:
	var source := source_dir_edit.text.strip_edges()
	if source == "":
		_coordinator._log("❌ Content directory is required for genmap.")
		return

	var output := output_dir_edit.text.strip_edges()
	if output == "":
		output = source
	var map_path := output.path_join("layout.xml")

	if FileAccess.file_exists(map_path):
		var confirm := ConfirmationDialog.new()
		confirm.dialog_text = "layout.xml already exists at:\n%s\n\nOverwrite it?" % map_path
		confirm.title = "Overwrite Mapping File?"
		confirm.confirmed.connect(func():
			_do_genmap(source, map_path)
			confirm.queue_free())
		confirm.canceled.connect(func(): confirm.queue_free())
		add_child(confirm)
		confirm.popup_centered()
		return

	_do_genmap(source, map_path)


func _do_genmap(source: String, map_path: String) -> void:
	DirAccess.make_dir_recursive_absolute(map_path.get_base_dir())
	_coordinator._log("Generating mapping file...")
	var result = _coordinator.get_makepkg().genmap(source, map_path)
	_coordinator._log_result(result)
	if result["exit_code"] == 0:
		map_file_edit.text = map_path


func on_validate() -> void:
	var source := source_dir_edit.text.strip_edges()
	var map_file := map_file_edit.text.strip_edges()
	var output := output_dir_edit.text.strip_edges()
	if source == "" or map_file == "":
		_coordinator._log("❌ Content directory and mapping file are required for validation.")
		return
	if output == "":
		output = source
	if not _coordinator.get_content_preparer().ensure_content_dir_ready(source, Callable(_coordinator, "_log")):
		return

	var progress := AcceptDialog.new()
	progress.exclusive = false
	progress.title = "Validating Package"
	progress.dialog_text = "Validating package, this may take a minute..."
	progress.get_ok_button().visible = false
	add_child(progress)
	progress.popup_centered(Vector2i(450, 150))

	await get_tree().process_frame
	await get_tree().process_frame

	_coordinator._log("Validating package layout...")
	var result = _coordinator.get_makepkg().validate(map_file, source, output)
	_coordinator._log_result(result)

	progress.get_ok_button().visible = true
	if result["exit_code"] == 0:
		progress.dialog_text = "✅ Package validation passed!"
	else:
		progress.dialog_text = "❌ Package validation failed.\nCheck the Output panel for details."
	progress.confirmed.connect(func(): progress.queue_free())


func on_pack() -> void:
	var source := source_dir_edit.text.strip_edges()
	var output := output_dir_edit.text.strip_edges()
	if source == "":
		_coordinator._log("❌ Content directory is required.")
		return
	if output == "":
		_coordinator._log("❌ Output directory is required.")
		return
	if not _coordinator.get_content_preparer().ensure_content_dir_ready(source, Callable(_coordinator, "_log")):
		return

	var progress := AcceptDialog.new()
	progress.exclusive = false
	progress.title = "Creating Package"
	progress.dialog_text = "Creating MSIXVC package...\nThis may take a minute."
	progress.get_ok_button().visible = false
	add_child(progress)
	progress.popup_centered(Vector2i(400, 120))

	await get_tree().process_frame

	var map_file := map_file_edit.text.strip_edges()
	if auto_genmap_check.button_pressed or map_file == "":
		var map_path := output.path_join("layout.xml")
		DirAccess.make_dir_recursive_absolute(output)

		if FileAccess.file_exists(map_path):
			_coordinator._log("Overwriting existing layout.xml for packaging...")

		progress.dialog_text = "Generating mapping file..."
		await get_tree().process_frame

		var genmap_result = _coordinator.get_makepkg().genmap(source, map_path)
		_coordinator._log_result(genmap_result)
		if genmap_result["exit_code"] != 0:
			_coordinator._log("❌ Mapping file generation failed — aborting package.")
			progress.dialog_text = "❌ Mapping file generation failed."
			progress.get_ok_button().visible = true
			progress.confirmed.connect(func(): progress.queue_free())
			return
		map_file = map_path
		map_file_edit.text = map_file

	progress.dialog_text = "Creating MSIXVC package...\nThis may take a minute."
	await get_tree().process_frame

	var options := {}
	if content_id_edit.text.strip_edges() != "":
		options["content_id"] = content_id_edit.text.strip_edges()
	if product_id_edit.text.strip_edges() != "":
		options["product_id"] = product_id_edit.text.strip_edges()

	match encrypt_option.selected:
		ENCRYPT_LICENSE:
			options["encrypt"] = true
		ENCRYPT_CUSTOM_KEY:
			options["encrypt_key"] = encrypt_key_edit.text.strip_edges()

	var updcompat_map := [3, 2, 1]
	options["updcompat"] = updcompat_map[updcompat_option.selected]

	_coordinator._log("Creating MSIXVC package...")
	var result = _coordinator.get_makepkg().pack(source, map_file, output, options)
	_coordinator._log_result(result)

	progress.get_ok_button().visible = true
	if result["exit_code"] == 0:
		progress.dialog_text = "✅ Package created successfully!"
	else:
		progress.dialog_text = "❌ Package creation failed.\nCheck the Output panel for details."
	progress.confirmed.connect(func(): progress.queue_free())


func on_export() -> void:
	var build_dir = _coordinator.get_build_dir()
	DirAccess.make_dir_recursive_absolute(build_dir)

	if clean_build_check.button_pressed:
		_coordinator._clean_directory(build_dir)
		_coordinator._log("Cleaned Build/ folder")

	var preset_name = preset_selector.get_item_text(preset_selector.selected)
	if preset_name == "" or preset_name.begins_with("No "):
		export_status_label.text = "❌ No valid export preset selected"
		return

	var game_name = ProjectSettings.get_setting("application/config/name", "Game")
	var exe_path = build_dir.path_join(game_name + ".exe")

	var progress := AcceptDialog.new()
	progress.exclusive = false
	progress.title = "Exporting"
	progress.dialog_text = "Exporting project to Build/...\nThis may take a minute."
	progress.get_ok_button().visible = false
	add_child(progress)
	progress.popup_centered(Vector2i(450, 150))

	await get_tree().process_frame
	await get_tree().process_frame

	var godot_path = OS.get_executable_path()
	var project_path = ProjectSettings.globalize_path("res://")
	_coordinator._log("Exporting '%s' preset to: %s" % [preset_name, exe_path])

	var output: Array = []
	var exit_code = OS.execute(godot_path, PackedStringArray([
		"--headless",
		"--path", project_path,
		"--export-debug", preset_name, exe_path
	]), output, true, false)

	var stdout_text = str(output[0]) if output.size() > 0 else ""

	if exit_code == OK:
		export_status_label.text = "✅ Exported to Build/"
		_coordinator._log("Export completed successfully")
		_post_export_prepare(build_dir)
		if source_dir_edit.text.strip_edges() == "":
			source_dir_edit.text = build_dir
			_coordinator._save_packaging_settings()
	else:
		export_status_label.text = "❌ Export failed (exit code %d)" % exit_code
		_coordinator._log("Export failed (exit code %d): %s" % [exit_code, stdout_text])
		push_error("[GDK] Export failed (exit code %d)" % exit_code)

	progress.get_ok_button().visible = true
	if exit_code == OK:
		progress.dialog_text = "✅ Export completed!\nBuild files are in the Build/ folder."
	else:
		progress.dialog_text = "❌ Export failed.\nCheck the Output panel for details."
	progress.confirmed.connect(func(): progress.queue_free())


func _post_export_prepare(build_dir: String) -> void:
	if not _coordinator.get_content_preparer().ensure_content_dir_ready(build_dir, Callable(_coordinator, "_log")):
		_coordinator._log("⚠️ Post-export config setup had issues")

	_copy_addon_runtime_dlls(build_dir)

	if output_dir_edit.text.strip_edges() == "":
		output_dir_edit.text = _coordinator.get_package_dir()
		_coordinator._save_packaging_settings()


## Copies satellite runtime DLLs from each addon's bin/ folder next to the
## exported exe.  Godot's standard exporter only copies the GDExtension
## library itself; native dependencies that the GDExtension links against
## (e.g. libHttpClient.dll, Microsoft.Xbox.Services.C.Thunks*.dll for
## godot_gdk; Party.dll, PlayFabCore.dll, etc. for godot_playfab) live next
## to the GDExtension in the source tree and are picked up by the OS loader
## via lazy resolution.  In an exported / registered loose-layout build the
## GDExtension lives next to the exe instead, so its satellites must be
## copied there too — otherwise the kernel can't resolve the imports and the
## extension silently fails to load (HUD shows "GDK OFFLINE").
##
## Declaring the satellites in the .gdextension's [dependencies] block was
## tried first, but that causes Godot's loader to force-load every listed
## DLL eagerly via OS::open_dynamic_library, including in F5-from-editor.
## When the bin/ folder contains parallel variants of the same XSAPI runtime
## (e.g. Microsoft.Xbox.Services.C.Thunks.Debug.dll alongside the GDK
## variant), force-loading multiple variants collides on identical exports
## and hangs XGameRuntime initialization.  Lazy resolution picks exactly one
## variant per import — whichever the GDExtension's IAT actually references
## — so this packaging-time copy preserves that behavior in exported builds.
func _copy_addon_runtime_dlls(build_dir: String) -> void:
	var addons_root := "res://addons"
	var addons_dir := DirAccess.open(addons_root)
	if addons_dir == null:
		return

	addons_dir.list_dir_begin()
	var copied_total := 0
	var addon_name := addons_dir.get_next()
	while addon_name != "":
		if addons_dir.current_is_dir() and not addon_name.begins_with("."):
			var bin_dir_res := "%s/%s/bin" % [addons_root, addon_name]
			copied_total += _copy_addon_bin_dlls(addon_name, bin_dir_res, build_dir)
		addon_name = addons_dir.get_next()
	addons_dir.list_dir_end()

	if copied_total > 0:
		_coordinator._log("Copied %d addon runtime DLL(s) into build dir" % copied_total)


## Copies non-GDExtension DLLs from a single addon's bin/ folder into the
## build dir.  Returns the number of files copied.  Skips .pdb files and the
## GDExtension library itself (Godot's exporter already places that next to
## the exe).
func _copy_addon_bin_dlls(addon_name: String, bin_dir_res: String, build_dir: String) -> int:
	var bin_dir := DirAccess.open(bin_dir_res)
	if bin_dir == null:
		return 0

	var copied := 0
	var gdext_prefix := "%s.windows." % addon_name
	bin_dir.list_dir_begin()
	var fname := bin_dir.get_next()
	while fname != "":
		if not bin_dir.current_is_dir() and fname.to_lower().ends_with(".dll"):
			# Skip the GDExtension library itself — Godot's exporter handles it.
			if not fname.begins_with(gdext_prefix):
				var src := "%s/%s" % [bin_dir_res, fname]
				var dst := build_dir.path_join(fname)
				var src_abs := ProjectSettings.globalize_path(src)
				var copy_err := DirAccess.copy_absolute(src_abs, dst)
				if copy_err == OK:
					copied += 1
				else:
					_coordinator._log("⚠️ Failed to copy %s → %s (err %d)" % [src_abs, dst, copy_err])
		fname = bin_dir.get_next()
	bin_dir.list_dir_end()

	return copied


func on_register_loose() -> void:
	var build_dir = _coordinator.get_build_dir()
	if not DirAccess.dir_exists_absolute(build_dir):
		export_status_label.text = "❌ Build/ folder not found — export first"
		return

	if not _coordinator.get_wdapp_manager().is_available():
		export_status_label.text = "❌ wdapp.exe not found"
		return

	_coordinator._log("Registering loose build: %s" % build_dir)
	var result = _coordinator.get_wdapp_manager().register_loose(build_dir)
	if result["exit_code"] == 0:
		export_status_label.text = "✅ Registered loose build"
		_coordinator._log("wdapp register succeeded")
	else:
		export_status_label.text = "❌ Registration failed"
		_coordinator._log("wdapp register failed: %s" % result["stdout"])
		push_warning("[GDK] wdapp register failed — may need admin privileges")


func on_export_and_register() -> void:
	await on_export()
	if export_status_label.text.begins_with("✅"):
		on_register_loose()


func on_export_and_package() -> void:
	await on_export()
	if export_status_label.text.begins_with("✅"):
		source_dir_edit.text = _coordinator.get_build_dir()
		output_dir_edit.text = _coordinator.get_package_dir()
		_coordinator._save_packaging_settings()
		await on_pack()
