## Length-prefixed JSON frame codec.
##
## Mirror of tests/godot/mp_orchestrator/scripts/frame_codec.gd. Keep both copies
## in sync byte-for-byte; a later checkpoint may consolidate them.
extends RefCounted

const MAX_FRAME_SIZE: int = 4 * 1024 * 1024

var _read_buffer: PackedByteArray = PackedByteArray()


static func encode(frame: Dictionary) -> PackedByteArray:
	var json_text: String = JSON.stringify(frame)
	var json_bytes: PackedByteArray = json_text.to_utf8_buffer()
	var size: int = json_bytes.size()
	var result: PackedByteArray = PackedByteArray()
	result.resize(4 + size)
	result[0] = (size >> 24) & 0xff
	result[1] = (size >> 16) & 0xff
	result[2] = (size >> 8) & 0xff
	result[3] = size & 0xff
	for i in range(size):
		result[4 + i] = json_bytes[i]
	return result


func feed(bytes: PackedByteArray) -> void:
	if bytes.size() == 0:
		return
	_read_buffer.append_array(bytes)


func try_pop_frame() -> Dictionary:
	if _read_buffer.size() < 4:
		return { "status": "empty" }

	var length: int = (
		(_read_buffer[0] << 24)
		| (_read_buffer[1] << 16)
		| (_read_buffer[2] << 8)
		| _read_buffer[3]
	)

	if length < 0 or length > MAX_FRAME_SIZE:
		return {
			"status": "error",
			"reason": "frame_too_large",
			"detail": length,
		}

	if _read_buffer.size() < 4 + length:
		return { "status": "empty" }

	var payload_bytes: PackedByteArray = _read_buffer.slice(4, 4 + length)
	_read_buffer = _read_buffer.slice(4 + length)

	var json_text: String = payload_bytes.get_string_from_utf8()
	var parsed: Variant = JSON.parse_string(json_text)
	if typeof(parsed) != TYPE_DICTIONARY:
		return {
			"status": "error",
			"reason": "invalid_json",
			"detail": json_text.substr(0, 256),
		}

	return { "status": "frame", "frame": parsed }


func buffered_byte_count() -> int:
	return _read_buffer.size()
