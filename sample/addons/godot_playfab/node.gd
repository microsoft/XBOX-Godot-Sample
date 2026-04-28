extends Node
@onready var play_fab_manager: PlayFabManager = $PlayFabManager


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.

func _run_playfab() -> void:
	print($PlayFabManager.RunPlayFabSDKSample())
pass
