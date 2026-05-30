---
description: Godot GameInput addon architecture, threading model, action-bridge conventions, and sample workflow
applyTo: "addons/godot_gameinput/**, tests/godot/gameinput/**, sample/tutorial_gameinput/**, docs/gameinput/**, spec/gdext-gameinput.md"
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

- `sample/tutorial_gameinput/` ships as the standalone GameInput action-bridge
  sample. It uses the `GameInputBootstrap` autoload, displays connected-device
  count and hot-plug events, and is the canonical manual host for mapper,
  device discovery, and hot-plug checks.
- A GameInput scenario panel inside `sample/tutorial_app/` is not present yet.
  Until that lands, raw rumble verification follows the
  [GameInput manual-test checklist](../../docs/gameinput/manual-tests.md) using
  a small local scene or other GameInput-enabled project.
- The headless test entry point for GameInput is the repo-root orchestrator:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

  GameInput suites run in the `gut:tests/godot/gameinput` stage; bootstrap-autoload coverage runs in the `bootstrap:tests/godot/gameinput:*` stages. To iterate on the GameInput host alone:

```powershell
cd tests\godot\gameinput
..\..\..\sample\Godot_v4.6.1-stable_win64_console.exe --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests -ginclude_subdirs -gexit
```

  GameInput suites live in `tests/godot/gameinput/tests/` and `extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"` (mirrored from `addons\godot_gdk\tests_support\bases\gameinput_test_base.gd`). Add new suites as `test_*.gd` files in that directory — GUT discovers them automatically via `-gdir` + `-ginclude_subdirs`; there is no central registration script. Bootstrap-autoload scenarios go under `tests\godot\gameinput\tests\bootstrap\` as one-shot scripts the orchestrator launches in fresh Godot processes.

## GDScript Conventions

- snake_case for methods and properties; `&"action_name"` for action `StringName`s.
- Avoid `:=` when the right-hand side comes from a Variant-returning engine
  API (e.g. `gi.initialize()`) — the parser cannot infer the type. Use
  `var x: bool = gi.initialize()` instead.
- For float comparisons in tests, use `assert_eq_approx` (a GUT built-in
  assertion) — C++ float properties round-trip through 32-bit storage and
  won't equal 64-bit double literals exactly.

## Documentation & Specs

- F1 in-editor docs live in `addons/godot_gameinput/doc_classes/`. Wire new
  classes through `target_doc_sources` in the CMakeLists.
- `docs/gameinput/plugin.md` is the user-facing reference. Update it when
  public API or sample workflow changes.
- `spec/gdext-gameinput.md` is the source of truth for design decisions and
  deferred work. Mark sections shipped or note deviations there when
  scope changes.
