# GameInput addon (`godot_gameinput`)

`godot_gameinput` is a standalone GDExtension addon that brings the Microsoft
GameInput API to Godot 4.x on Windows. It works independently of the
`godot_gdk` addon — you can ship one, both, or neither.

The addon gives GDScript first-class access to:

* The GameInput runtime (initialize, shutdown, per-frame poll)
* Connected gamepads (display name, vendor / product id, battery, vibration support)
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
| `GameInputDevice.get_battery_level()` / `get_device_info()` | Shipped (issue #23) |
| `GameInputBinding` / `GameInputActionMap` / `GameInputMapper` | Shipped |
| `EditorPlugin` autoload installer + Project Settings | Shipped |
| Sample integration (`shamwow`, `multiplayer_pong`) | Shipped |
| Headless test suite | Shipped |
| Manual hardware checklist ([docs/godot-gameinput-manual-tests.md](godot-gameinput-manual-tests.md)) | Shipped |
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
APIs and gain GameInput device support transparently.

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

* **`sample/shamwow`** — full GameInput scenario panel with Initialize /
  Shutdown / List Devices / Inspect Primary / Rumble Pulse / Stop Rumble.
  Live device count + battery surface in the state panel; hot-plug events
  appear in the event log.
* **`sample/multiplayer_pong`** — paddle-hit and score events vibrate the
  primary controller; the lobby surfaces controller hot-plug / disconnect as
  status messages.

## See also

* `spec/gdext-gameinput.md` — design spec, deferred work, deviations from the
  original sketch.
* `addons/godot_gameinput/CMakeLists.txt` — the build target. Add new sources
  to `_GAMEINPUT_SRCS` and new sync files to `godot_addon_sync_files_to_sample`'s
  FILES list.
* `.github/instructions/godot-gameinput.instructions.md` — path-scoped
  Copilot guidance for changes inside the addon and its samples.
