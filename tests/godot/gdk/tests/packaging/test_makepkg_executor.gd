extends GutTest

const MakePkgExecutorScript = preload("res://addons/godot_gdk_packaging/core/makepkg_executor.gd")

class _FakeToolchain:
	extends RefCounted

	var makepkg_path: String
	var calls: Array[Dictionary] = []
	var exit_code: int = 0
	var stdout: String = "fake stdout"
	var stderr: String = ""

	func _init(p_makepkg_path: String) -> void:
		makepkg_path = p_makepkg_path

	func get_makepkg_path() -> String:
		return makepkg_path

	func execute_tool(exe_path: String, args: PackedStringArray) -> Dictionary:
		calls.append({"exe_path": exe_path, "args": PackedStringArray(args)})
		return {"exit_code": exit_code, "stdout": stdout, "stderr": stderr}


func test_pack_builds_expected_argv_for_all_optional_switches() -> void:
	var executor = _new_executor()
	var args: PackedStringArray = executor.build_pack_args(
		"C:\\Content Dir",
		"C:\\Maps\\layout.xml",
		"C:\\Output Dir",
		{
			"content_id": "CID-123",
			"product_id": "PID-456",
			"encrypt_key": "C:\\Keys\\key with spaces.ekb",
			"updcompat": 2,
		})

	assert_eq(_argv(args), [
		"pack", "/f", "C:\\Maps\\layout.xml",
		"/d", "C:\\Content Dir",
		"/pd", "C:\\Output Dir",
		"/pc",
		"/contentid", "CID-123",
		"/productid", "PID-456",
		"/lk", "C:\\Keys\\key with spaces.ekb",
		"/updcompat", "2",
	])


func test_pack_builds_license_encrypt_argv_when_no_key_is_supplied() -> void:
	var executor = _new_executor()
	var args: PackedStringArray = executor.build_pack_args("content", "layout.xml", "out", {
		"encrypt": true,
		"updcompat": 3,
	})

	assert_true(_argv(args).has("/l"), "license encryption switch emitted")
	assert_false(_argv(args).has("/lk"), "license mode does not emit key switch")
	assert_eq(_argv(args).slice(_argv(args).size() - 2), ["/updcompat", "3"])


func test_genmap_and_validate_build_expected_argv() -> void:
	var executor = _new_executor()

	assert_eq(_argv(executor.build_genmap_args("C:\\Content Dir", "C:\\Maps\\layout.xml")), [
		"genmap", "/f", "C:\\Maps\\layout.xml", "/d", "C:\\Content Dir",
	])
	assert_eq(_argv(executor.build_validate_args("C:\\Maps\\layout.xml", "C:\\Content Dir", "C:\\Validate Out")), [
		"validate", "/f", "C:\\Maps\\layout.xml", "/d", "C:\\Content Dir", "/pd", "C:\\Validate Out", "/pc",
	])


func test_pack_execute_uses_makepkg_path_and_propagates_tool_result() -> void:
	var fake := _FakeToolchain.new("C:\\GDK Bin\\makepkg.exe")
	fake.exit_code = 17
	fake.stdout = "pack stdout"
	fake.stderr = "pack stderr"
	var executor = MakePkgExecutorScript.new(fake)

	var result: Dictionary = executor.pack("Content", "layout.xml", "Out", {"content_id": "CID"})

	assert_eq(fake.calls.size(), 1)
	assert_eq(fake.calls[0]["exe_path"], "C:\\GDK Bin\\makepkg.exe")
	assert_eq(_argv(fake.calls[0]["args"]), ["pack", "/f", "layout.xml", "/d", "Content", "/pd", "Out", "/pc", "/contentid", "CID"])
	assert_eq(result.get("exit_code", -1), 17)
	assert_eq(result.get("stdout", ""), "pack stdout")
	assert_eq(result.get("stderr", ""), "pack stderr")


func test_genmap_and_validate_execute_propagate_tool_result_shape() -> void:
	var fake := _FakeToolchain.new("makepkg.exe")
	fake.exit_code = 4
	fake.stdout = "tool stdout"
	fake.stderr = "tool stderr"
	var executor = MakePkgExecutorScript.new(fake)

	var genmap: Dictionary = executor.genmap("Content", "layout.xml")
	var validate: Dictionary = executor.validate("layout.xml", "Content", "Out")

	assert_eq(fake.calls.size(), 2)
	assert_eq(_argv(fake.calls[0]["args"]), ["genmap", "/f", "layout.xml", "/d", "Content"])
	assert_eq(_argv(fake.calls[1]["args"]), ["validate", "/f", "layout.xml", "/d", "Content", "/pd", "Out", "/pc"])
	assert_eq(genmap.get("exit_code", -1), 4)
	assert_eq(validate.get("exit_code", -1), 4)
	assert_eq(genmap.get("stdout", ""), "tool stdout")
	assert_eq(validate.get("stderr", ""), "tool stderr")


func _new_executor() -> RefCounted:
	return MakePkgExecutorScript.new(_FakeToolchain.new("makepkg.exe"))


func _argv(args: PackedStringArray) -> Array:
	var values: Array = []
	for arg: String in args:
		values.append(arg)
	return values
