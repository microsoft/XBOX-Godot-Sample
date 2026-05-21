@tool
extends AcceptDialog
## GDK Package Manager — a machine-wide view of every package registered with
## wdapp.exe, with actions to install a .msixvc from any path on disk and to
## uninstall any registered package.
##
## The dialog is project-agnostic: it reflects every package currently
## installed on this PC, not just the current Godot project.
##
## All shell-outs go through [code]wdapp_manager.gd[/code]'s async API so the
## editor UI does not freeze while wdapp is running (install on an MSIXVC
## can take many seconds). The dialog connects to the manager's completion
## signals once in [method setup] and dispatches via the [code]*_async[/code]
## entry points. Only one wdapp operation runs at a time per manager; the
## refresh / install / uninstall buttons are disabled while an op is in
## flight. When the tool is missing, the dialog still opens but disables the
## action buttons and explains why.

const WdappManagerScript = preload("res://addons/godot_gdk_packaging/core/wdapp_manager.gd")

const COL_PFN := 0
const COL_AUMID := 1

var _toolchain: RefCounted
var _wdapp_manager: RefCounted

var _status_label: Label
var _install_btn: Button
var _refresh_btn: Button
var _export_btn: Button
var _uninstall_btn: Button
var _tree: Tree
var _install_file_dialog: FileDialog
var _confirm_uninstall_dialog: ConfirmationDialog

# Pending-op state. _pending_install_path / _pending_uninstall_pfn carry the
# user-supplied path / PFN across the async dispatch so the completion
# handler can include them in the status message. _pending_status_prefix
# preserves the install / uninstall result line across the auto-refresh that
# follows -- without this, the "Listing..." status would clobber it.
var _pending_uninstall_pfn: String = ""
var _pending_install_path: String = ""
var _pending_status_prefix: String = ""

var _registered_apps: Array[Dictionary] = []


func _init() -> void:
	title = "GDK Package Manager"
	ok_button_text = "Close"
	min_size = Vector2i(820, 480)
	exclusive = false
	unresizable = false


func setup(toolchain: RefCounted) -> void:
	_toolchain = toolchain
	# Replace any prior manager safely: dispose its worker thread first so we
	# don't orphan an in-flight op or leak a live Godot Thread.
	if _wdapp_manager != null:
		_wdapp_manager.dispose()
		_wdapp_manager = null
	# Guard: a null toolchain means GDK detection failed or this dialog was
	# constructed programmatically without one. Leave _wdapp_manager null so
	# refresh() shows the "not initialized" status instead of dereferencing a
	# null toolchain inside WdappManager.is_available().
	if toolchain == null:
		_wdapp_manager = null
	else:
		_wdapp_manager = WdappManagerScript.new(toolchain)
		_wdapp_manager.list_completed.connect(_on_list_completed)
		_wdapp_manager.install_completed.connect(_on_install_completed)
		_wdapp_manager.uninstall_completed.connect(_on_uninstall_completed)
	if get_child_count() == 0:
		_build_ui()
	refresh()


func _notification(what: int) -> void:
	if what == NOTIFICATION_PREDELETE:
		# Explicit shutdown: join any in-flight wdapp thread before this
		# dialog (and its _wdapp_manager) are freed.
		if _wdapp_manager != null:
			_wdapp_manager.dispose()


func show_centered_clamped() -> void:
	_apply_screen_size_cap()
	popup_centered()
	refresh()


# Caps max_size to fit the current monitor (with a small chrome / taskbar
# margin) so the dialog never opens -- or gets resized -- taller than the
# user's screen. Re-evaluated per popup because the editor may move between
# monitors of different sizes between opens.
func _apply_screen_size_cap() -> void:
	var screen_idx: int = _current_screen_index()
	var usable: Rect2i = DisplayServer.screen_get_usable_rect(screen_idx)
	if usable.size.x <= 0 or usable.size.y <= 0:
		return  # DisplayServer didn't report a usable rect; keep defaults
	var capped_w: int = max(min_size.x, usable.size.x - 80)
	var capped_h: int = max(min_size.y, usable.size.y - 120)
	max_size = Vector2i(capped_w, capped_h)
	# Also actively shrink the current size if a previous open left it
	# larger than what fits the current screen (e.g. user moved monitors).
	if size.x > capped_w or size.y > capped_h:
		size = Vector2i(mini(size.x, capped_w), mini(size.y, capped_h))


