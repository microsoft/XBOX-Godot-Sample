# GameInput addon (`godot_gameinput`)

`godot_gameinput` is a standalone GDExtension addon that brings the Microsoft
GameInput API to Godot 4.x on Windows. It works independently of the
`godot_gdk` addon — you can ship one, both, or neither.

The addon gives GDScript first-class access to:

* The GameInput runtime (initialize, shutdown, per-frame poll)
* Connected gamepads (display name, vendor / product id, vibration support)
* Per-frame readings (button bitmask + axes, with edge-detected press / release)
* Vibration (low + high freq motors, plus trigger rumble)
* Hot-plug signals on the main thread
* An inspector-friendly **action bridge** (`GameInputBinding` + `GameInputActionMap` +
  `GameInputMapper`) that drives Godot's `Input` / `InputMap` system from any
  GameInput device.

## Status

| Feature | Status |
| --- | --- |
| `GameInput` engine singleton | Shipped |
| Devices + readings + vibration | Shipped |
| `GameInputDevice.get_device_info()` | Shipped (issue #23, device-info half) |
| `GameInputBinding` / `GameInputActionMap` / `GameInputMapper` | Shipped |
| `EditorPlugin` autoload installer + Project Settings | Shipped |
| Sample integration (`sample/tutorial_app/` action-bridge scene + `sample/tutorial_gameinput/`) | Returning in PR 3 of the tutorial-driven sample revamp |
| Headless test suite | Shipped |
| Manual hardware checklist ([docs/gameinput/manual-tests.md](manual-tests.md)) | Shipped |
| Reading callbacks (event-driven) | Deferred — see issue list |
| Force feedback / arcade stick / racing wheel | Deferred |
| KB / mouse raw input | Deferred |

See `spec/gdext-gameinput.md` for the full spec and known deviations.

## Building

```powershell
# Build only the GameInput addon
cmake --preset gameinput-only
cmake --build --preset debug-gameinput

# Or build everything (both addons)
cmake --preset default
cmake --build build --preset debug
```

DLLs land in `addons/godot_gameinput/bin/` and are copied into every sample
project's `addons/godot_gameinput/` by the build's sample-sync step.

## Adding the addon to your Godot project

1. Copy `addons/godot_gameinput/` into your project (or symlink it).
2. Open the project once — Godot will detect `plugin.cfg`.
3. Project → Project Settings → Plugins → enable **Godot GameInput**.

Enabling the plugin installs an autoload called `GameInputBootstrap` that
reads two project settings and runs the lifecycle for you:

| Setting | Default | Behaviour |
| --- | --- | --- |
| `game_input/runtime/initialize_on_startup` | `false` | When `true`, the bootstrap calls `GameInput.initialize()` on `_ready`. |
| `game_input/runtime/auto_poll` | `true` | When `true`, the bootstrap calls `GameInput.poll()` from `_process`. |
| `game_input/mapper/default_action_map` | `""` | Path to a `.tres` `GameInputActionMap`. When set, the bootstrap spawns a `GameInputMapper` named `DefaultMapper` as its own child and assigns the loaded resource — so your project's `InputMap` can be driven from a GameInput action map without dropping a Mapper node into any scene. The same path is used as the fallback `action_map` for any user-placed `GameInputMapper` whose `action_map` property is null. |

Disabling the plugin removes the autoload — there is no orphaned state.

If you want full control instead, leave `initialize_on_startup` off and call
the lifecycle yourself; `GameInputMapper` nodes also call `poll()` defensively
(it is per-frame idempotent), so dropping a Mapper into a scene is enough even
without the autoload's `auto_poll`.

## Quick recipes

### Poll a device and rumble it on a button press

```gdscript
extends Node

func _ready() -> void:
    var gi = Engine.get_singleton("GameInput")
    if not gi.is_initialized():
        gi.initialize()
    gi.device_connected.connect(func(device): print("connected: ", device.get_display_name()))

func _process(_delta: float) -> void:
    var gi = Engine.get_singleton("GameInput")
    if not gi.is_initialized():
        return
    gi.poll()
    var pad := gi.get_primary_device()
    if pad == null:
        return
    var reading := gi.get_current_reading(pad)
    if reading != null and reading.was_button_pressed(GameInputDevice.BUTTON_A):
        gi.set_vibration(pad, 0.6, 0.3)
        await get_tree().create_timer(0.15).timeout
        gi.stop_haptics(pad)
```

### Drive Godot actions with a `GameInputMapper`

In the editor:

1. Right-click a folder → **Create New Resource** → `GameInputActionMap`.
2. Edit the map; add `GameInputBinding` rows. For each row:
   * `action` — a Godot action name that already exists in
     **Project Settings → Input Map** (e.g. `&"jump"`).
   * `source` — a `GameInputDevice.SRC_*` value
     (e.g. `SRC_BTN_A`, `SRC_AXIS_LEFT_X`).
   * `is_axis` — toggle on for thumbsticks / triggers.
   * `axis_threshold` — for axis-as-button, fire `action_press` when
     `|value| >= threshold`.
   * `axis_invert` — flip sign before evaluating. Handy for thumbstick Y in
     Godot's "down positive" convention.
   * `deadzone` — values within `[-deadzone, deadzone]` are clamped to 0.
3. Save the resource.
4. In your scene, add a `GameInputMapper` node and assign the action map to
   its `action_map` property — *or* set the project-wide
   `game_input/mapper/default_action_map` to your `.tres` and skip the node
   entirely (the bootstrap spawns a `DefaultMapper` for you).

The Mapper calls `Input.action_press(action, strength)` /
`Input.action_release(action)` each frame, so the rest of your code can stay
on Godot's standard `Input.is_action_pressed("jump")` / `Input.get_axis()`
APIs and gain GameInput device support transparently. On every
press / release transition the Mapper also pushes an `InputEventAction`
through `Input.parse_input_event` so event-driven consumers — UI focus
traversal for `ui_*`, `_gui_input` listeners, `_input` /
`_unhandled_input` handlers — actually see the action change. When Godot's
built-in joypad backend is already wired to deliver the same action through
a matching `InputEventJoypadButton` / `InputEventJoypadMotion` in your
`InputMap` (the default project mapping for `ui_accept` etc.), the Mapper
suppresses its own `InputEventAction` for that binding so menu actions fire
exactly once per physical press instead of twice.

### Soft-fail behaviour

Every public method on `GameInput`, `GameInputDevice`, and `GameInputReading`
returns a safe default and emits a single `push_warning` if called before
`initialize()`, after `shutdown()`, or on a host where GameInput is
unavailable. Your scene won't crash if the addon isn't ready yet — checks like
`if gi.is_initialized():` are optional, just preferred for clarity.

## Hot-plug / threading model

* Connect / disconnect events from GameInput arrive on a worker thread; the
  addon enqueues them under a mutex and emits Godot signals on the **main
  thread** during `poll()`.
* `GameInputDevice` wrappers hold a session-local monotonic id, **never** a
  raw `IGameInputDevice*`. Stale wrappers stay alive but `is_connected()`
  starts returning `false` and other methods return safe defaults.
* Device ids are never recycled within a session.

## In-editor docs

XML class documentation lives in `addons/godot_gameinput/doc_classes/` and is
wired into the addon's CMake `target_doc_sources`. Inside the editor, hover or
press F1 on any `GameInput*` symbol to see the full class reference.

## Sample integration

> **No sample projects currently.** PR 3 of the tutorial-driven
> sample revamp will add a GameInput scenario panel inside
> `sample/tutorial_app/` (Initialize / Shutdown / List Devices /
> Inspect Primary / Rumble Pulse / Stop Rumble, with live device
> count and hot-plug events in an event log), plus the standalone
> `sample/tutorial_gameinput/` project that builds the action
> bridge from scratch. Until then, follow
> [Tutorial — GameInput action bridge](../tutorials/gameinput-action-bridge.md)
> in your own project.

## Testing this addon

`godot_gameinput` is exercised by the `tests\godot\gameinput\` host. Coverage lives under `tests\godot\gameinput\tests\` and includes files such as `test_gameinput_core.gd`, `test_gameinput_device.gd`, `test_gameinput_reading.gd`, `test_gameinput_resource.gd`, `test_gameinput_mapper.gd`, `test_gameinput_mapper_extensions.gd`, and `test_gameinput_threading_smoke.gd`. Bootstrap autoload checks live under `tests\godot\gameinput\tests\bootstrap\`.

GameInput headless tests are deterministic by default and do not require live Xbox or PlayFab credentials. Hardware-specific behavior such as real controllers, rumble feel, and hot-plug should still be checked with [`gameinput/manual-tests.md`](manual-tests.md).

Run the standard pipeline from the repository root:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

See [`gdk/sample-and-tests.md`](../gdk/sample-and-tests.md) for the orchestrator stages, GUT layout, bootstrap mini-runners, baselines, and troubleshooting pointers.

## See also

* `spec/gdext-gameinput.md` — design spec, deferred work, deviations from the
  original sketch.
* `addons/godot_gameinput/CMakeLists.txt` — the build target. Add new sources
  to `_GAMEINPUT_SRCS` and new sync files to `godot_addon_sync_files_to_sample`'s
  FILES list.
* `.github/instructions/godot-gameinput.instructions.md` — path-scoped
  Copilot guidance for changes inside the addon and its samples.
