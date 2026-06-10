extends RefCounted

# Keeps sample scripts parseable before CMake mirrors native addons into this project.
static func singleton(name: String) -> Object:
	if Engine.has_singleton(name):
		return Engine.get_singleton(name)
	return null

static func instantiate(native_class: String) -> Object:
	if not ClassDB.class_exists(native_class) or not ClassDB.can_instantiate(native_class):
		push_error("Native class %s is unavailable. Build and enable the addon first." % native_class)
		return null
	return ClassDB.instantiate(native_class)

static func constant(native_class: String, constant_name: String, fallback: int = 0) -> int:
	if ClassDB.class_exists(native_class) and ClassDB.class_has_integer_constant(native_class, constant_name):
		return ClassDB.class_get_integer_constant(native_class, constant_name)
	push_warning("Native constant %s.%s is unavailable. Build and enable the addon first." % [native_class, constant_name])
	return fallback
