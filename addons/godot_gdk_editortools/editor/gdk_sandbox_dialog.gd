@tool
extends AcceptDialog
## GDK Sandbox Switcher — minimal modal dialog opened from the GDK menu.
##
## Wraps the [code]XblPCSandbox.exe[/code] tool shipped with the GDK so
## developers can switch between Xbox development sandboxes (or back to
## RETAIL) without leaving the editor. Mutations are confirmed before they
## run; elevation failures surface a clear user-visible error.
##
## The dialog never reads or writes registry directly — every operation
## goes through [code]XblPCSandbox.exe[/code]. When the tool is missing,
## the dialog still opens but disables the action buttons and explains why.

const _RETAIL_LABEL: String = "RETAIL"
# Wrap width for the autowrap labels. Kept below min_size.x (540) so it never
# forces the window wider, but pins a width so autowrap heights are computed
# correctly on first open instead of inflating the window vertically.
const _CONTENT_WIDTH: int = 500
const _MACHINE_WIDE_WARNING: String = (
	"⚠  Sandbox switching is machine-wide. It affects every signed-in user, "
	+ "every Xbox tool, and every Xbox-aware app running on this PC — not "
	+ "just Godot. Switching back to RETAIL is required before consuming "
	+ "retail Xbox Live services."
)

var _toolchain: RefCounted
var _current_label: Label
var _target_edit: LineEdit
var _set_btn: Button
var _retail_btn: Button
var _refresh_btn: Button
var _status_label: Label


func _init() -> void:
	title = "GDK Sandbox Switcher"
	ok_button_text = "Close"
	min_size = Vector2i(540, 0)
	exclusive = false
	unresizable = false


func setup(toolchain: RefCounted) -> void:
	_toolchain = toolchain
	if get_child_count() == 0:
		_build_ui()
	refresh_status()


func show_centered_clamped() -> void:
	# First-open fix (issue #61). The dialog's height is derived from its
	# content's combined minimum size, but an AUTOWRAP Label only knows its
	# real (wrapped) height once it has been laid out against a constrained
	# width. Before the first popup no layout pass has run, so a pre-show
	# reset_size() overshoots and the window opens taller than the screen.
	# Cap to the current screen so the transient frame can never go
	# off-screen, show (which runs a layout pass at the real min_size width),
	# then snap to the now-correct content size on the next frame.
	_apply_screen_size_cap()
	popup_centered()
	refresh_status()
	await get_tree().process_frame
	if not is_inside_tree() or not visible:
		return
	reset_size()
	move_to_center()


# Caps max_size to fit the current monitor (with a small chrome / taskbar
# margin) so the dialog never opens taller than the user's screen. Re-evaluated
# per popup because the editor may move between monitors of different sizes.
func _apply_screen_size_cap() -> void:
	var screen_idx: int = _current_screen_index()
	var usable: Rect2i = DisplayServer.screen_get_usable_rect(screen_idx)
	if usable.size.x <= 0 or usable.size.y <= 0:
		return  # DisplayServer didn't report a usable rect; keep defaults
	var capped_w: int = max(min_size.x, usable.size.x - 80)
	var capped_h: int = max(min_size.y, usable.size.y - 120)
	max_size = Vector2i(capped_w, capped_h)
	# Actively shrink the current size if a previous open left it larger than
	# what fits the current screen (e.g. user moved monitors between opens).
	if size.x > capped_w or size.y > capped_h:
		size = Vector2i(mini(size.x, capped_w), mini(size.y, capped_h))


# Returns the screen index the editor's main window is currently on, so the
# cap matches the monitor the dialog will actually open on. Falls back to the
# primary screen if no editor window is available.
func _current_screen_index() -> int:
	var base: Control = EditorInterface.get_base_control()
	if base != null:
		var w: Window = base.get_window()
		if w != null:
			return w.current_screen
	return DisplayServer.get_primary_screen()