# Returns the screen index the editor's main window is currently on, so the
# cap matches the monitor the dialog will actually open on. Falls back to
# the primary screen if no editor window is available.
func _current_screen_index() -> int:
	var base: Control = EditorInterface.get_base_control()
	if base != null:
		var w: Window = base.get_window()
		if w != null:
			return w.current_screen
	return DisplayServer.get_primary_screen()


func _build_ui() -> void:
	var root: VBoxContainer = VBoxContainer.new()
	root.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	root.size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_child(root)

	var header: Label = Label.new()
	header.text = "Packages registered with wdapp.exe (machine-wide)"
	header.add_theme_font_size_override("font_size", 14)
	root.add_child(header)

	var desc: Label = Label.new()
	desc.text = "This list reflects every package currently installed on this machine — not just the current Godot project."
	desc.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	desc.add_theme_font_size_override("font_size", 11)
	desc.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(desc)

	root.add_child(HSeparator.new())

	var toolbar: HBoxContainer = HBoxContainer.new()
	root.add_child(toolbar)

	_refresh_btn = Button.new()
	_refresh_btn.text = "Refresh"
	_refresh_btn.tooltip_text = "Re-run wdapp list and refresh the package list"
	_refresh_btn.pressed.connect(refresh)
	toolbar.add_child(_refresh_btn)

	_install_btn = Button.new()
	_install_btn.text = "Install from .msixvc…"
	_install_btn.tooltip_text = "Pick a .msixvc package on disk and install it via wdapp"
	_install_btn.pressed.connect(_on_install_pressed)
	toolbar.add_child(_install_btn)

	_export_btn = Button.new()
	_export_btn.text = "Export Project…"
	_export_btn.tooltip_text = "Open Project > Export... to build a new MSIXVC for the current project"
	_export_btn.pressed.connect(_on_export_pressed)
	toolbar.add_child(_export_btn)

	var spacer: Control = Control.new()
	spacer.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	toolbar.add_child(spacer)

	_uninstall_btn = Button.new()
	_uninstall_btn.text = "Uninstall Selected"
	_uninstall_btn.tooltip_text = "Uninstall the selected package via wdapp"
	_uninstall_btn.disabled = true
	_uninstall_btn.pressed.connect(_on_uninstall_pressed)
	toolbar.add_child(_uninstall_btn)

	_tree = Tree.new()
	_tree.columns = 2
	_tree.column_titles_visible = true
	_tree.set_column_title(COL_PFN, "Package Full Name")
	_tree.set_column_title(COL_AUMID, "AUMID")
	_tree.set_column_expand(COL_PFN, true)
	_tree.set_column_expand(COL_AUMID, true)
	_tree.set_column_clip_content(COL_PFN, true)
	_tree.set_column_clip_content(COL_AUMID, true)
	_tree.hide_root = true
	_tree.select_mode = Tree.SELECT_ROW
	_tree.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_tree.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_tree.custom_minimum_size = Vector2(0, 280)
	_tree.item_selected.connect(_on_tree_item_selected)
	_tree.nothing_selected.connect(_on_tree_nothing_selected)
	root.add_child(_tree)

	_status_label = Label.new()
	_status_label.text = ""
	_status_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	root.add_child(_status_label)

	_install_file_dialog = FileDialog.new()
	_install_file_dialog.title = "Select .msixvc package to install"
	_install_file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	_install_file_dialog.access = FileDialog.ACCESS_FILESYSTEM
	_install_file_dialog.add_filter("*.msixvc", "MSIXVC packages")
	_install_file_dialog.use_native_dialog = true
	_install_file_dialog.file_selected.connect(_on_install_file_chosen)
	add_child(_install_file_dialog)

	_confirm_uninstall_dialog = ConfirmationDialog.new()
	_confirm_uninstall_dialog.title = "Uninstall package?"
	_confirm_uninstall_dialog.ok_button_text = "Uninstall"
	_confirm_uninstall_dialog.confirmed.connect(_on_uninstall_confirmed)
	add_child(_confirm_uninstall_dialog)


func refresh() -> void:
	if _tree == null:
		return
	_tree.clear()
	_registered_apps.clear()
	_uninstall_btn.disabled = true

	if _wdapp_manager == null:
		_set_status(_join_status(_pending_status_prefix, "Package manager not initialized -- pass a toolchain via setup()."))
		_install_btn.disabled = true
		_refresh_btn.disabled = false
		return

	if not _wdapp_manager.is_available():
		_set_status(_join_status(_pending_status_prefix, "wdapp.exe not found -- install the Microsoft GDK to use this window."))
		_install_btn.disabled = true
		_refresh_btn.disabled = false
		return

	if not _wdapp_manager.list_registered_apps_async():
		# Already busy with another wdapp op (or disposed) -- buttons stay
		# disabled by the in-flight op and its completion handler refreshes.
		_set_status(_join_status(_pending_status_prefix, "Busy with another wdapp operation; please wait..."))
		return

	_set_wdapp_buttons_busy(true)
	_set_status(_join_status(_pending_status_prefix, "Listing registered packages..."))


