# GameInput action bridge

> **Standalone track.** This tutorial is independent of the main
> cumulative chain (Sign-in → Achievements → Leaderboards → Game
> Saves → Lobby → MPA → Party → Capstone). You can read it before,
> after, or in parallel with that chain — it does not depend on
> a signed-in XBOX user or any PlayFab state.

## What you'll build

A `GameInputActionMap` resource that bridges a Microsoft GameInput
gamepad into Godot's standard `InputMap`. By the end:

- The `godot_gameinput` runtime polls the first connected gamepad
  every frame, with no manual `_process` code in your scenes.
- A `GameInputActionMap` resource maps face buttons and thumbsticks
  to your project's actions (`ui_accept`, `move_left`, `jump`, …).
- Anywhere in your gameplay code you keep using
  `Input.is_action_pressed("jump")` — GameInput appears under the
  hood without your character controller knowing.
- You react to hot-plug via the
  `GameInput.device_connected` / `device_disconnected` signals.

When it works, a Sprite that uses `move_left` / `move_right` /
`jump` reacts to gamepad input even when the player's XBOX
controller would otherwise be routed through GameInput-only paths
(some Windows builds, all Microsoft GDK title-process builds).

## Prerequisites

- The `godot_gameinput` addon is enabled in
  **Project Settings → Plugins** (the addon installs a
  `GameInputBootstrap` autoload).
- A scratch Godot project with a single scene to attach the
  mapper to. You do **not** need to have followed any tutorial
  in the main cumulative chain — GameInput is entirely
  independent of XBOX / PlayFab sign-in.
- `game_input/runtime/initialize_on_startup` is set to `true`.
  This defaults to `false`, so flip it explicitly in
  **Project Settings → Game Input → Runtime** (or call
  `GameInput.initialize()` yourself before the first mapper poll).
  The bootstrap autoload reads this on `_ready`.
- `game_input/runtime/auto_poll` is `true` (the default; the
  bootstrap pumps `GameInput.poll()` every process frame). Without
  auto-poll, `GameInputMapper` calls poll defensively per frame
  anyway — auto-poll is mostly a perf optimization when multiple
  consumers share state.
- A wired or wireless XBOX controller (or other GameInput-compatible
  gamepad) plugged into the dev PC. The action bridge is
  GDScript-only on top of `GameInput` — it works on any
  GameInput-supported device kind.
- One-page primer on async patterns:
  [Async patterns](../async-patterns.md) — GameInput is mostly
  synchronous, but the surrounding addon conventions still apply.

## Relevant addon surfaces

- [`GameInput`](../../addons/godot_gameinput/doc_classes/GameInput.xml)
  — `is_initialized`, `poll`, `get_devices`,
  `get_connected_device_count`, signals `device_connected` /
  `device_disconnected`.
- [`GameInputActionMap`](../../addons/godot_gameinput/doc_classes/GameInputActionMap.xml)
  — the typed `Resource` you author in the inspector (or in code).
- [`GameInputBinding`](../../addons/godot_gameinput/doc_classes/GameInputBinding.xml)
  — one row per action / source pair.
- [`GameInputMapper`](../../addons/godot_gameinput/doc_classes/GameInputMapper.xml)
  — the `Node` that polls and feeds `Input.action_press` /
  `Input.action_release` every frame.
- [`GameInputDevice`](../../addons/godot_gameinput/doc_classes/GameInputDevice.xml)
  — wrapper exposing `get_device_id`, `get_display_name`, and the
  `SRC_*` constants.

> **GameInput vs. Godot's built-in joypad backend.** Godot has its
> own joypad backend that delivers `InputEventJoypadButton` and
> `InputEventJoypadMotion`. GameInput is the Microsoft GDK runtime that
> exposes the same hardware through the XBOX-supported input stack.
> Use this tutorial when your project ships to Microsoft GDK targets (so the
> built-in backend may not see the controller) or when you want
> richer feedback like impulse triggers and per-motor rumble.

## Step 1 — Declare your actions

Open **Project Settings → Input Map** and add the actions you want
to drive from the gamepad. For this tutorial:

| Action | Default event |
|---|---|
| `ui_accept` | Keyboard Enter (already defined by Godot) |
| `move_left` | Keyboard A |
| `move_right` | Keyboard D |
| `jump` | Keyboard Space |

You do not need to add gamepad events here. The mapper hands the
press / release through `Input.action_press` directly. (You can
keep keyboard events; the bridge is additive.)

> **One gotcha:** if you also bind `ui_accept` to "Joypad Button A"
> in the Input Map, the built-in joypad backend will fire it too —
> the mapper detects this and skips its own synthetic event for
> that one binding so you don't double-fire. The polled state
> (`Input.is_action_pressed("ui_accept")`) is still refreshed each
> frame either way.

