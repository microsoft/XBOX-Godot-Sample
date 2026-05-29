## Writes mp-test-results.{json,md} after a scenario run.
extends RefCounted

const STATUS_PASSED: String = "passed"
const STATUS_FAILED: String = "failed"
const STATUS_SKIPPED: String = "skipped"
const STATUS_QUARANTINED_FAILURE: String = "quarantined_failure"
const STATUS_INVALID: String = "invalid_metadata"

var run_id: String = ""
var orchestrator_version: String = "0.1.0"
var started_at_unix: int = 0
var finished_at_unix: int = 0
var scenarios: Array[Dictionary] = []


func add_scenario(entry: Dictionary) -> void:
	scenarios.append(entry)


func summary() -> Dictionary:
	var counts: Dictionary = {
		STATUS_PASSED: 0,
		STATUS_FAILED: 0,
		STATUS_SKIPPED: 0,
		STATUS_QUARANTINED_FAILURE: 0,
		STATUS_INVALID: 0,
	}
	for entry in scenarios:
		var status: String = String(entry.get("status", ""))
		counts[status] = int(counts.get(status, 0)) + 1
	return {
		"total": scenarios.size(),
		"passed": counts[STATUS_PASSED],
		"failed": counts[STATUS_FAILED],
		"skipped": counts[STATUS_SKIPPED],
		"quarantined_failures": counts[STATUS_QUARANTINED_FAILURE],
		"invalid": counts[STATUS_INVALID],
	}


func write_json(path: String) -> int:
	var payload: Dictionary = {
		"run_id": run_id,
		"orchestrator_version": orchestrator_version,
		"started_at_unix": started_at_unix,
		"finished_at_unix": finished_at_unix,
		"summary": summary(),
		"scenarios": scenarios,
	}
	var text: String = JSON.stringify(payload, "  ")
	var dir_path: String = path.get_base_dir()
	if not DirAccess.dir_exists_absolute(dir_path):
		DirAccess.make_dir_recursive_absolute(dir_path)
	var f: FileAccess = FileAccess.open(path, FileAccess.WRITE)
	if f == null:
		return FileAccess.get_open_error()
	f.store_string(text)
	f.close()
	return OK


func write_markdown(path: String) -> int:
	var summary_block: Dictionary = summary()
	var lines: Array[String] = []
	lines.append("# MP Test Orchestrator Results")
	lines.append("")
	lines.append("- run_id: `%s`" % run_id)
	lines.append("- orchestrator: `%s`" % orchestrator_version)
	lines.append("- started: %d" % started_at_unix)
	lines.append("- finished: %d" % finished_at_unix)
	lines.append("")
	lines.append("| Metric | Count |")
	lines.append("| --- | --- |")
	lines.append("| Total | %d |" % int(summary_block.total))
	lines.append("| Passed | %d |" % int(summary_block.passed))
	lines.append("| Failed | %d |" % int(summary_block.failed))
	lines.append("| Skipped | %d |" % int(summary_block.skipped))
	lines.append("| Quarantined failures | %d |" % int(summary_block.quarantined_failures))
	lines.append("| Invalid metadata | %d |" % int(summary_block.invalid))
	lines.append("")
	lines.append("## Scenarios")
	lines.append("")
	lines.append("| ID | Name | Status | Duration (ms) | Failure reason |")
	lines.append("| --- | --- | --- | --- | --- |")
	for entry in scenarios:
		lines.append("| `%s` | %s | `%s` | %d | %s |" % [
			String(entry.get("id", "?")),
			String(entry.get("name", "?")).replace("|", "\\|"),
			String(entry.get("status", "?")),
			int(entry.get("duration_ms", 0)),
			String(entry.get("failure_reason", "")).replace("|", "\\|"),
		])
	var text: String = "\n".join(lines) + "\n"
	var dir_path: String = path.get_base_dir()
	if not DirAccess.dir_exists_absolute(dir_path):
		DirAccess.make_dir_recursive_absolute(dir_path)
	var f: FileAccess = FileAccess.open(path, FileAccess.WRITE)
	if f == null:
		return FileAccess.get_open_error()
	f.store_string(text)
	f.close()
	return OK


func has_failures(allow_quarantined: bool = true) -> bool:
	var s: Dictionary = summary()
	var fail_count: int = int(s.failed) + int(s.invalid)
	if not allow_quarantined:
		fail_count += int(s.quarantined_failures)
	return fail_count > 0
