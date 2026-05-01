# GameInput Manual Hardware Test Checklist

The headless test suite under `sample/gdk_launch_point/tests/` covers everything that
can be verified without a real controller. This document covers the pieces
that need a human + hardware in the loop.

## Setup

1. Build the addon and launch the **GDK Launch Point** sample editor:

    ```powershell
    cmake --preset default
    cmake --build build --preset debug
    cd sample\gdk_launch_point
    .\launch_editor.bat
    ```

2. Press **Play** to run the scenario shell.
3. From the home screen, open the **GameInput** group.

> Pong rumble verification needs the **multiplayer_pong** sample instead:
>
> ```powershell
> cd sample\multiplayer_pong
> .\launch_editor.bat
> ```

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
- [ ] **Inspect Primary Device** logs vendor / product / battery / vibration.

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

### Battery — wired

Connect a wired controller (e.g. an Xbox controller via USB cable).

- [ ] **Inspect Primary Device** reports `battery=wired/unknown`.

### Battery — wireless

Connect a wireless controller (e.g. Xbox Wireless via Bluetooth or the
Xbox Wireless Adapter).

- [ ] **Inspect Primary Device** reports `battery=N%` where `N` matches the
  controller's actual charge to within ~10%.
- [ ] State panel `Battery:` line updates accordingly.

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

The Mapper is exercised through pong; for a focused unit you can also drop a
`GameInputMapper` node into a test scene and assign a small
`GameInputActionMap` with one binding.

- [ ] With the pong sample running and a controller plugged in,
  `move_up` / `move_down` actions in the existing `[input]` map drive the
  paddle. (This works through Godot's standard joypad mapping; the Mapper
  layer adds GameInput devices that aren't recognised by the legacy joypad
  enum.)
- [ ] When you create a custom `GameInputBinding` that targets an action
  **not** present in `InputMap`, the Mapper logs **one** warning per missing
  action (not per frame).

## Pong-specific checks

Run the **multiplayer_pong** sample in **Single Player** mode.

- [ ] Score events vibrate the primary controller (low-freq pulse, ~0.25 s).
- [ ] Paddle hits vibrate the primary controller (short low-freq pulse,
  ~0.08 s).
- [ ] Plug a controller in at the lobby — status text shows
  `Controller connected: <Name>`.
- [ ] Unplug the controller — status text shows `Controller disconnected.`
- [ ] Pong remains fully playable on keyboard if no controller is connected
  (rumble paths soft-fail silently).

## Regression smoke

After any change to the GameInput addon C++:

- [ ] Headless tests still pass:
  ```powershell
  cd sample\gdk_launch_point
  .\Godot_v4.6.1-stable_win64_console.exe --headless --script res://tests/run_tests.gd
  ```
  Expected output ends with `Results: N passed, 0 failed, 0 skipped`.
- [ ] Both editors (`gdk_launch_point`, `multiplayer_pong`) launch without
  `ERROR:` lines mentioning `gameinput` in editor output.
- [ ] `GameInput` singleton appears in **Project → Project Settings →
  Globals** (or wherever Godot lists engine singletons in your version).

If a row above fails, capture the editor output and the failing scenario
notes in the related issue or PR.
