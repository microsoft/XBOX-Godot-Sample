extends Control

## Shared placeholder script for Commit A scenes. Replaced by the real
## tutorial scene script in subsequent commits.

@export var tutorial_title: String = "Tutorial — placeholder"
@export var tutorial_subtitle: String = "Real content lands in a subsequent commit."

@onready var _title: Label = $Root/Title
@onready var _subtitle: Label = $Root/Subtitle
@onready var _back: Button = $Root/Back

func _ready() -> void:
	_title.text = tutorial_title
	_subtitle.text = tutorial_subtitle
	_back.pressed.connect(_on_back_pressed)

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
