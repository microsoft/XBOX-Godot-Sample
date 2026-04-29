extends Node

@onready var play_fab_manager = $PlayFabManager

func _run_playfab() -> void:
	print(play_fab_manager.RunPlayFabSDKSample())
