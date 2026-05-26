# GameInput Manual Hardware Test Checklist

The headless test suite under `tests/godot/gameinput/tests/` covers everything that
can be verified without a real controller. This document covers the pieces
that need a human + hardware in the loop.

> **Sample-dependent steps are temporarily inert.** The repository is
> mid-revamp; samples are returning in PR 3 of the tutorial-driven
> sample series (`sample/tutorial_app/` for the action-bridge demo
> and `sample/tutorial_gameinput/` as a standalone scene). The
> checklist below references the historical sample names —
> `gdk_launch_point` for the scenario shell and `multiplayer_pong`
> for the rumble-on-event verification — and will be re-pointed at
> the new samples once they land. Until then, GameInput hardware
> testing falls back to the GUT host plus your own minimal Godot
> project that calls `GameInput.initialize()` and exercises the
> action bridge.

## Setup

1. Build the addon:

    ```powershell
    cmake --preset default
    cmake --build build --preset debug
    ```

2. Open the GameInput-using Godot project of your choice. Until the
   samples land (PR 3), this means a project you wired up
   following [Tutorial — GameInput action bridge](../tutorials/gameinput-action-bridge.md).

## Per-feature checklist

### Initialize / shutdown lifecycle

- [ ] **Initialize GameInput** scenario reports
  `GameInput.initialize() succeeded.`
- [ ] State panel "GameInput" section flips to `Initialized: true`.
- [ ] **Shutdown GameInput** scenario reports `GameInput.shutdown() called.`
- [ ] State panel flips back to `Initialized: false`.
- [ ] Re-initializing right after shutdown succeeds without restarting the
  sample.

### Device discovery (one controller)

Plug in any GameInput-compatible gamepad (Xbox Series, Xbox One, Elite,
Razer Wolverine, etc.) **before** initializing.

- [ ] After **Initialize GameInput**, state panel reports `Devices: 1`.
- [ ] **List Devices** logs `1 device(s) — <Name> (#1)` with a sensible name.
- [ ] **Inspect Primary Device** logs vendor / product / vibration.

### Hot-plug (connect mid-frame)

Start with no controller plugged in, run **Initialize GameInput**, then plug
in a controller.

- [ ] State panel flips to `Devices: 1` and shows the new primary.
- [ ] Event log shows `GameInput device connected: <Name> (#1)`.
- [ ] `Last Event:` line in the state panel updates to
  `Connected — <Name> (#1)`.

### Hot-plug (disconnect mid-frame)

With a controller connected, unplug it.

- [ ] State panel device count drops by 1.
- [ ] Event log shows `GameInput device disconnected: #<id>`.
- [ ] Re-plugging assigns a **new** device id (ids are session-local
  monotonic; never recycled).

### Vibration — primary device

With a vibration-capable controller connected:

- [ ] **Rumble Pulse** scenario causes the controller to vibrate for ~0.4 s.
- [ ] **Stop Rumble** ends any in-flight rumble immediately.
- [ ] Event log records both events.

### Vibration — two controllers

Connect two vibration-capable controllers.

- [ ] **Rumble Pulse** vibrates the **primary** device only (not the
  secondary).
- [ ] After unplugging the primary, **Rumble Pulse** vibrates the new
  primary.

### Mapper — `Input.is_action_pressed` integration

The Mapper is exercised by any project that wires a `GameInputMapper` node
to a `GameInputActionMap`. For a focused unit, drop a `GameInputMapper`
node into a scene and assign a small `GameInputActionMap` with one binding.

- [ ] With a project that maps `move_up` / `move_down` to controller axes,
  the actions drive your gameplay. (This works through Godot's standard
  joypad mapping; the Mapper layer adds GameInput devices that aren't
  recognised by Godot's built-in joypad enum.)
- [ ] When you create a custom `GameInputBinding` that targets an action
  **not** present in `InputMap`, the Mapper logs **one** warning per missing
  action (not per frame).

## Regression smoke

After any change to the GameInput addon C++:

- [ ] Headless tests still pass:
  ```powershell
  pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 -Hosts @('tests\godot\gameinput')
  ```
  Expected output ends with `Overall: pass`.
- [ ] Any GameInput-using Godot project (yours or, once PR 3 lands, the
  tutorial samples) launches without `ERROR:` lines mentioning `gameinput`
  in editor output.
- [ ] `GameInput` singleton appears in **Project → Project Settings →
  Globals** (or wherever Godot lists engine singletons in your version).

If a row above fails, capture the editor output and the failing scenario
notes in the related issue or PR.