## Step 2 — Create a `GameInputActionMap` resource

1. In the FileSystem dock, right-click your `res://` root and pick
   **New Resource…**.
2. Search for **GameInputActionMap** and create it. Save as
   `res://input/gamepad.tres`.
3. With the resource selected, switch to the Inspector. The
   `bindings` field is a typed array of `GameInputBinding`.
4. Click the `+` to add a binding for each row in this table:

| `action`      | `source`                   | `is_axis` | `axis_invert` | Notes |
|---------------|----------------------------|-----------|---------------|-------|
| `ui_accept`   | `SRC_BTN_A`                | false     | false         | A button. |
| `jump`        | `SRC_BTN_A`                | false     | false         | Same source as `ui_accept`. |
| `move_left`   | `SRC_AXIS_LEFT_X`          | true      | true          | Inverted so "stick left" reads as positive. |
| `move_right`  | `SRC_AXIS_LEFT_X`          | true      | false         | Right is the native positive direction. |
| `dpad_up`     | `SRC_BTN_DPAD_UP`          | false     | false         | If you've added it. |

For axis rows, leave `deadzone` at `0.2` and `axis_threshold` at
`0.5` to start — those defaults match Godot's deadzone convention.

The `bindings` field is a real typed array, so you can also build
the map in code:

```gdscript
func _build_default_map() -> GameInputActionMap:
    var map := GameInputActionMap.new()

    var accept := GameInputBinding.new()
    accept.action = &"ui_accept"
    accept.source = GameInputDevice.SRC_BTN_A
    map.add_binding(accept)

    var jump := GameInputBinding.new()
    jump.action = &"jump"
    jump.source = GameInputDevice.SRC_BTN_A
    map.add_binding(jump)

    var left := GameInputBinding.new()
    left.action = &"move_left"
    left.source = GameInputDevice.SRC_AXIS_LEFT_X
    left.is_axis = true
    left.axis_invert = true
    map.add_binding(left)

    var right := GameInputBinding.new()
    right.action = &"move_right"
    right.source = GameInputDevice.SRC_AXIS_LEFT_X
    right.is_axis = true
    map.add_binding(right)

    return map
```

Either path produces the same resource; the inspector path is
better for designers, the code path is better for tests and for
runtime remapping screens.

## Step 3 — Add a `GameInputMapper` node

The mapper is a `Node`. The pattern is to add it once high up in
your scene tree (an autoload, or the root of your main scene) and
let every gameplay scene rely on standard `Input` calls below it.

The simplest approach: a one-line autoload.

```gdscript
# res://input/gamepad_autoload.gd
extends Node

func _ready() -> void:
    var mapper := GameInputMapper.new()
    mapper.name = "GamepadMapper"
    mapper.action_map = preload("res://input/gamepad.tres")
    add_child(mapper)
```

Register it in **Project Settings → Globals → Autoload**:

| Path | Node Name |
|---|---|
| `res://input/gamepad_autoload.gd` | `Gamepad` |

That's it for the wiring. Every existing
`Input.is_action_pressed("jump")` call in your project now sees the
gamepad.

## Step 4 — Use the actions from gameplay

No change to gameplay code. The reason this tutorial is short is
exactly because gameplay code stays oblivious:

```gdscript
extends CharacterBody2D

const SPEED := 200.0
const JUMP_VELOCITY := -400.0

func _physics_process(delta: float) -> void:
    var direction := Input.get_action_strength("move_right") - Input.get_action_strength("move_left")
    velocity.x = direction * SPEED

    if Input.is_action_just_pressed("jump") and is_on_floor():
        velocity.y = JUMP_VELOCITY

    move_and_slide()
```

`Input.get_action_strength("move_right")` returns the post-deadzone
strength fed in from the mapper, which lets analog stick movement
work without any extra logic.

## Step 5 — React to hot-plug

For a single-player game, the mapper's default
`target_kind_mask = KIND_GAMEPAD` and `target_device_id = -1` is
enough: it always uses the **primary** connected gamepad in
insertion order, so a controller plugged in mid-game is picked up
automatically.

For split-screen or per-player binding you need to pin each mapper
to a specific device id when its owner connects. Listen to
`GameInput.device_connected`:

```gdscript
extends Node

@export var _mappers: Array[GameInputMapper] = []

func _ready() -> void:
    GameInput.device_connected.connect(_on_device_connected)
    GameInput.device_disconnected.connect(_on_device_disconnected)

    # Bind any devices that were already connected before _ready.
    for device in GameInput.get_devices(GameInput.DEVICE_GAMEPAD):
        _bind_device(device)

func _on_device_connected(device: GameInputDevice) -> void:
    print("[Pad] connected: id=%d (%s)" % [device.get_device_id(), device.get_display_name()])
    _bind_device(device)

func _on_device_disconnected(device_id: int) -> void:
    print("[Pad] disconnected: id=%d" % device_id)
    for mapper in _mappers:
        if mapper.target_device_id == device_id:
            mapper.target_device_id = -1  # fall back to primary

func _bind_device(device: GameInputDevice) -> void:
    for mapper in _mappers:
        if mapper.target_device_id == -1:
            mapper.target_device_id = device.get_device_id()
            return
```

