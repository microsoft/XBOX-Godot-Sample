---
description: Godot GameInput addon architecture, threading model, action-bridge conventions, and sample workflow
applyTo: "addons/godot_gameinput/**, sample/shamwow/addons/godot_gameinput/**, sample/multiplayer_pong/addons/godot_gameinput/**, sample/gdk_demo/addons/godot_gameinput/**, sample/shamwow/tests/**, sample/multiplayer_pong/logic/lobby.gd, sample/multiplayer_pong/logic/pong.gd, sample/multiplayer_pong/logic/paddle.gd, docs/godot-gameinput.md, spec/gdext-gameinput.md"
---

# Godot GameInput Addon Instructions

## Public Architecture

- `GameInput` is the only engine singleton registered by this addon. Access
  via `Engine.get_singleton("GameInput")` (or just `GameInput` once cached
  from the singleton).
- Wrapper classes — `GameInputDevice`, `GameInputReading`, `GameInputBinding`,
  `GameInputActionMap`, `GameInputMapper` — are part of the public Godot-facing
  contract. Treat their inspector-visible properties and signals as stable
  surfaces.
- The addon is standalone: there is no build-time or runtime dependency on
  `godot_gdk`.

## Threading & Lifecycle

- `IGameInput` device callbacks fire on a GameInput-owned worker thread. That
  thread MUST NOT mutate the device cache, emit Godot signals, or call into
  godot-cpp APIs. Worker callbacks may only push events into the
  mutex-protected pending queue and `AddRef` the native device pointer.
- The main thread drains the pending queue inside `GameInput::poll()`. Signals
  are emitted from there — no `call_deferred` plumbing needed.
- `GameInput::poll()` is per-frame idempotent. The check uses
  `Engine::get_singleton()->get_process_frames()`. Real refresh runs at most
  once per frame regardless of how many `GameInputMapper` nodes (or the
  bootstrap autoload) call `poll()` defensively.
- `prev` button state for `GameInputReading::was_button_pressed()` /
  `was_button_released()` is updated only on the real refresh. Multiple
  `poll()` calls in the same frame won't drop edges.

## Soft-Fail Conventions

- Every public method on `GameInput`, `GameInputDevice`, `GameInputReading`,
  and `GameInputMapper` must return a safe default (`false`, `0`, `-1.0`,
  empty `Array`, `null`, etc.) and emit at most one `push_warning` when the
  runtime is uninitialized, the addon is not loaded, or a referenced device
  has been disconnected. Crashes from calling into a stale or absent runtime
  are bugs.
- `_ensure_initialized()` is the standard helper in `gameinput_singleton.cpp`.
  Use it in every new public method.
- `GameInputMapper` debounces missing-action warnings via
  `m_warned_missing_actions`; do not spam `push_warning` per-frame.

## Device IDs

- `int64_t` device ids are session-local monotonic and **never recycled**.
  After disconnect the id is retired permanently. New devices get fresh ids.
- `GameInputDevice` wrappers hold only the id (a weak handle), never a raw
  `IGameInputDevice*`. This is what makes stale wrappers safe — they become
  inert once the underlying device is gone.

## Action Bridge Rules

- `GameInputMapper` emits **actions only** (`Input.action_press` /
  `Input.action_release`). Do not also synthesize `InputEventJoypadButton` /
  `InputEventJoypadMotion` — that creates a split-brain.
- Actions referenced by a `GameInputBinding` must already exist in Godot's
  `InputMap`. Mapper warns once per missing action and skips it.
- `GameInputBinding` is a `Resource` with typed exports. New binding fields
  must keep the existing inspector-friendly pattern (typed `@export`, hint
  ranges for floats, dropdowns for enums).
- `GameInputActionMap.bindings` is a `TypedArray<GameInputBinding>`; preserve
  the `PROPERTY_HINT_ARRAY_TYPE` registration so the editor renders an
  inspector for resource children.

## C++ and Registration Conventions

- Every header that includes Windows / GameInput APIs must define
  `WIN32_LEAN_AND_MEAN` and include `<windows.h>` before Godot or GameInput
  headers.
- **Never name a local header the same as an SDK header (case-insensitively).**
  Windows file systems are case-insensitive; `gameinput.h` will silently
  shadow `<GameInput.h>` and break the build with hundreds of cryptic
  errors. The singleton header is named `gameinput_singleton.h` for that
  reason. Keep new file names disambiguated.
- Register new native classes in
  `addons\godot_gameinput\src\register_types.cpp`. Add new
  implementation files to the `_GAMEINPUT_SRCS` list in
  `addons\godot_gameinput\CMakeLists.txt`.
- When exposing object-returning properties from C++, set the `PropertyInfo`
  class name (e.g. `GameInputActionMap`) so Godot does not instantiate
  anonymous object defaults.
- Every project setting introduced by the addon goes through
  `_register_setting()` in `register_types.cpp`. Always gate writes with
  `has_setting()` so editor reloads stay clean, then call
  `set_initial_value()` and `add_property_info()`.

## EditorPlugin / Bootstrap

- The single bootstrap surface is the `GameInputBootstrap` autoload installed
  by `editor/gameinput_editor_plugin.gd::_enable_plugin()`. Project settings
  alone never bootstrap runtime logic.
- The autoload only calls `shutdown()` if it owned the `initialize()` (the
  `_initialized_here` flag). This keeps editor reloads and tests that drive
  the runtime themselves working correctly.

## Sample Integration

- The GameInput addon is enabled in `sample/shamwow` (full scenario panel)
  and `sample/multiplayer_pong` (rumble on hit, controller hot-plug surface
  in the lobby). Update both samples when public `godot_gameinput` behaviour
  changes.
- Pong's `pulse_rumble()` helper is the canonical "raw API" usage pattern;
  shamwow's GameInput group is the canonical "explore the API surface"
  pattern.
- The headless test entry point lives in shamwow:

```powershell
cd sample/shamwow
.\Godot_v4.6.1-stable_win64_console.exe --headless --script res://tests/run_tests.gd
```

  Tests live in `sample/shamwow/tests/`. Add new suites under
  `sample/shamwow/tests/suites/` and register them in `tests/run_tests.gd`.

## GDScript Conventions

- snake_case for methods and properties; `&"action_name"` for action `StringName`s.
- Avoid `:=` when the right-hand side comes from a Variant-returning engine
  API (e.g. `gi.initialize()`) — the parser cannot infer the type. Use
  `var x: bool = gi.initialize()` instead.
- For float comparisons in tests, use `assert_eq_approx` (defined in
  `sample/shamwow/tests/test_context.gd`) — C++ float properties round-trip
  through 32-bit storage and won't equal 64-bit double literals exactly.

## Documentation & Specs

- F1 in-editor docs live in `addons/godot_gameinput/doc_classes/`. Wire new
  classes through `target_doc_sources` in the CMakeLists.
- `docs/godot-gameinput.md` is the user-facing reference. Update it when
  public API or sample workflow changes.
- `spec/gdext-gameinput.md` is the source of truth for design decisions and
  deferred work. Mark sections shipped or note deviations there when
  scope changes.
