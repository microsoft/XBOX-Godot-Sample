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
	title = "Microsoft GDK for Godot — Getting Started"
	size = Vector2i(750, 560)
	exclusive = false
	_build_slides()
	_build_ui()
	_show_slide(0)

func _build_slides() -> void:
	_slides = [
		{
			"title": "Welcome to the Microsoft GDK for Godot",
			"body": """[font_size=16][color=#107c10][b]Microsoft Game Development Kit[/b][/color] integration for the Godot Engine.[/font_size]

This addon brings Xbox PC development tools directly into the Godot Editor — from configuring your game identity, to exporting, packaging, and launching your builds.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=15][b]What this wizard covers:[/b][/font_size]

    [color=#107c10]▸[/color]  [b]Config[/b] — Set up your MicrosoftGame.config and store logos
    [color=#107c10]▸[/color]  [b]Sandbox[/b] — Switch Xbox sandboxes and manage test accounts
    [color=#107c10]▸[/color]  [b]Export & Package[/b] — Export your game and create MSIXVC packages
    [color=#107c10]▸[/color]  [b]Install & Launch[/b] — Install, launch, and terminate builds
    [color=#107c10]▸[/color]  [b]Achievements[/b] — Configure achievement testing
    [color=#107c10]▸[/color]  [b]PlayFab[/b] — Connect to PlayFab services

[color=gray]Navigate with the buttons below, or close this window at any time.[/color]"""
		},
		{
			"title": "⚙️  Config",
			"body": """[font_size=15][b]Your game's identity starts here.[/b][/font_size]

The [b]MicrosoftGame.config[/b] file defines your game's name, publisher, logos, and Xbox Live IDs. Every GDK game needs one.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Actions[/b][/font_size]
[color=#107c10]▸[/color]  [b]GDK → Create MicrosoftGame.config[/b] — Generates a template when the file is missing
[color=#107c10]▸[/color]  [b]GDK → Edit MicrosoftGame.config[/b] — Opens Microsoft's visual config editor when the file exists

[font_size=14][b]Config Preview[/b][/font_size]
All parsed values are shown with [i]hover tooltips[/i] explaining each field from the MicrosoftGame.config schema.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tip:[/color] After changing the tile image in GameConfigEditor, click [b]Refresh[/b] in Godot. The plugin auto-detects logo files at the project root and moves them to [b]storelogos/[/b]."""
		},
		{
			"title": "🔒  Sandbox",
			"body": """[font_size=15][b]Xbox sandbox and account management.[/b][/font_size]

Xbox Live services run in isolated sandboxes. Your PC must be in a development sandbox to use test accounts.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]PC Sandbox[/b][/font_size]
[color=#107c10]▸[/color]  Shows the current sandbox (e.g., [b]XDKS.1[/b] or [b]RETAIL[/b])
[color=#107c10]▸[/color]  [b]Set Sandbox[/b] — Switches to a development sandbox
[color=#107c10]▸[/color]  [b]Switch to RETAIL[/b] — Returns to the consumer sandbox

[font_size=14][b]Partner Center Account[/b][/font_size]
[color=#107c10]▸[/color]  Shows the signed-in developer email
[color=#107c10]▸[/color]  [b]Sign In / Sign Out[/b] — Partner Center authentication
[color=#107c10]▸[/color]  [b]Test Accounts[/b] — Opens the Xbox Live Test Account Manager

[font_size=14][b]Active Test Account[/b][/font_size]
Track which test account gamertag you're currently signed into via the Xbox App.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tips:[/color]
•  Setting the sandbox requires [b]administrator privileges[/b]
•  Your PC sandbox must match your test account's sandbox
•  Switch to RETAIL before updating the Xbox App or Gaming Services"""
		},
		{
			"title": "📦  Export & Package",
			"body": """[font_size=15][b]Build, export, and package your game for distribution.[/b][/font_size]

This tab handles the full pipeline — from exporting your Godot project to creating a distributable MSIXVC package.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Export Presets & Headless Actions[/b][/font_size]
[color=#107c10]▸[/color]  Select a [b]Windows Desktop[/b] export preset from the dropdown
[color=#107c10]▸[/color]  [b]gdkpkg export[/b] — Exports to the [b]Build/[/b] folder and prepares content
[color=#107c10]▸[/color]  [b]gdkpkg register_loose[/b] — Registers an existing Build/ folder with wdapp

[font_size=14][b]Packaging Actions[/b][/font_size]
[color=#107c10]▸[/color]  [b]gdkpkg genmap[/b] — Create layout.xml mapping file
[color=#107c10]▸[/color]  [b]gdkpkg pack[/b] — Create an MSIXVC package from Build/
[color=#107c10]▸[/color]  [b]gdkpkg validate[/b] — Run the Submission Validator without packaging

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tips:[/color]
•  You need a [b]Windows Desktop export preset[/b] in Project → Export
•  The plugin automatically copies MicrosoftGame.config, patches the exe name, adds the VC14 dependency, and copies logos to the Build/ folder
•  Use [b]📂[/b] buttons to open Content/Output directories"""
		},
		{
			"title": "🚀  Install & Launch",
			"body": """[font_size=15][b]Install, launch, and manage your builds.[/b][/font_size]

After exporting or packaging, use this tab to install and run your game.

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Install[/b][/font_size]
[color=#107c10]▸[/color]  [b]Install[/b] — Installs the .msixvc package from the Package/ folder
[color=#107c10]▸[/color]  [b]Uninstall[/b] — Removes the selected registered app

[font_size=14][b]Launch[/b][/font_size]
[color=#107c10]▸[/color]  [b]Registered App[/b] — Dropdown listing all registered/installed apps
[color=#107c10]▸[/color]  [b]Refresh[/b] — Queries [i]wdapp list[/i] to update the app list
[color=#107c10]▸[/color]  [b]Launch[/b] — Starts the selected app
[color=#107c10]▸[/color]  [b]Terminate[/b] — Stops the running app

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tips:[/color]
•  After exporting and registering, click [b]Refresh[/b] to see your app
•  Terminate uses [i]wdapp terminate[/i] for packaged builds and [i]taskkill[/i] for loose builds
•  Select the app you want from the dropdown before clicking Launch"""
		},
		{
			"title": "🏆  Achievements",
			"body": """[font_size=15][b]Configure achievement testing for the sample project.[/b][/font_size]

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Demo Achievement ID[/b][/font_size]
Enter the achievement ID you want to test (e.g., [b]1[/b]) and click [b]Save[/b]. The value is stored in [b]sample_config.cfg[/b] and read at runtime by the sample.

[font_size=14][b]Setup Requirements[/b][/font_size]
[color=#107c10]▸[/color]  Configure achievements in [b]Partner Center → Xbox Live → Achievements[/b]
[color=#107c10]▸[/color]  Publish to your development sandbox
[color=#107c10]▸[/color]  Sign in with a test account in the matching sandbox

[font_size=14][b]Resetting Achievements[/b][/font_size]
Use the included helper script to wipe achievement progress:
[code].\tools\reset_player_data.ps1[/code]

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tip:[/color] Resets only work on Xbox test accounts in a development sandbox. Restart the game after resetting."""
		},
		{
			"title": "☁️  PlayFab",
			"body": """[font_size=15][b]Connect your game to Azure PlayFab services.[/b][/font_size]

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]PlayFab Title ID[/b][/font_size]
Enter your Title ID from [b]Game Manager → Settings → API Keys[/b] and click [b]Save[/b].
You can also set an optional endpoint override; leaving it blank uses the default endpoint derived from the Title ID.

[font_size=14][b]Runtime Usage[/b][/font_size]
[code]var title_id = str(ProjectSettings.get_setting("playfab/runtime/title_id", ""))
var endpoint: String = str(ProjectSettings.get_setting("playfab/runtime/endpoint", ""))[/code]

[font_size=14][b]Tools[/b][/font_size]
[color=#107c10]▸[/color]  [b]Open Game Manager[/b] — Opens the PlayFab portal
[color=#107c10]▸[/color]  [b]SDK Version[/b] — Shows the detected PlayFab SDK version from PlayFabCore.dll

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[color=#d4830b]⚠  Tip:[/color] These values are stored in [i]project.godot[/i] so the PlayFab runtime can read them directly."""
		},
		{
			"title": "You're Ready!",
			"body": """[font_size=16][color=#107c10][b]Recommended Workflow[/b][/color][/font_size]

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=15][b]Step 1[/b]  ⚙️  Create your MicrosoftGame.config and set your tile image[/font_size]

[font_size=15][b]Step 2[/b]  🔒  Set your dev sandbox and sign into Partner Center[/font_size]

[font_size=15][b]Step 3[/b]  📦  Click [b]Export + Register[/b] for fast testing[/font_size]

[font_size=15][b]Step 4[/b]  🚀  Launch your registered build from Install & Launch[/font_size]

[font_size=15][b]Step 5[/b]  📦  Click [b]Export & Package[/b] when ready for distribution[/font_size]

[color=#107c10]━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━[/color]

[font_size=14][b]Documentation[/b] (available in the GDK menu):[/font_size]
    [color=#107c10]▸[/color]  PC Packaging Overview
    [color=#107c10]▸[/color]  makepkg Reference
    [color=#107c10]▸[/color]  GameConfigEditor Reference
    [color=#107c10]▸[/color]  Achievements Guide
    [color=#107c10]▸[/color]  PlayFab Game Manager & Quickstart

[color=gray]Reopen this wizard anytime from [b]GDK → 🎓 Getting Started[/b][/color]"""
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
