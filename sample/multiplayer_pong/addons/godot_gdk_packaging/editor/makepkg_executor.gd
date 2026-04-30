@tool
extends RefCounted
## Builds and executes makepkg.exe subcommands: pack, genmap, validate.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/editor/gdk_toolchain.gd")

var _toolchain: RefCounted


func _init(toolchain: RefCounted) -> void:
	_toolchain = toolchain


# ── pack ────────────────────────────────────────────────────────────────────

## Creates an MSIXVC package for PC.
##
## Required: source_dir, map_file, output_dir
## Optional keys in [param options]:
##   content_id  : String  — /contentid value
##   product_id  : String  — /productid value
##   encrypt     : bool    — if true, adds /l (license-encrypt)
##   encrypt_key : String  — path to EKB file; adds /lk <file> (overrides /l)
##   updcompat   : int     — /updcompat level (1, 2, or 3; default 3)
func pack(source_dir: String, map_file: String, output_dir: String,
		options: Dictionary = {}) -> Dictionary:
	var args: PackedStringArray = ["pack"]
	args.append("/f")
	args.append(map_file)
	args.append("/d")
	args.append(source_dir)
	args.append("/pd")
	args.append(output_dir)
	args.append("/pc")

	if options.has("content_id") and options["content_id"] != "":
		args.append("/contentid")
		args.append(str(options["content_id"]))

	if options.has("product_id") and options["product_id"] != "":
		args.append("/productid")
		args.append(str(options["product_id"]))

	# Encryption: /lk takes precedence over /l
	if options.has("encrypt_key") and options["encrypt_key"] != "":
		args.append("/lk")
		args.append(str(options["encrypt_key"]))
	elif options.get("encrypt", false):
		args.append("/l")

	if options.has("updcompat"):
		args.append("/updcompat")
		args.append(str(options["updcompat"]))

	print("[GDK Packaging] makepkg ", " ".join(args))
	return _toolchain.execute_tool(_toolchain.get_makepkg_path(), args)


# ── genmap ──────────────────────────────────────────────────────────────────

## Generates a mapping XML file from a content directory.
func genmap(content_dir: String, output_file: String) -> Dictionary:
	var args: PackedStringArray = [
		"genmap",
		"/f", output_file,
		"/d", content_dir,
	]
	print("[GDK Packaging] makepkg ", " ".join(args))
	return _toolchain.execute_tool(_toolchain.get_makepkg_path(), args)


# ── validate ────────────────────────────────────────────────────────────────

## Validates a package layout without creating it.
func validate(map_file: String, source_dir: String, output_dir: String) -> Dictionary:
	var args: PackedStringArray = [
		"validate",
		"/f", map_file,
		"/d", source_dir,
		"/pd", output_dir,
		"/pc",
	]
	print("[GDK Packaging] makepkg ", " ".join(args))
	return _toolchain.execute_tool(_toolchain.get_makepkg_path(), args)
