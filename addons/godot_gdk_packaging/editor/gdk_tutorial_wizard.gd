@tool
extends Window
## GDK Tutorial Wizard — a slide-based walkthrough of the GDK editor tools.

const TutorialWizardState = preload("res://addons/godot_gdk_packaging/editor/tutorial_wizard_state.gd")

var _current_slide: int = 0
var _slides: Array[Dictionary] = []
var _title_label: Label
var _body_label: RichTextLabel
var _slide_counter: Label
var _prev_btn: Button
var _next_btn: Button
var _close_btn: Button

func _ready() -> void:
	title = "XBOX Godot Sample — Getting Started"
	size = Vector2i(750, 560)
	exclusive = false
	_build_slides()
	_build_ui()
	_show_slide(0)

func _build_slides() -> void:
	_slides = [
		{
			"title": "Welcome to the XBOX Godot Sample",
			"body": """[font_size=16][color=#107c10][b]XBOX Godot Sample[/b][/color] — the official Godot integration sample for the Microsoft Game Development Kit.[/font_size]

This addon brings Xbox PC development tools directly into the Godot Editor through the [b]Microsoft GDK[/b] menu in the editor menu bar — from configuring your game identity, to exporting, packaging, installing, and launching your builds. The companion [b]XBOX Godot Sample Tutorial App[/b] under [b]sample/tutorial_app[/b] is a ready-to-run reference that exercises every surface this wizard covers.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=15][b]What this wizard covers:[/b][/font_size]

    [color=#107c10]▸[/color]  [b]Config[/b] — Set up your MicrosoftGame.config and store logos
    [color=#107c10]▸[/color]  [b]Sandbox[/b] — Switch the PC's Xbox sandbox for test accounts
    [color=#107c10]▸[/color]  [b]Sample app[/b] — Explore the tutorial app shipped with the addon
    [color=#107c10]▸[/color]  [b]Export & Package[/b] — Export your game and create MSIXVC packages
    [color=#107c10]▸[/color]  [b]Package Manager[/b] — Install, uninstall, and launch builds
    [color=#107c10]▸[/color]  [b]PlayFab[/b] — Point the runtime at your PlayFab title

[color=gray]The wizard is informational — it never changes project files or machine state. Navigate with the buttons below, or close it at any time.[/color]"""
		},
		{
			"title": "⚙️  Config",
			"body": """[font_size=15][b]Your game's identity starts here.[/b][/font_size]

The [b]MicrosoftGame.config[/b] file defines your game's name, publisher, logos, and Xbox Live IDs. Every Microsoft GDK game needs one.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Menu actions[/b][/font_size]
[color=#107c10]▸[/color]  [b]Microsoft GDK → Create MicrosoftGame.config[/b] — Writes a starter config (and placeholder logos at the project root) when none exists, then opens Microsoft's [b]GameConfigEditor[/b]
[color=#107c10]▸[/color]  [b]Microsoft GDK → Edit MicrosoftGame.config[/b] — The same item relabels itself once the file exists, and reopens GameConfigEditor on it

[font_size=14][b]Fill in at minimum[/b][/font_size]
[color=#107c10]▸[/color]  [b]Identity / Name[/b], [b]Title Id[/b], and [b]Store Id[/b] from your Partner Center title
[color=#107c10]▸[/color]  A [b]Version[/b] such as [b]1.0.0.0[/b]

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tip:[/color] Commit [b]MicrosoftGame.config[/b] to source control so the whole team shares the same identity. Use [b]GameConfigEditor Reference[/b] in the menu for Microsoft's field-by-field docs."""
		},
		{
			"title": "🔒  Sandbox",
			"body": """[font_size=15][b]Switch the PC's Xbox sandbox.[/b][/font_size]

Xbox Live services run in isolated sandboxes. Test accounts only authenticate against the sandbox they were created in, so your PC must be in the matching development sandbox.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Microsoft GDK → Change Sandbox…[/b][/font_size]
[color=#107c10]▸[/color]  Shows the current sandbox (e.g., [b]XDKS.1[/b] or [b]RETAIL[/b])
[color=#107c10]▸[/color]  [b]Set Sandbox[/b] — Switches to a development sandbox via [i]XblPCSandbox.exe[/i]
[color=#107c10]▸[/color]  [b]Switch to RETAIL[/b] — Returns the PC to consumer Xbox Live

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tips:[/color]
•  Switching the sandbox is [b]machine-wide[/b] and requires [b]administrator privileges[/b]
•  Your PC sandbox must match your test account's sandbox
•  Switch back to RETAIL before updating the Xbox App or Gaming Services"""
		},
		{
			"title": "🎮  Sample app",
			"body": """[font_size=15][b]A ready-to-run tutorial app ships with the addon.[/b][/font_size]

The [b]sample/tutorial_app[/b] project under the repo root is the finished version of every tutorial in the documentation chain (T1 → T8), wired up against the GDK and PlayFab addons. Use it as a reference when your own project drifts from the tutorial — open the matching scene and compare.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Run it[/b][/font_size]
[color=#107c10]▸[/color]  Build the addons once so the sample's mirrored [b]addons/[/b] are populated: [code]cmake --build build --preset debug[/code]
[color=#107c10]▸[/color]  Open [b]sample/tutorial_app/project.godot[/b] in the Godot editor
[color=#107c10]▸[/color]  Run [code]pwsh -File .\\tools\\setup_sample.ps1[/code] once to fill in your Partner Center / PlayFab identifiers
[color=#107c10]▸[/color]  Press [b]F5[/b] — the default scene is a tutorial picker; each button loads one tutorial's scene

[font_size=14][b]Tutorial scenes[/b][/font_size]
[color=#107c10]▸[/color]  [b]t01_signin[/b] — Sign in a local Xbox user
[color=#107c10]▸[/color]  [b]t02_achievement[/b] — Unlock an achievement
[color=#107c10]▸[/color]  [b]t03_leaderboard[/b] — Post and read a PlayFab leaderboard score
[color=#107c10]▸[/color]  [b]t04_game_saves[/b] — Read and write a PlayFab Game Save
[color=#107c10]▸[/color]  [b]t05_lobby[/b] — Create and join a PlayFab multiplayer lobby
[color=#107c10]▸[/color]  [b]t06_mpa[/b] — Multiplayer Activity surfaces (invites, join-in-progress)
[color=#107c10]▸[/color]  [b]t07_party[/b] — PlayFab Party text and voice chat
[color=#107c10]▸[/color]  [b]t08_integration[/b] — End-to-end tech demo combining T1–T7

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tip:[/color] The full per-tutorial walkthroughs live in [b]docs/tutorials/[/b]. The picker scene is [b]sample/tutorial_app/shared/tutorial_picker.tscn[/b]."""
		},
		{
			"title": "📦  Export & Package",
			"body": """[font_size=15][b]Export your project and build an MSIXVC package.[/b][/font_size]

Exporting and packaging run through Godot's [b]Project → Export…[/b] dialog and the addon's headless [b]gdkpkg[/b] runner — there is no separate dock.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Export[/b][/font_size]
[color=#107c10]▸[/color]  Add a [b]Windows Desktop[/b] preset in [b]Project → Export…[/b]
[color=#107c10]▸[/color]  [code]gdkpkg export[/code] — Exports and prepares content; the addon copies MicrosoftGame.config, patches the exe name, adds the VC14 dependency, and copies logos

[font_size=14][b]Package (headless gdkpkg verbs)[/b][/font_size]
[color=#107c10]▸[/color]  [code]gdkpkg genmap[/code] — Create the layout mapping file
[color=#107c10]▸[/color]  [code]gdkpkg pack[/code] — Create an MSIXVC package from the content dir
[color=#107c10]▸[/color]  [code]gdkpkg validate[/code] — Run the Submission Validator without packaging

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tip:[/color] Run [code]gdkpkg help[/code] for the full verb and flag list. The [b]makepkg Reference[/b] and [b]PC Packaging Overview[/b] menu items link Microsoft's docs."""
		},
		{
			"title": "🚀  Package Manager",
			"body": """[font_size=15][b]Install, uninstall, and launch builds on this PC.[/b][/font_size]

[b]Microsoft GDK → Package Manager…[/b] opens a machine-wide view of every package registered with [i]wdapp.exe[/i] — not just the current project.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Actions[/b][/font_size]
[color=#107c10]▸[/color]  [b]Install[/b] — Installs a [b].msixvc[/b] you pick from disk
[color=#107c10]▸[/color]  [b]Uninstall[/b] — Removes the selected registered package
[color=#107c10]▸[/color]  [b]Refresh[/b] — Re-queries [i]wdapp[/i] for the current package list
[color=#107c10]▸[/color]  [b]Export…[/b] — Jumps to Godot's [b]Project → Export…[/b] dialog

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tips:[/color]
•  Only one [i]wdapp[/i] operation runs at a time; buttons disable while an op is in flight
•  For fast iteration, register a loose [b]Build/[/b] folder with [code]gdkpkg register_loose[/code] instead of packing
•  Launch and terminate are also available as [code]gdkpkg launch[/code] / [code]gdkpkg terminate[/code]"""
		},
		{
			"title": "☁️  PlayFab",
			"body": """[font_size=15][b]Point the runtime at your PlayFab title.[/b][/font_size]

PlayFab-backed features read the Title ID from Project Settings. Set it once and the [b]godot_playfab[/b] runtime derives the endpoint automatically.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Set the Title ID[/b][/font_size]
In [b]Project → Project Settings → General[/b] (enable [i]Advanced Settings[/i]) set [code]playfab/runtime/title_id[/code] to your Title ID from [b]Game Manager → Settings → API features[/b]. Leave [code]playfab/runtime/endpoint[/code] blank to use the default.

[font_size=14][b]Runtime usage[/b][/font_size]
[code]var title_id := str(ProjectSettings.get_setting("playfab/runtime/title_id", ""))[/code]

[font_size=14][b]Menu shortcuts[/b][/font_size]
[color=#107c10]▸[/color]  [b]PlayFab Game Manager[/b] — Opens the PlayFab portal
[color=#107c10]▸[/color]  [b]PlayFab + Microsoft GDK Quickstart[/b] — Microsoft's integration docs

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tip:[/color] These values live in [i]project.godot[/i] so the PlayFab runtime reads them directly at startup."""
		},
		{
			"title": "You're Ready!",
			"body": """[font_size=16][color=#107c10][b]Recommended Workflow[/b][/color][/font_size]

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=15][b]Step 1[/b]  ⚙️  Create your MicrosoftGame.config and set your tile image[/font_size]

[font_size=15][b]Step 2[/b]  🔒  Switch the PC to your development sandbox and sign in a test account[/font_size]

[font_size=15][b]Step 3[/b]  🎮  Open [b]sample/tutorial_app[/b] and run a tutorial scene to see the addons in action[/font_size]

[font_size=15][b]Step 4[/b]  📦  Add a Windows Desktop export preset and run [code]gdkpkg export[/code][/font_size]

[font_size=15][b]Step 5[/b]  🚀  Install and launch your build from [b]Package Manager…[/b][/font_size]

[font_size=15][b]Step 6[/b]  📦  Run [code]gdkpkg pack[/code] (and [code]validate[/code]) when ready for distribution[/font_size]

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Documentation[/b] (available in the Microsoft GDK menu):[/font_size]
    [color=#107c10]▸[/color]  PC Packaging Overview
    [color=#107c10]▸[/color]  makepkg Reference
    [color=#107c10]▸[/color]  GameConfigEditor Reference
    [color=#107c10]▸[/color]  Achievements Guide
    [color=#107c10]▸[/color]  PlayFab Game Manager & Quickstart

[color=gray]For the full setup walkthrough see [b]docs/getting-started.md[/b] and the [b]sample/tutorial_app[/b] project. Reopen this wizard anytime from [b]Microsoft GDK → Getting Started[/b].[/color]"""
		},
	]

