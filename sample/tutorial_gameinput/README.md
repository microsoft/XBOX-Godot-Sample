# Tutorial GameInput — standalone sample

This Godot 4.x project is the **end-state** of the
[GameInput action-bridge tutorial](../../docs/tutorials/gameinput-action-bridge.md),
the standalone track of the tutorial series. It does **not** depend
on Xbox sign-in or PlayFab — bring a wired or wireless Xbox
controller and you're ready to run.

## Quick start

1. **Build the addon.** From the repo root:

   ```powershell
   cmake --preset default
   cmake --build build --preset debug
   ```

2. **Plug in a GameInput-supported gamepad.** Xbox controllers
   over USB or Bluetooth are the common case; any GameInput-
   compatible device works.

3. **Open the project in Godot and run `main.tscn`.** The
   `GameInputBootstrap` autoload (installed by the addon's editor
   plugin) initializes `GameInput` on `_ready` and pumps `poll()`
   every process frame — your action map starts firing as soon as
   the mapper picks up the device.

## Producing a packaged build

The sample includes a committed `export_presets.cfg` with a `Windows Desktop`
preset, so a clean clone can open **Project → Export** and edit or run the
export without first authoring a preset.

Install the Godot 4.6.1 export templates under
`%APPDATA%\Godot\export_templates\` before exporting.

## See also

- [Tutorial — GameInput action bridge](../../docs/tutorials/gameinput-action-bridge.md)
- [GameInput plugin reference](../../docs/gameinput/plugin.md)
- Integrated GDK + PlayFab sample: [`sample/tutorial_app/`](../tutorial_app/README.md)