func _build_ui() -> void:
	var root := VBoxContainer.new()
	root.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	add_child(root)

	var warn := Label.new()
	warn.text = _MACHINE_WIDE_WARNING
	warn.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	# Pin a wrap width so the autowrap height is computed against a known
	# width even before the first layout pass. Without this the label reports
	# an inflated height on first open and the window opens screen-tall.
	warn.custom_minimum_size.x = _CONTENT_WIDTH
	warn.add_theme_color_override("font_color", Color(0.83, 0.51, 0.04))
	root.add_child(warn)

	root.add_child(HSeparator.new())

	var current_row := HBoxContainer.new()
	root.add_child(current_row)
	var current_prefix := Label.new()
	current_prefix.text = "Current sandbox:"
	current_prefix.custom_minimum_size.x = 150
	current_row.add_child(current_prefix)
	_current_label = Label.new()
	_current_label.text = "(refreshing…)"
	_current_label.add_theme_font_size_override("font_size", 14)
	current_row.add_child(_current_label)

	root.add_child(HSeparator.new())

	var target_row := HBoxContainer.new()
	root.add_child(target_row)
	var target_prefix := Label.new()
	target_prefix.text = "Target sandbox ID:"
	target_prefix.custom_minimum_size.x = 150
	target_row.add_child(target_prefix)
	_target_edit = LineEdit.new()
	_target_edit.placeholder_text = "e.g. XDKS.1"
	_target_edit.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	target_row.add_child(_target_edit)

	var button_row := HBoxContainer.new()
	root.add_child(button_row)

	_set_btn = Button.new()
	_set_btn.text = "Switch to Target"
	_set_btn.tooltip_text = "Runs XblPCSandbox.exe /set <target> /noApps. Requires admin privileges."
	_set_btn.pressed.connect(_on_set_pressed)
	button_row.add_child(_set_btn)

	_retail_btn = Button.new()
	_retail_btn.text = "Switch to RETAIL"
	_retail_btn.tooltip_text = "Runs XblPCSandbox.exe /retail /noApps. Requires admin privileges."
	_retail_btn.pressed.connect(_on_retail_pressed)
	button_row.add_child(_retail_btn)

	_refresh_btn = Button.new()
	_refresh_btn.text = "Refresh"
	_refresh_btn.pressed.connect(refresh_status)
	button_row.add_child(_refresh_btn)

	_status_label = Label.new()
	_status_label.text = ""
	_status_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_status_label.custom_minimum_size.x = _CONTENT_WIDTH
	_status_label.visible = false
	root.add_child(_status_label)


func refresh_status() -> void:
	if _toolchain == null:
		_set_actions_enabled(false)
		if _current_label != null:
			_current_label.text = "(toolchain not initialized)"
		return
	var sandbox_exe := str(_toolchain.get_sandbox_path())
	if sandbox_exe.is_empty():
		_set_actions_enabled(false)
		_current_label.text = "(XblPCSandbox.exe not found in GDK install)"
		_set_status(
			"XblPCSandbox.exe was not located in the active GDK install. "
			+ "Install the GDK or update its path under Editor Settings to "
			+ "enable sandbox switching."
		)
		return
	_set_actions_enabled(true)
	var result: Dictionary = _toolchain.execute_tool(sandbox_exe, PackedStringArray(["/get"]))
	var exit_code := int(result.get("exit_code", -1))
	var stdout := str(result.get("stdout", "")).strip_edges()
	if exit_code != 0:
		_current_label.text = "(could not determine — exit %d)" % exit_code
		_set_status("XblPCSandbox.exe /get failed. Output:\n%s" % (
			stdout if not stdout.is_empty() else "(no output)"
		))
		return
	var parsed := _parse_sandbox_id(stdout)
	_current_label.text = parsed if not parsed.is_empty() else stdout
	if not parsed.is_empty() and parsed != _RETAIL_LABEL:
		if _target_edit.text.strip_edges().is_empty():
			_target_edit.text = parsed
	_set_status("")


