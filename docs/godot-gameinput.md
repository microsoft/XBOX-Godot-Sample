# GameInput addon (`godot_gameinput`)

`godot_gameinput` is a standalone GDExtension addon for GameInput integration
on Windows. It has no hard dependency on `godot_gdk` and can be used
independently.

## Current status

The addon is currently a **scaffold** — it builds, loads in Godot, and
registers one probe class to verify the extension pipeline works end-to-end.

### Implemented now

- GDExtension build target with its own `CMakeLists.txt`
- `GodotGameInputProbe` node class with a `status_text` property
  (returns `"godot_gameinput loaded"`)
- Addon metadata (`.gdextension` file)
- Build output synced to the sample project

### Not implemented yet

- GameInput runtime initialization (`GameInputCreate`)
- Device discovery and lifecycle
- Polling API (current-reading access)
- Reading callbacks (event-driven input)
- Vibration / rumble (`SetRumbleState`)
- `GameInputMapper` action bridge node
- `GameInputActionMap` resource

## Planned API

The full planned API is described in the design spec at
[`spec/gdext-gameinput.md`](../spec/gdext-gameinput.md).

Key planned surfaces:

| Surface | Description |
|---------|-------------|
| `GameInput` singleton | Root singleton for device discovery, polling, vibration |
| `GameInputDevice` | Wrapper around `IGameInputDevice` |
| `GameInputReading` | Wrapper around input reading state |
| `GameInputMapper` | Optional `Node` that bridges GameInput → Godot actions |
| `GameInputActionMap` | `Resource` for action bindings |

## Building

```powershell
# Build only the GameInput addon
cmake --preset gameinput-only
cmake --build --preset debug-gameinput

# Or build everything (both addons)
cmake --preset default
cmake --build build --preset debug
```

Output DLLs land in `addons/godot_gameinput/bin/` and are synced to
`sample/addons/godot_gameinput/bin/` by the build.

## Using in a project

1. Copy `addons/godot_gameinput/` into your Godot project
2. Enable the addon in Project → Project Settings → Plugins
3. The `GodotGameInputProbe` node type will be available (scaffold only for
   now)
