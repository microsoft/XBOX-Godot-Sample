## Pending await for an event matching (event_type, filter) on a single client.
##
## Created by ClientProxy.expect_event(...). Scenarios then `await waiter.wait(timeout_ms, tree)`,
## which returns { ok, event, timed_out } Dictionary.
##
## Implementation polls per frame instead of racing signals; the orchestrator main
## loop is already frame-driven so the latency added is bounded by one frame
## (~16 ms at 60 fps, less at the orchestrator's typical 0-budget frame).
extends RefCounted

var event_type: String
var filter: Dictionary
var _tree: SceneTree = null
var _delivered_payload: Dictionary = {}
var _is_delivered: bool = false


func _init(p_event_type: String, p_filter: Dictionary, p_tree: SceneTree = null) -> void:
	event_type = p_event_type
	filter = p_filter
	_tree = p_tree


func matches(event_frame: Dictionary) -> bool:
	if String(event_frame.get("event_type", "")) != event_type:
		return false
	if filter.is_empty():
		return true
	var payload: Dictionary = event_frame.get("payload", {})
	for key in filter.keys():
		var dotted: String = str(key)
		var actual: Variant = _lookup(payload, dotted)
		if actual == null:
			actual = _lookup(event_frame, dotted)
		if not _values_equal(actual, filter[key]):
			return false
	return true


func deliver(event_frame: Dictionary) -> void:
	if _is_delivered:
		return
	_is_delivered = true
	_delivered_payload = event_frame


func is_delivered() -> bool:
	return _is_delivered


func wait(timeout_ms: int) -> Dictionary:
	if _is_delivered:
		return { "ok": true, "event": _delivered_payload, "timed_out": false }
	if _tree == null:
		return { "ok": false, "event": {}, "timed_out": false,
			"error": { "code": "waiter_not_bound", "message": "EventWaiter has no SceneTree bound; expect_event() must propagate the orchestrator tree" } }
	var deadline_ms: int = Time.get_ticks_msec() + timeout_ms
	while not _is_delivered:
		if Time.get_ticks_msec() >= deadline_ms:
			return { "ok": false, "event": {}, "timed_out": true }
		await _tree.process_frame
	return { "ok": true, "event": _delivered_payload, "timed_out": false }


func _lookup(source: Dictionary, dotted_key: String) -> Variant:
	var parts: PackedStringArray = dotted_key.split(".")
	var cursor: Variant = source
	for part in parts:
		if typeof(cursor) != TYPE_DICTIONARY:
			return null
		var d: Dictionary = cursor
		if not d.has(part):
			return null
		cursor = d[part]
	return cursor


func _values_equal(actual: Variant, expected: Variant) -> bool:
	if typeof(actual) == TYPE_STRING or typeof(expected) == TYPE_STRING:
		return str(actual) == str(expected)
	return actual == expected
