extends Node
@onready var play_fab_manager: PlayFabManager = $PlayFabManager


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	# var summator = Summator.new()
	# summator.add(3)
	# summator.add(4)
	# print(summator.get_total())
	pass # Replace with function body.

func _run_playfab() -> void:
	print($PlayFabManager.RunPlayFabSDKSample())
pass
