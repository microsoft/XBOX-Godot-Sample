extends SceneTree

const TestOrchestrator := preload("res://scripts/test_orchestrator.gd")

var _orch: TestOrchestrator = null


func _initialize() -> void:
	_orch = TestOrchestrator.new()
	_orch.bind_tree(self)
	call_deferred("_run")


func _run() -> void:
	var exit_code: int = await _orch.run_async()
	quit(exit_code)
