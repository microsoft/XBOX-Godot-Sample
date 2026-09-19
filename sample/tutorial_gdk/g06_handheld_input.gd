extends Control

const AddonApi = preload("res://shared/addon_api.gd")

## GDK Tutorial 6 reference scene — Windows handheld detection and in-place
## virtual-keyboard input. These baseline Windows calls do not require
## GDK.initialize() or a signed-in Xbox user.

@onready var _device_status: Label = $Root/DeviceStatus
@onready var _text_input: LineEdit = $Root/TextInput
@onready var _result: Label = $Root/Result
@onready var _show_btn: Button = $Root/Buttons/ShowBtn
@onready var _hide_btn: Button = $Root/Buttons/HideBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn

var _gdk: Object = null

func _ready() -> void:
	_show_btn.pressed.connect(_on_show_pressed)
	_hide_btn.pressed.connect(_on_hide_pressed)
	_back_btn.pressed.connect(_on_back_pressed)

	_gdk = AddonApi.singleton("GDK")
	if _gdk == null:
		_device_status.text = "GDK extension is not loaded."
		_show_btn.disabled = true
		_hide_btn.disabled = true
		return

	var handheld: bool = _gdk.system.is_handheld()
	_device_status.text = "Windows gaming handheld: %s" % ("yes" if handheld else "no")
	_text_input.grab_focus()

func _on_show_pressed() -> void:
	# The input pane targets the currently focused control. Button activation
	# moves focus, so restore the LineEdit and allow one frame for it to settle.
	_text_input.grab_focus()
	await get_tree().process_frame
	var result = _gdk.game_ui.show_virtual_keyboard()
	_report_result("Show", result)

func _on_hide_pressed() -> void:
	var result = _gdk.game_ui.hide_virtual_keyboard()
	_report_result("Hide", result)

func _report_result(action: String, result) -> void:
	if result.ok:
		_result.text = "%s request accepted by Windows: %s" % [action, result.data]
	else:
		_result.text = "%s failed: %s (%s)" % [action, result.message, result.code]

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