func _set_status(text: String) -> void:
	if _status_label == null:
		return
	_status_label.text = text
	_status_label.visible = not text.is_empty()
	if is_inside_tree():
		reset_size()


func _set_actions_enabled(enabled: bool) -> void:
	if _set_btn:
		_set_btn.disabled = not enabled
	if _retail_btn:
		_retail_btn.disabled = not enabled
	if _refresh_btn:
		_refresh_btn.disabled = not enabled


## Parses XblPCSandbox.exe /get stdout — the tool prints something like
## "Current sandbox id: XDKS.1" so we split on the first ':' if present.
func _parse_sandbox_id(out: String) -> String:
	if out.is_empty():
		return ""
	var idx := out.find(":")
	if idx < 0:
		return out
	return out.substr(idx + 1).strip_edges()


func _on_set_pressed() -> void:
	var target := _target_edit.text.strip_edges()
	if target.is_empty():
		_set_status("Enter a target sandbox ID first.")
		return
	if target.to_upper() == _RETAIL_LABEL:
		_on_retail_pressed()
		return
	_confirm_then(
		(
			"Switch the machine-wide Xbox sandbox to '%s'?\n\n"
			+ "This affects every signed-in user and every Xbox-aware app on "
			+ "this PC. Requires admin privileges; UAC will prompt."
		) % target,
		func() -> void: _run_switch(["/set", target, "/noApps"], target)
	)


func _on_retail_pressed() -> void:
	_confirm_then(
		(
			"Switch the machine-wide Xbox sandbox back to RETAIL?\n\n"
			+ "Required before consuming retail Xbox Live services. Requires "
			+ "admin privileges; UAC will prompt."
		),
		func() -> void: _run_switch(["/retail", "/noApps"], _RETAIL_LABEL)
	)


func _confirm_then(prompt: String, action: Callable) -> void:
	var dlg := ConfirmationDialog.new()
	dlg.title = "Confirm Sandbox Switch"
	dlg.dialog_text = prompt
	dlg.ok_button_text = "Switch"
	dlg.confirmed.connect(func() -> void:
		dlg.queue_free()
		action.call())
	dlg.canceled.connect(func() -> void: dlg.queue_free())
	add_child(dlg)
	dlg.popup_centered(Vector2i(500, 220))


func _run_switch(args: Array, target_label: String) -> void:
	if _toolchain == null:
		_set_status("Toolchain unavailable.")
		return
	var sandbox_exe := str(_toolchain.get_sandbox_path())
	if sandbox_exe.is_empty():
		_set_status("XblPCSandbox.exe not found.")
		return
	_set_actions_enabled(false)
	_set_status("Switching to %s …" % target_label)
	var packed := PackedStringArray()
	for a in args:
		packed.push_back(str(a))
	var result: Dictionary = _toolchain.execute_tool(sandbox_exe, packed)
	var exit_code := int(result.get("exit_code", -1))
	var stdout := str(result.get("stdout", "")).strip_edges()
	if exit_code == 0:
		_set_status("Switched to %s." % target_label)
		print("[GDK Sandbox] Switched to %s" % target_label)
		_set_actions_enabled(true)
		refresh_status()
	else:
		_set_status(_format_switch_failure(exit_code, stdout))
		push_error("[GDK Sandbox] Switch to %s failed (exit %d): %s" % [
			target_label, exit_code, stdout
		])
		_set_actions_enabled(true)


func _format_switch_failure(exit_code: int, stdout: String) -> String:
	return (
		"XblPCSandbox.exe failed (exit %d).\n%s\n\n"
		+ "Common causes:\n"
		+ "  • Godot was not launched as Administrator.\n"
		+ "  • UAC prompt was dismissed.\n"
		+ "  • Another Xbox-aware app is holding the sandbox state."
	) % [exit_code, stdout if not stdout.is_empty() else "(no output)"]