The two events are queued from the GameInput worker thread and
fire on the **main thread** during the next `GameInput.poll()` —
the device wrapper is safe to use from your handler.

Device ids are session-local and never recycled, so storing the id
of "player 1's pad" in a dictionary is safe for the entire
process lifetime.

## Step 6 — (Optional) Sanity-check the runtime

When a build runs without a GameInput-capable host (for example a
non-Microsoft GDK Windows build that did not satisfy
`GameInputCreate()`), every GameInput call soft-fails into a safe
default and `GameInput.is_initialized()` returns `false`. Your
gameplay code keeps working off keyboard input; the mapper just
does nothing.

For a one-shot diagnostic in development:

```gdscript
func _ready() -> void:
    if not GameInput.is_initialized():
        push_warning("[Pad] GameInput runtime not available — gamepad input disabled.")
        return
    print("[Pad] %d gamepad(s) currently connected" % GameInput.get_connected_device_count())
```

## Verify

With one gamepad connected, running the scene prints:

```
[Pad] 1 gamepad(s) currently connected
```

Pressing the A button fires `ui_accept` (focused UI elements
react) **and** `jump` (the character jumps). Tilting the left stick
fires `move_left` / `move_right` with smooth analog strength
visible in `Input.get_action_strength`.

Common failures:

| Output | Diagnosis | Fix |
|---|---|---|
| Single `push_warning` per missing action like `[GameInputMapper] Action not found in InputMap: 'jump'` | The action name on a `GameInputBinding` does not exist in **Project Settings → Input Map**. | Add the action, or fix the typo on the binding. |
| Action fires twice (e.g., your jump triggers a double-jump) | A native joypad event for the same button is bound in the Input Map AND the mapper is also firing. | Either remove the native joypad event for that action, or accept the mapper's automatic skip (it should detect this case — if it doesn't, file a bug). |
| Analog stick "snaps" to full strength at a small tilt | `deadzone` on the axis binding is too low. | Raise `deadzone` to `0.2` or higher (the default). |
| `[Pad] GameInput runtime not available` on a Microsoft GDK build | `GameInputCreate()` failed at runtime. | Check that the Microsoft GDK is installed on the target machine and the addon `bin/` ships with the build. |

## Reference implementation

The end-state lives in the standalone GameInput sample at
[`sample/tutorial_gameinput/`](../../sample/tutorial_gameinput/README.md):

- Scene: [`sample/tutorial_gameinput/main.tscn`](../../sample/tutorial_gameinput/main.tscn)
- Script: [`sample/tutorial_gameinput/main.gd`](../../sample/tutorial_gameinput/main.gd)

> **Layout note.** The tutorial splits the work across an autoload
> (`res://input/gamepad_autoload.gd`), a resource
> (`res://input/gamepad.tres`), and a gameplay scene that consumes
> the autoload. The sample collapses this into a single `main.tscn`
> + `main.gd` that builds the action map in code (the Step 2 code
> path) and adds the `GameInputMapper` at scene `_ready` instead of
> via autoload. Same wiring, fewer files — and it adds visible
> debug UI (device list, hot-plug log, live action strengths,
> jumping player) so you can see the bridge react.

## What's next

- **Add rumble.** `GameInput.set_vibration(device, low, high, lt, rt)`
  takes the same `GameInputDevice` wrappers the bridge uses. The
  [GameInput addon doc](../gameinput/plugin.md) covers the rumble
  patterns the addon supports.
- **Per-player binding for split-screen.** The hot-plug handler in
  Step 5 is the starting point — extend it to pin specific device
  ids to per-player `GameInputMapper` nodes.
- **Wire GameInput into a signed-in XBOX session.** If you also
  built through the main cumulative chain (signs in, lobbies,
  Party, MPA), the [capstone integration tech demo](integrated/02-tech-demo.md)
  is the natural place to drop the gamepad autoload alongside the
  identity / lobby / Party panels.

- Reference: [`GameInput`](../../addons/godot_gameinput/doc_classes/GameInput.xml),
  [`GameInputActionMap`](../../addons/godot_gameinput/doc_classes/GameInputActionMap.xml),
  [`GameInputBinding`](../../addons/godot_gameinput/doc_classes/GameInputBinding.xml),
  [`GameInputMapper`](../../addons/godot_gameinput/doc_classes/GameInputMapper.xml),
  [`GameInputDevice`](../../addons/godot_gameinput/doc_classes/GameInputDevice.xml)

