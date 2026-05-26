# GameInput GDExtension Spec

> **Status: v1 shipped.** Devices, polling, vibration, action bridge, project
> settings, EditorPlugin-installed bootstrap autoload, and sample integration
> in `gdk_launch_point` and `multiplayer_pong` are all live. Headless tests pass under
> `tests/godot/gameinput/tests/`. Device metadata is exposed via
> `GameInputDevice.get_device_info()`.
>
> Deviations from the original sketch are listed in
> [§ Deviations](#deviations-from-the-original-sketch). Deferred items are in
> [§ Deferred to v2](#deferred-to-v2).

## Overview

This document defines a **GDScript-first** plan for the `godot_gameinput` Godot GDExtension plugin.

`godot_gameinput` owns device discovery, polling, callbacks, haptics, and an optional Godot action bridge. It is the companion input document to `gdext-gdk.md`, but it should be able to ship independently on Windows builds that want GameInput without the rest of the GDK service layer.

The core architectural rule is: **C++ is internal; GDScript is the primary public surface**.

## Design goals

1. **GDScript-first API**: snake_case methods, signals, Godot types, no raw native handles.
2. **GDExtension-only**: no custom Godot fork required.
3. **Standalone plugin**: no hard dependency on `godot_gdk`.
4. **Godot-native ergonomics**: wrappers, signals, scene-tree-friendly mapper nodes, optional `Resource` configs.
5. **Graceful failure in editor/non-target runtime**: no crashes if GameInput is unavailable.

## Scope

| Domain | v1 | Notes |
| --- | --- | --- |
| Runtime init/shutdown | Shipped | `GameInputCreate`, callback registration, idempotent teardown |
| Device discovery/lifecycle | Shipped | connected/disconnected device cache, hot-plug signals on the main thread |
| Polling API | Shipped | `GameInput.poll()` is per-frame idempotent |
| Reading callbacks | Deferred | event-driven readings — see [§ Deferred to v2](#deferred-to-v2) |
| Vibration/rumble | Shipped | `SetRumbleState`-backed; `supportedRumbleMotors` checked first |
| Force feedback / advanced haptics | Deferred | optional follow-on after basic rumble |
| Battery state | Removed | GameInput v3 SDK dropped the battery API (`IGameInputDevice::GetBatteryState`, `GameInputBatteryState`) — no replacement upstream |
| Device info | Shipped | `GameInputDevice.get_device_info()` (issue #23, device-info half) |
| Godot action bridge | Shipped | `GameInputMapper` + `GameInputActionMap` + `GameInputBinding` |
| Project Settings + bootstrap autoload | Shipped | EditorPlugin installs `GameInputBootstrap` autoload |
| Dependency on `godot_gdk` | None | ships independently |

## Rationale and prior art

This spec borrows the Godot-facing integration patterns that already work well in platform plugins, especially singleton registration, project settings, callback dispatch, and optional Godot-native adapters. The main prior-art reference is [GodotSteam](https://godotsteam.com/) and its active source tree on [Codeberg](https://codeberg.org/godotsteam/godotsteam), which demonstrates the value of native singletons, project settings, callback dispatch, and optional Godot-facing wrapper types in a platform plugin.

### Why GDScript-first wrappers instead of raw native handles

Godot's scripting model is built around first-class [signals](https://docs.godotengine.org/en/stable/classes/class_signal.html), [`RefCounted`](https://docs.godotengine.org/en/stable/classes/class_refcounted.html), [`Node`](https://docs.godotengine.org/en/stable/classes/class_node.html), and [`Resource`](https://docs.godotengine.org/en/stable/classes/class_resource.html) objects. Exposing raw native handles such as `IGameInputDevice*` or `IGameInputReading*` directly to GDScript would fight both Godot ergonomics and Godot lifetime rules.

Wrapping native state in Godot objects such as `GameInputDevice` and `GameInputReading` keeps the public API aligned with normal GDScript usage patterns and hides COM-style lifetime management inside C++.

### Why polling and callbacks are both part of the public contract

[GameInput fundamentals](https://learn.microsoft.com/en-us/gaming/gdk/docs/features/common/input/overviews/input-fundamentals?view=gdk-2510) and [GameInput callbacks](https://learn.microsoft.com/en-us/gaming/gdk/docs/features/common/input/advanced/input-callbacks?view=gdk-2604) support both current-reading polling and event-driven updates. Godot gameplay code often wants both: deterministic per-frame sampling for movement/gameplay logic and connection or reading notifications for UX and device lifecycle.

The plugin should therefore expose both styles instead of forcing one. Polling covers the normal gameplay loop; callbacks keep device caches and connection state current.

### Why `GameInputMapper` exists

GameInput gives raw device state, but Godot gameplay code commonly goes through [`Input`](https://docs.godotengine.org/en/stable/classes/class_input.html) and [`InputMap`](https://docs.godotengine.org/en/stable/classes/class_inputmap.html). The optional `GameInputMapper` exists so GDScript-heavy projects can keep using `Input.is_action_pressed()` and project-defined actions while still benefiting from GameInput's device coverage and haptics.

## Public API conventions

### Global singleton

- `GameInput`

### Wrapper types exposed to GDScript

| Native concept | GDScript wrapper |
| --- | --- |
| `IGameInputDevice` | `GameInputDevice` |
| Input reading state | `GameInputReading` |

### General rules

1. Public methods use snake_case and Godot-native types.
2. GDScript-facing values stay within Godot's type system: `bool`, `int`, `float`, `String`, `Dictionary`, `Array`, and `PackedByteArray`.
3. Long-lived script objects use `RefCounted`, `Resource`, or `Node` when lifecycle matters.
4. Raw handles, pointers, and native query structs stay internal to C++.

## Plugin spec

### Root singleton

#### Root API

```gdscript
GameInput.initialize() -> bool
GameInput.shutdown() -> void
GameInput.is_initialized() -> bool
GameInput.poll() -> void
GameInput.get_devices(kind_mask := DEVICE_ALL) -> Array[GameInputDevice]
GameInput.get_primary_device(kind_mask := DEVICE_GAMEPAD) -> GameInputDevice
GameInput.get_current_reading(device: GameInputDevice) -> GameInputReading
GameInput.set_vibration(device: GameInputDevice, low_freq: float, high_freq: float, left_trigger := 0.0, right_trigger := 0.0) -> bool
GameInput.stop_haptics(device: GameInputDevice) -> void
GameInput.enable_device_callbacks(enabled := true) -> void
```

#### Signals

```gdscript
device_connected(device: GameInputDevice)
device_disconnected(device_id: int)
reading_available(device_id: int)
```

#### `GameInputDevice`

```gdscript
get_device_id() -> int
get_display_name() -> String
get_kind_mask() -> int
supports_vibration() -> bool
supports_haptics() -> bool
```

#### `GameInputReading`

```gdscript
is_button_down(button: int) -> bool
was_button_pressed(button: int) -> bool
get_axis(axis: int) -> float
get_timestamp() -> int
```

#### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `GameInput.initialize()` | `GameInputCreate`, `IGameInput::RegisterDeviceCallback`, `IGameInput::RegisterReadingCallback` | Creates the root GameInput interface and optionally primes device and reading callbacks. |
| `GameInput.shutdown()` | `IGameInput::UnregisterCallback`, `IGameInput::Release` | Unregister callbacks first, then release the root interface and any cached COM-style objects. |
| `GameInput.poll()` | `IGameInput::GetCurrentReading` | Refreshes cached readings for tracked devices in polling mode. |
| `GameInput.get_devices()` / `GameInput.get_primary_device()` | `IGameInput::RegisterDeviceCallback` | Build a device cache from the initial enumeration delivered by callback registration and keep it current with subsequent device-status callbacks. |
| `GameInput.get_current_reading()` | `IGameInput::GetCurrentReading` | Returns a wrapped `IGameInputReading` snapshot for the requested device. |
| `GameInput.set_vibration()` | `IGameInputDevice::SetRumbleState` | v1 haptics path is controller rumble, including trigger rumble when supported. |
| `GameInput.stop_haptics()` | `IGameInputDevice::SetRumbleState` | Send a zeroed rumble state. Advanced force-feedback work can later layer on the force-feedback APIs. |
| `GameInput.enable_device_callbacks()` | `IGameInput::RegisterDeviceCallback`, `IGameInput::RegisterReadingCallback`, `IGameInput::UnregisterCallback` | Toggles the event-driven device and reading feed. |
| `GameInputDevice` getters | `IGameInputDevice::GetDeviceInfo` | Cache display name, kind mask, and vibration/haptics capability flags from `GameInputDeviceInfo`. |
| `GameInputReading` getters | `IGameInputReading::GetGamepadState`, `IGameInputReading::GetKeyState`, `IGameInputReading::GetMouseState`, `IGameInputReading::GetTimestamp` | Normalize native readings into one Godot-facing wrapper without exposing raw GameInput structs. |

### Action bridge

`GameInput` should expose raw input, but it also needs a Godot-native bridge.

#### Additional types

- `GameInputActionMap` (`Resource`)
- `GameInputMapper` (`Node`)

#### `GameInputMapper`

`GameInputMapper` is a `Node` intended to live in the scene tree. It polls `GameInput` or consumes device updates each frame, then emits synthetic `InputEventAction` events against a configured `GameInputActionMap`.

Use it when gameplay code already depends on `Input`, `InputMap`, and project-defined actions. Skip it when a system needs raw per-device state, custom deadzones, or device-specific UX.

- polls `GameInput` each frame
- translates readings into `InputEventAction`
- lets game code keep using:

```gdscript
Input.is_action_pressed("jump")
```

#### Rationale

Raw API is still the right fit for low-level systems. The mapper exists so GDScript-heavy projects can keep using Godot's normal action flow without having to re-author gameplay code around device-level readings.

## Plugin settings

### Runtime

| Setting | Default | Purpose |
| --- | --- | --- |
| `game_input/runtime/initialize_on_startup` | `false` | Bootstrap autoload calls `GameInput.initialize()` after the SceneTree is ready. |
| `game_input/runtime/auto_poll` | `true` | Bootstrap autoload calls `GameInput.poll()` from `_process` so apps don't have to. `GameInputMapper` nodes also call `poll()` defensively (idempotent). |

### Mapper

| Setting | Default | Purpose |
| --- | --- | --- |
| `game_input/mapper/default_action_map` | `""` | When set to a `GameInputActionMap` resource path (e.g. `res://input/actions.tres`), the bootstrap autoload spawns a `GameInputMapper` named `DefaultMapper` as its child and assigns the loaded resource to it. Lets a project drive its `InputMap` from a GameInput action map without adding any nodes to its scenes. Also serves as the fallback `action_map` for any user-placed `GameInputMapper` whose `action_map` property is null. |

> **Note:** `game_input/runtime/embed_dispatch` was dropped — GameInput
> dispatches callbacks on its own worker thread and we don't manually drive
> `IGameInputDispatcher`. `game_input/runtime/enable_device_callbacks` was
> also dropped: device callbacks are always-on after `initialize()` because
> the device cache and `get_devices()` depend on them.

## Build and packaging rules

1. **Plugin ships as its own `.gdextension`**
   - `godot_gameinput.gdextension`

2. **Can share internal support code with companion plugins**
   - string conversion
   - logging
   - optional common helper code if `godot_gdk` ships beside it

3. **Soft-fail outside supported runtimes**
   - editor should still load docs/classes
   - runtime-only methods return unavailable errors instead of crashing

4. **No hard dependency on `godot_gdk`**
   - ship together if needed
   - use separately if desired

## Rollout

| Step | Deliverable | Status |
| --- | --- | --- |
| 1 | `GameInput` raw polling + device callbacks + vibration | Shipped |
| 2 | `GameInputMapper` + action map resource | Shipped |
| 3 | Device info (issue #23) | Shipped (battery half removed — GameInput v3 dropped the API) |
| 4 | Sample integration (`gdk_launch_point` panel + `multiplayer_pong` rumble & hot-plug) | Shipped |
| 5 | Headless test suite + manual hardware checklist | Shipped |
| 6 | F1 doc XML + user docs + path-scoped instructions | Shipped |

## Deviations from the original sketch

These are intentional design changes from the early draft of this spec, made
during implementation and validated through a rubber-duck design pass:

- **`GameInputMapper` is action-level, not physical-event-level.**
  The Mapper emits via `Input.action_press(action, strength)` /
  `Input.action_release(action)` keyed off `GameInputBinding` rows so polled
  consumers (paddle handlers reading
  `Input.is_action_pressed("move_up")`) keep working, and on every press /
  release transition it additionally pushes an `InputEventAction` through
  `Input.parse_input_event` so event-driven consumers — Viewport GUI focus
  traversal for `ui_*`, `_gui_input`, `_input` / `_unhandled_input` — see the
  action transition. It does **not** inject `InputEventJoypadButton` /
  `InputEventJoypadMotion` — that would create a split-brain where the same
  input flows through two paths.
  When Godot's built-in joypad backend is already wired to deliver the same
  action via a matching `InputEventJoypadButton` / `InputEventJoypadMotion` in
  the project's `InputMap` (e.g., the default `ui_accept` mapping that
  includes joypad button A), the Mapper detects the duplicate per binding,
  caches the result, and skips its own `InputEventAction` so menu actions
  fire exactly once per physical press instead of twice.
  Action names must already exist in `InputMap`; missing actions trigger a
  single per-instance `push_warning` (debounced via
  `HashSet<StringName> m_warned_missing_actions`).
- **Bootstrap autoload installed by `EditorPlugin`.**
  Project settings alone never bootstrap runtime logic. The
  `GameInputBootstrap` autoload is the single bootstrap surface;
  `editor/gameinput_editor_plugin.gd::_enable_plugin()` calls
  `add_autoload_singleton(...)`, and `_disable_plugin()` removes it. There is
  no orphaned state when the plugin is disabled.
- **Threading model is explicit.**
  GameInput device callbacks fire on a worker thread. They only push events
  into a mutex-protected queue and `AddRef` the device pointer. The main
  thread drains the queue inside `poll()` and emits Godot signals from there
  — no `call_deferred` dance.
- **`poll()` is per-frame idempotent.**
  Backed by `Engine::get_singleton()->get_process_frames()`. Multiple
  Mappers + the bootstrap autoload all call `poll()` defensively, but the
  real refresh runs only once per frame and `prev` button state stays correct
  for `was_button_pressed()`.
- **Device IDs are session-local monotonic, never recycled.**
  `GameInputDevice` wrappers hold only the id (a weak handle), never a raw
  `IGameInputDevice*`. After disconnect the id is retired, the wrapper stays
  alive as an inert RefCounted, and methods return safe defaults.
- **`GameInputBinding` is a Resource per binding** (not `Array[Dictionary]`).
  Inspector-friendly typed exports for `action`, `source`, `is_axis`,
  `axis_threshold`, `axis_invert`, `deadzone`. `GameInputActionMap.bindings`
  is a `TypedArray<GameInputBinding>` with `PROPERTY_HINT_ARRAY_TYPE` so the
  editor renders an inspector for resource children.
- **Single combined `Source` enum** for `GameInputBinding.source` covering
  buttons (0–13) and axes (100–105).
- **Soft-fail only — no compile-time platform guards.**
  This repo is Windows-only by mission. Every public method checks
  `_ensure_initialized()` and returns safe defaults (`false`, `null`, empty
  `Array`, `-1.0`) when conditions fail.
- **`embed_dispatch` and `enable_device_callbacks` settings dropped.**
  Documented above.
- **Doc XML race fix:** `addons/godot_gameinput/CMakeLists.txt` serializes
  `target_doc_sources` execution against the `godot_gdk` doc target via
  `add_dependencies`, sidestepping the MSB8065 "two writers race for the
  same gen/ output dir" failure.

## Deferred to v2

These were on the wishlist for v1 but pushed out so the cohesive raw API +
action bridge could ship quickly. Each will get its own GitHub issue:

- **Reading callbacks** — event-driven readings via
  `IGameInput::RegisterReadingCallback` (so games can react in the same
  millisecond that input arrives, instead of waiting for the next frame).
- **Force feedback / advanced haptics** — beyond the basic
  `SetRumbleState` rumble that ships in v1.
- **Arcade stick / fight stick** support.
- **Racing wheel** support.
- **Keyboard / mouse raw input** through GameInput (the addon is gamepad-only
  in v1; KB/M still flows through Godot's standard event pipeline).
- **XInput fallback** for hosts where GameInput isn't available.
- **Linux/macOS native paths** — out of scope for the GDK-flavoured Windows
  mission of this repo.