func _on_list_completed(result: Dictionary) -> void:
	var prefix: String = _pending_status_prefix
	_pending_status_prefix = ""
	_set_wdapp_buttons_busy(false)

	if int(result.get("exit_code", -1)) != 0:
		_set_status(_join_status(prefix, "wdapp list failed:\n%s" % _format_failure(result)))
		return

	_registered_apps = result.get("apps", [])
	var root_item: TreeItem = _tree.create_item()
	for app: Dictionary in _registered_apps:
		var item: TreeItem = _tree.create_item(root_item)
		item.set_text(COL_PFN, str(app.get("pfn", "")))
		item.set_text(COL_AUMID, str(app.get("aumid", "")))
		item.set_tooltip_text(COL_PFN, str(app.get("pfn", "")))
		item.set_tooltip_text(COL_AUMID, str(app.get("aumid", "")))
		item.set_metadata(0, app)

	var summary: String
	if _registered_apps.is_empty():
		summary = "No packages registered."
	else:
		summary = "Found %d registered package%s." % [
			_registered_apps.size(),
			"" if _registered_apps.size() == 1 else "s",
		]
	_set_status(_join_status(prefix, summary))


func _on_install_pressed() -> void:
	_install_file_dialog.popup_centered_ratio(0.75)


func _on_export_pressed() -> void:
	# Drive the standard Project > Export... dialog so the user reuses their
	# configured GDK preset (no duplicated preset surface inside this addon).
	# The export dialog runs the GDK ``EditorExportPlatformExtension``, which
	# stages, packs MSIXVC, and (in dev/register_loose mode) registers
	# automatically. The Package Manager auto-refreshes on next open.
	if _open_project_export_dialog():
		_set_status("Opened Project > Export...")
	else:
		_set_status("Could not locate Project > Export... -- open it manually from the Project menu.")
		push_warning("[GDK Package Manager] Could not find Project > Export... menu item")


func _on_install_file_chosen(path: String) -> void:
	if path == "" or _wdapp_manager == null:
		return
	if not _wdapp_manager.install_package_async(path):
		_set_status("Busy with another wdapp operation; please wait...")
		return
	_pending_install_path = path
	_set_wdapp_buttons_busy(true)
	_set_status("Installing %s..." % path)


func _on_install_completed(result: Dictionary) -> void:
	var path: String = _pending_install_path
	_pending_install_path = ""
	if int(result.get("exit_code", -1)) == 0:
		_pending_status_prefix = "Installed: %s" % path
	else:
		_pending_status_prefix = "Install failed: %s\n%s" % [path, _format_failure(result)]
		push_warning("[GDK Package Manager] wdapp install failed for %s: exit %d, %s" % [
			path, int(result.get("exit_code", -1)), str(result.get("stdout", "")),
		])
	# refresh() re-enables / re-disables buttons based on the new list state.
	refresh()


func _on_uninstall_pressed() -> void:
	var pfn: String = _get_selected_pfn()
	if pfn == "":
		_set_status("No package selected.")
		return
	_pending_uninstall_pfn = pfn
	_confirm_uninstall_dialog.dialog_text = "Uninstall this package?\n\n%s\n\nThis cannot be undone." % pfn
	_confirm_uninstall_dialog.popup_centered(Vector2i(420, 160))


func _on_uninstall_confirmed() -> void:
	# Don't clear _pending_uninstall_pfn here -- the async completion handler
	# needs the PFN to include it in the status message.
	var pfn: String = _pending_uninstall_pfn
	if pfn == "" or _wdapp_manager == null:
		return
	if not _wdapp_manager.uninstall_package_async(pfn):
		_set_status("Busy with another wdapp operation; please wait...")
		_pending_uninstall_pfn = ""
		return
	_set_wdapp_buttons_busy(true)
	_set_status("Uninstalling %s..." % pfn)


