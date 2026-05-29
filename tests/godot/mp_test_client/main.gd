extends SceneTree

const TestClient := preload("res://scripts/test_client.gd")

var _client: TestClient = null


func _initialize() -> void:
	_client = TestClient.new()
	_client.bind_tree(self)
	call_deferred("_run")


func _run() -> void:
	var exit_code: int = await _client.run_async()
	quit(exit_code)
