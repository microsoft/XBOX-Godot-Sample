## Command name → handler dispatcher for the test client.
##
## Handlers are Callables that take a Dictionary `params` and return either:
##   - a Dictionary  { ok: bool, result?: Dictionary, error?: Dictionary }
##   - an awaitable that resolves to the same shape
##
## On unknown command, returns { ok: false, error: { code: "unknown_command", ... } }.
extends RefCounted

var _handlers: Dictionary = {}


func register(command_name: String, handler: Callable) -> void:
	_handlers[command_name] = handler


func has(command_name: String) -> bool:
	return _handlers.has(command_name)


func dispatch(command_name: String, params: Dictionary) -> Dictionary:
	if not _handlers.has(command_name):
		return {
			"ok": false,
			"result": {},
			"error": {
				"code": "unknown_command",
				"message": "no handler registered for command '%s'" % command_name,
			},
		}
	var handler: Callable = _handlers[command_name]
	# Handlers may return a Dictionary, a Signal, or be a coroutine that
	# yields internally (returns a GDScriptFunctionState). In Godot 4, `await`
	# on a non-signal value returns the value as-is, on a Signal blocks on
	# emission, and on a coroutine state blocks until the function returns.
	# Awaiting unconditionally covers all three cases.
	var value: Variant = await handler.call(params)
	if value is Dictionary:
		var d: Dictionary = value
		if not d.has("ok"):
			d["ok"] = true
		if not d.has("result"):
			d["result"] = {}
		if not d.has("error"):
			d["error"] = {}
		return d
	return {
		"ok": false,
		"result": {},
		"error": {
			"code": "invalid_handler_return",
			"message": "handler for '%s' returned non-Dictionary (type=%d)" % [command_name, typeof(value)],
		},
	}