func _on_uninstall_completed(result: Dictionary) -> void:
	var pfn: String = _pending_uninstall_pfn
	_pending_uninstall_pfn = ""
	if int(result.get("exit_code", -1)) == 0:
		_pending_status_prefix = "Uninstalled: %s" % pfn
	else:
		_pending_status_prefix = "Uninstall failed: %s\n%s" % [pfn, _format_failure(result)]
		push_warning("[GDK Package Manager] wdapp uninstall failed for %s: exit %d, %s" % [
			pfn, int(result.get("exit_code", -1)), str(result.get("stdout", "")),
		])
	refresh()


func _on_tree_item_selected() -> void:
	_uninstall_btn.disabled = _get_selected_pfn() == ""


func _on_tree_nothing_selected() -> void:
	_uninstall_btn.disabled = true


func _get_selected_pfn() -> String:
	var item: TreeItem = _tree.get_selected()
	if item == null:
		return ""
	var meta: Variant = item.get_metadata(0)
	if typeof(meta) != TYPE_DICTIONARY:
		return ""
	return str((meta as Dictionary).get("pfn", ""))


func _set_status(text: String) -> void:
	if _status_label:
		_status_label.text = text


# Combines an optional result prefix (e.g. "Installed: X") with a follow-up
# status line (e.g. "Listing registered packages..."). Used to preserve the
# install / uninstall result across the auto-refresh that follows it,
# instead of having "Listing..." clobber it.
static func _join_status(prefix: String, msg: String) -> String:
	if prefix == "":
		return msg
	return prefix + "\n" + msg


# Toggles the wdapp-action buttons. Export stays enabled regardless because
# it opens Godot's own Project > Export... dialog (not a wdapp shell-out).
func _set_wdapp_buttons_busy(busy: bool) -> void:
	if _refresh_btn:
		_refresh_btn.disabled = busy
	if _install_btn:
		_install_btn.disabled = busy
	if _uninstall_btn:
		# Even when not busy, only enable uninstall if there is a current
		# selection. _on_tree_item_selected will re-enable it on selection.
		_uninstall_btn.disabled = busy or _get_selected_pfn() == ""


# Walks the editor's MenuBar to find ``Project > Export...`` and activates it
# by emitting ``id_pressed`` on the corresponding PopupMenu, which is what
# the editor itself listens to. Returns ``true`` on success.
#
# Matches the menu title ``Project`` and the menu item text ``Export…`` /
# ``Export...`` exactly, with a substring fallback ("Export") so we still
# work if Godot tweaks the ellipsis. Localized editors will fall through
# the fallback; we accept that as best-effort.
func _open_project_export_dialog() -> bool:
	var base: Control = EditorInterface.get_base_control()
	if base == null:
		return false
	var menu_bar: MenuBar = _find_menu_bar(base)
	if menu_bar == null:
		return false
	for i in menu_bar.get_menu_count():
		if menu_bar.get_menu_title(i) != "Project":
			continue
		var popup: PopupMenu = menu_bar.get_menu_popup(i)
		if popup == null:
			return false
		var export_id: int = _find_menu_item_id(popup, "Export…")
		if export_id < 0:
			export_id = _find_menu_item_id(popup, "Export...")
		if export_id < 0:
			# Fallback: substring match anywhere ("Export ...", localized strings)
			for j in popup.get_item_count():
				var txt: String = popup.get_item_text(j)
				if txt.findn("export") >= 0 and not popup.is_item_separator(j):
					export_id = popup.get_item_id(j)
					break
		if export_id < 0:
			return false
		popup.id_pressed.emit(export_id)
		return true
	return false


static func _find_menu_item_id(popup: PopupMenu, exact_text: String) -> int:
	for j in popup.get_item_count():
		if popup.get_item_text(j) == exact_text:
			return popup.get_item_id(j)
	return -1


static func _find_menu_bar(node: Node) -> MenuBar:
	if node is MenuBar:
		return node
	for child: Node in node.get_children():
		var result: MenuBar = _find_menu_bar(child)
		if result:
			return result
	return null


# Builds a "exit N: <output>" string from a wdapp_manager result dict.
# Note: GDKToolchain.execute_tool merges stderr into stdout (Godot's
# OS.execute with read_stderr=false), so the "output" field below already
# contains both streams. We surface that fact in the format so reviewers
# / users don't expect a separate stderr line.
static func _format_failure(result: Dictionary) -> String:
	var exit_code: int = int(result.get("exit_code", -1))
	var output: String = str(result.get("stdout", "")).strip_edges()
	if output == "":
		return "exit %d (no output)" % exit_code
	return "exit %d, output (stdout+stderr merged):\n%s" % [exit_code, output]