func _build_ui() -> void:
	var panel: PanelContainer = PanelContainer.new()
	panel.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(panel)

	var margin: MarginContainer = MarginContainer.new()
	margin.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	margin.add_theme_constant_override("margin_left", 24)
	margin.add_theme_constant_override("margin_right", 24)
	margin.add_theme_constant_override("margin_top", 18)
	margin.add_theme_constant_override("margin_bottom", 12)
	panel.add_child(margin)

	var vbox: VBoxContainer = VBoxContainer.new()
	vbox.size_flags_vertical = Control.SIZE_EXPAND_FILL
	margin.add_child(vbox)

	_title_label = Label.new()
	_title_label.add_theme_font_size_override("font_size", 24)
	vbox.add_child(_title_label)

	var spacer: Control = Control.new()
	spacer.custom_minimum_size.y = 6
	vbox.add_child(spacer)

	_body_label = RichTextLabel.new()
	_body_label.bbcode_enabled = true
	_body_label.fit_content = false
	_body_label.scroll_active = true
	_body_label.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_body_label.add_theme_font_size_override("normal_font_size", 14)
	_body_label.add_theme_font_size_override("bold_font_size", 14)
	vbox.add_child(_body_label)

	var nav_sep: HSeparator = HSeparator.new()
	vbox.add_child(nav_sep)

	var nav_row: HBoxContainer = HBoxContainer.new()
	nav_row.add_theme_constant_override("separation", 8)
	vbox.add_child(nav_row)

	_prev_btn = Button.new()
	_prev_btn.text = "  ← Previous  "
	_prev_btn.pressed.connect(_on_prev)
	nav_row.add_child(_prev_btn)

	_slide_counter = Label.new()
	_slide_counter.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_slide_counter.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_slide_counter.add_theme_font_size_override("font_size", 13)
	_slide_counter.add_theme_color_override("font_color", Color(0.6, 0.6, 0.6))
	nav_row.add_child(_slide_counter)

	_next_btn = Button.new()
	_next_btn.text = "  Next →  "
	_next_btn.pressed.connect(_on_next)
	nav_row.add_child(_next_btn)

	_close_btn = Button.new()
	_close_btn.text = "  Close  "
	_close_btn.pressed.connect(_on_close)
	nav_row.add_child(_close_btn)

	close_requested.connect(_on_close)

func _show_slide(index: int) -> void:
	_current_slide = TutorialWizardState.clamp_slide_index(index, _slides.size())
	var slide: Dictionary = _slides[_current_slide]
	_title_label.text = slide["title"]
	_body_label.text = slide["body"]
	_slide_counter.text = TutorialWizardState.format_counter(_current_slide, _slides.size())
	_prev_btn.disabled = TutorialWizardState.is_first(_current_slide, _slides.size())
	_next_btn.text = TutorialWizardState.next_button_label(_current_slide, _slides.size())

func _on_prev() -> void:
	_show_slide(TutorialWizardState.prev_slide_index(_current_slide, _slides.size()))

func _on_next() -> void:
	if TutorialWizardState.is_last(_current_slide, _slides.size()):
		_on_close()
	else:
		_show_slide(TutorialWizardState.next_slide_index(_current_slide, _slides.size()))

func _on_close() -> void:
	hide()
	queue_free()
